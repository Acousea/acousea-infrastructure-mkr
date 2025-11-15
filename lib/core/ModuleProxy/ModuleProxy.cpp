#include "ModuleProxy.hpp"

#include <cstdio>

#include "Logger/Logger.h"
#include "SharedMemory/SharedMemory.hpp"


// ===================================================
// ================ Constructor ======================
// ===================================================

constexpr const char* ModuleProxy::toString(const ModuleProxy::DeviceAlias alias) noexcept
{
    switch (alias)
    {
    case ModuleProxy::DeviceAlias::PIDevice: return "PIDevice";
    case ModuleProxy::DeviceAlias::VR2C: return "VR2C";
    default: return "Unknown";
    }
}

#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
ModuleProxy::ModuleProxy(Router& router, const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap)
    : router(router), devicePortMap(devicePortMap)
{
}

#else
ModuleProxy::ModuleProxy(Router& router,
                         StorageManager& storageManager,
                         const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap) :
    router(router),
    storage(storageManager),
    devicePortMap(devicePortMap)
{
}

#endif

// ===================================================
// ============== Envío de comandos ==================
// ===================================================

bool ModuleProxy::requestModule(const acousea_ModuleCode code, const DeviceAlias alias)
{
    acousea_CommunicationPacket& pkt = buildRequestModulePacket(code);
    const auto portType = resolvePort(alias);
    if (portType == IPort::PortType::None)
    {
        Logger::logWarning("ModuleProxy::requestModule() -> Could not resolve port for alias");
        return false;
    }

    // The module will be marked as not fresh when the response is received
    invalidateModule(code);

    LOG_CLASS_INFO("Requesting module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(portType), toString(alias)
    );

    return router
           .from(Router::broadcastAddress)
           .through(portType)
           .send(pkt);
}

bool ModuleProxy::sendModule(const acousea_ModuleCode code,
                             const acousea_ModuleWrapper& module,
                             const DeviceAlias alias)
{
    acousea_CommunicationPacket& pkt = buildSetModulePacket(code, module);
    const auto portType = resolvePort(alias);
    if (portType == IPort::PortType::None)
    {
        LOG_CLASS_WARNING("ModuleProxy::sendModule() -> Could not resolve port for alias %s", toString(alias));
        return false;
    }

    // The module must be previously invalidated and set not fresh
    invalidateModule(code);

    LOG_CLASS_INFO("::sendModule() -> Sending module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(portType), toString(alias)
    );


    return router
           .from(Router::broadcastAddress)
           .through(portType)
           .send(pkt);
}

IPort::PortType ModuleProxy::resolvePort(const DeviceAlias alias) const noexcept
{
    const auto it = devicePortMap.find(alias);
    if (it == devicePortMap.end())
    {
        LOG_CLASS_WARNING(
            "ModuleProxy::resolvePort() -> Node has no port associated with device alias: %s",
            toString(alias)
        );
        return IPort::PortType::None;
    }
    return it->second;
}


// Build the request packet using SharedMemory's communicationPacketRef()
acousea_CommunicationPacket& ModuleProxy::buildRequestModulePacket(acousea_ModuleCode code)
{
    LOG_CLASS_FREE_MEMORY("::buildRequestPacket(start)");
    auto& pkt = SharedMemory::communicationPacketRef(); // Use SharedMemory's communication packet
    pkt = acousea_CommunicationPacket_init_default; // Initialize the packet
    pkt.has_routing = true;
    pkt.routing.sender = Router::broadcastAddress;
    pkt.routing.receiver = 0; // backend
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;
    pkt.body.command.command.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    pkt.body.command.command.requestedConfiguration.requestedModules_count = 1;
    pkt.body.command.command.requestedConfiguration.requestedModules[0] = code;
    LOG_CLASS_FREE_MEMORY("::buildRequestPacket(end)");
    return pkt;
}

// ===================================================
// ============ buildSetPacket (corregido) ===========
// ===================================================

acousea_CommunicationPacket& ModuleProxy::buildSetModulePacket(acousea_ModuleCode code,
                                                               const acousea_ModuleWrapper& module)
{
    auto& pkt = SharedMemory::communicationPacketRef(); // Usamos el paquete de SharedMemory
    pkt = acousea_CommunicationPacket_init_default; // Inicializamos el paquete en SharedMemory
    pkt.has_routing = true;
    pkt.routing.sender = Router::broadcastAddress;
    pkt.routing.receiver = 0; // backend
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
    pkt.body.command.command.setConfiguration.modules_count = 1;

    auto& entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.has_value = true;
    entry.key = code;
    entry.value = module;

    return pkt;
}

const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrRequestFromDevice(
    const acousea_ModuleCode code, const DeviceAlias alias
)
{
    const acousea_ModuleWrapper* optModulePtr = getIfFresh(code);

    if (optModulePtr != nullptr)
    {
        return optModulePtr;
    }

    if (const auto reqOk = requestModule(code, alias); !reqOk)
    {
        LOG_CLASS_WARNING("ModuleProxy::getIfFreshOrRequestTo() -> Failed to request module %d to %s",
                          static_cast<int>(code), toString(alias));
        return nullptr;
    }

    LOG_CLASS_INFO("ModuleProxy::getIfFreshOrRequestTo() -> Requested module %d to %s (waiting response)",
                   static_cast<int>(code), toString(alias));

    return nullptr;
}

const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrSetOnDevice(
    const acousea_ModuleCode code,
    const acousea_ModuleWrapper& module,
    const DeviceAlias alias
)
{
    const acousea_ModuleWrapper* optModulePtr = getIfFresh(code);

    if (optModulePtr != nullptr)
    {
        // Módulo confirmado en caché, no hace falta enviar nada
        return optModulePtr;
    }

    if (const bool sendOk = sendModule(code, module, alias); !sendOk)
    {
        LOG_CLASS_ERROR("ModuleProxy::getIfFreshOrSetOnDevice() -> Failed to send module %d to %s",
                        static_cast<int>(code), toString(alias));
        return nullptr;
    }

    LOG_CLASS_INFO("ModuleProxy::getIfFreshOrSetOnDevice() -> Sent module %d to %s (waiting confirmation)",
                   static_cast<int>(code), toString(alias));

    return nullptr;
}

// =============================================================================================
// ================================ CACHE IN RAM IMPLEMENTATION ================================
// =============================================================================================

#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
bool ModuleProxy::storeModule(const acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    const uint16_t idx = static_cast<uint16_t>(code) - 1;
    if (idx < 0 || idx > ModuleProxy::MAX_MODULES - 1)
    {
        LOG_CLASS_WARNING("::storeModule() -> Module idx %d out of bounds [0, %d)",
                          static_cast<int>(idx), ModuleProxy::MAX_MODULES - 1);
        return false; // Código inválido
    }
    entries[idx].emplace(wrapper);
    return true; // No hay espacio
}

const acousea_ModuleWrapper* ModuleProxy::getIfFresh(acousea_ModuleCode code)
{
    const auto& entry = entries[static_cast<uint16_t>(code) - 1];
    if (entry.has_value())
    {
        return &entry.value();
    }
    return nullptr;
}

void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    const uint16_t idx = static_cast<uint16_t>(code) - 1;
    if (idx < 0 || idx > ModuleProxy::MAX_MODULES - 1)
    {
        LOG_CLASS_WARNING("::invalidateModule() -> Module idx %d out of bounds [0, %d)",
                          static_cast<int>(idx), ModuleProxy::MAX_MODULES - 1);
        return; // Código inválido
    }
    entries[idx] = (std::nullopt);
}

void ModuleProxy::invalidateAll()
{
    for (auto& e : entries)
    {
        e = std::nullopt;
    }
}

#else

bool ModuleProxy::storeModule(const acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    // Use the code to build the filename (e.g., "mod1", "mod2", ..., "modN")
    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", static_cast<int>(code));

    // Serializamos el módulo
    uint8_t buffer[SharedMemory::tmpBufferSize()];
    auto encodeResult = ProtoUtils::ModuleWrapper::encodeInto(wrapper, buffer, sizeof(buffer));
    if (!encodeResult.isSuccess())
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to encode module %d", static_cast<int>(code));
        return false;
    }

    // Escribimos el módulo en el archivo correspondiente
    if (const bool writeResult = storage.writeFileBytes(filePath, buffer, encodeResult.getValue()); !writeResult)
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to write module %d to file %s", static_cast<int>(code), filePath);
        return false;
    }

    LOG_CLASS_INFO("::storeModule() -> Successfully stored module %d in file %s", static_cast<int>(code), filePath);
    return true;
}


const acousea_ModuleWrapper* ModuleProxy::getIfFresh(acousea_ModuleCode code)
{
    // Primero comprobamos si el módulo está cargado en memoria
    if (optLoadedModule_.has_value())
    {
        // Si el módulo está en memoria, lo devolvemos directamente
        return &optLoadedModule_.value();
    }

    // Si el módulo no está cargado, intentamos cargarlo desde el archivo correspondiente
    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", static_cast<int>(code));

    const auto& readBuffer = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
    constexpr auto readBufSize = SharedMemory::tmpBufferSize();

    // Leemos el archivo correspondiente (modX) directamente en el tmpBuffer de SharedMemory
    if (!storage.fileExists(filePath))
    {
        LOG_CLASS_INFO("::getIfFresh() -> Module file %s does not exist", filePath);
        return nullptr; // El archivo no existe, el módulo no está fresco
    }

    const size_t bytesRead = storage.readFileBytes(filePath, readBuffer, readBufSize);
    if (bytesRead == 0)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Failed to read module %d from file %s", static_cast<int>(code), filePath);
        return nullptr; // No se pudo cargar el módulo desde el archivo
    }

    // Intentamos decodificar el módulo directamente en optLoadedModule_
    auto decodeResult = ProtoUtils::ModuleWrapper::decodeInto(
        readBuffer,
        bytesRead,
        &optLoadedModule_.value() // Deserializamos directamente en optLoadedModule_
    );

    if (!decodeResult.isSuccess())
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Failed to decode module %d from file %s", static_cast<int>(code),
                          filePath);
        return nullptr; // Fallo al decodificar el módulo
    }
    // Devolvemos el módulo recién cargado
    return &optLoadedModule_.value();
}


void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    // Si el módulo no está cargado, intentamos cargarlo desde el archivo correspondiente
    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", static_cast<int>(code));

    if (const bool doesFileExist = storage.fileExists(filePath); !doesFileExist)
    {
        LOG_CLASS_INFO("::invalidateModule() -> Module file %s does not exist, nothing to invalidate", filePath);
        return; // El archivo no existe, nada que invalidar
    }
    // Borramos el archivo correspondiente
    if (const bool clearResult = storage.clearFile(filePath); !clearResult)
    {
        LOG_CLASS_WARNING("::invalidateModule() -> Failed to delete module file %s", filePath);
    }
    LOG_CLASS_INFO("::invalidateModule() -> Deleted module file %s", filePath);
}

void ModuleProxy::invalidateAll()
{
    constexpr auto minCode = _acousea_ModuleCode_MIN + 1;
    constexpr auto maxCode = _acousea_ModuleCode_MAX;
    for (uint8_t code = minCode; code <= maxCode; code++)
    {
        invalidateModule(static_cast<acousea_ModuleCode>(code));
    }
}


#endif
