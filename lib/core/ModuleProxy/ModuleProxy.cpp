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

bool ModuleProxy::begin()
{
    return true;
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

bool ModuleProxy::begin()
{
    // Initialize storage manager if needed
    if (const bool beginOk = storage.begin(); !beginOk)
    {
        LOG_CLASS_ERROR("::begin() -> Failed to initialize StorageManager");
        return false;
    }

    constexpr auto MIN_CODE = _acousea_ModuleCode_MIN + 1;
    constexpr auto MAX_CODE = _acousea_ModuleCode_MAX;
    for (auto code = MIN_CODE; code <= MAX_CODE; code++)
    {
        char path[10];
        snprintf(path, sizeof(path), "/mod%d", static_cast<int>(code));

        if (storage.fileExists(path))
        {
            LOG_CLASS_INFO("::begin() -> Module cache file exists for module %d", static_cast<int>(code));
            readOffset_[code] = storage.fileSize(path);
            writeOffset_[code] = storage.fileSize(path);
        }
        else
        {
            if (const bool createOk = storage.createEmptyFile(path); !createOk)
            {
                LOG_CLASS_ERROR("::begin() -> Cannot create module cache file: %s", path);
                return false;
            }
            LOG_CLASS_INFO("::begin() -> Created module cache file: %s", path);
        }
    }
    return true;
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
    // Archivo del módulo
    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", static_cast<int>(code));

    // Buffer temporal compartido
    auto* tmp = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
    constexpr size_t maxSize = SharedMemory::tmpBufferSize();

    // Reservamos los 3 bytes de cabecera en tmp
    // payload irá en tmp[3 .. 3+len-1]
    Result<size_t> encodeRes = ProtoUtils::ModuleWrapper::encodeInto(
        wrapper,
        tmp + 3, // dejamos espacio para start + len
        maxSize - 4 // también dejamos espacio para end byte
    );

    if (!encodeRes.isSuccess())
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to encode module %d", static_cast<int>(code));
        return false;
    }

    const auto payloadLen = static_cast<uint16_t>(encodeRes.getValue());
    const size_t totalRecordSize = 1 + 2 + payloadLen + 1; // START + LEN + PAYLOAD + END

    if (totalRecordSize > maxSize)
    {
        LOG_CLASS_ERROR("::storeModule() -> Module record too large (%u bytes)", totalRecordSize);
        return false;
    }

    // --- CABECERA ---
    tmp[0] = START_BYTE;
    tmp[1] = static_cast<uint8_t>(payloadLen & 0xFF);
    tmp[2] = static_cast<uint8_t>((payloadLen >> 8) & 0xFF);

    // --- FOOTER ---
    tmp[3 + payloadLen] = END_BYTE;

    // --- APPEND A SD ---
    if (const bool appendOk = storage.appendBytesToFile(filePath, tmp, totalRecordSize); !appendOk)
    {
        LOG_CLASS_WARNING("::storeModule() -> Failed to write module %d to file %s",
                          static_cast<int>(code),
                          filePath);
        return false;
    }

    writeOffset_[static_cast<int>(code)] += totalRecordSize;

    LOG_CLASS_INFO("::storeModule() -> Stored module %d (%u bytes) in %s",
                   static_cast<int>(code),
                   totalRecordSize,
                   filePath);

    return true;
}


const acousea_ModuleWrapper* ModuleProxy::getIfFresh(acousea_ModuleCode code)
{
    const int idx = static_cast<int>(code);
    constexpr uint8_t HEADER_SIZE = 3; // START + LEN_LO + LEN_HI
    constexpr uint8_t FOOTER_SIZE = 1; // END

    // Si ya está cargado en RAM → devolverlo directamente
    if (optLoadedModule_.has_value())
        return &optLoadedModule_.value();

    // Si no hay datos nuevos → no está fresco
    if (readOffset_[idx] == writeOffset_[idx])
    {
        LOG_CLASS_INFO("::getIfFresh() -> Module %d not fresh (readOffset==writeOffset)", idx);
        return nullptr;
    }

    char filePath[10];
    std::snprintf(filePath, sizeof(filePath), "/mod%d", idx);

    if (!storage.fileExists(filePath))
    {
        LOG_CLASS_INFO("::getIfFresh() -> Module file %s does not exist", filePath);
        return nullptr;
    }

    auto* readBuf = reinterpret_cast<uint8_t*>(SharedMemory::tmpBuffer());
    constexpr size_t readBufMaxSize = SharedMemory::tmpBufferSize();


    // --- 1) Leer cabecera: START + LEN_LO + LEN_HI ---
    if (storage.readFileRegionBytes(filePath, readOffset_[idx], readBuf, HEADER_SIZE) != HEADER_SIZE)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Failed to read header for module %d", idx);
        readOffset_[idx] = writeOffset_[idx]; // marcar como no fresco
        return nullptr;
    }

    if (readBuf[0] != START_BYTE)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Bad START_BYTE for module %d", idx);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    const uint16_t registeredLength = readBuf[1] | (readBuf[2] << 8);

    // Check if registered length exceeds the reading buffer capacity
    if (registeredLength + HEADER_SIZE + FOOTER_SIZE > readBufMaxSize) // 3 header + 1 footer
    {
        LOG_CLASS_ERROR("::getIfFresh() -> Module %d record too large (%u bytes)", idx, registeredLength);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    // --- 2) Leer payload + END_BYTE ---
    if (storage.readFileRegionBytes(filePath,
                                    readOffset_[idx] + HEADER_SIZE,
                                    readBuf + HEADER_SIZE,
                                    registeredLength + FOOTER_SIZE) != registeredLength + FOOTER_SIZE)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Failed to read payload for module %d", idx);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    if (readBuf[HEADER_SIZE + registeredLength] != END_BYTE)
    {
        LOG_CLASS_WARNING("::getIfFresh() -> Bad END_BYTE for module %d", idx);
        readOffset_[idx] = writeOffset_[idx];
        return nullptr;
    }

    // --- 3) Decodificar directamente en optLoadedModule_ ---
    optLoadedModule_.emplace();
    const auto decodeResult = ProtoUtils::ModuleWrapper::decodeInto(
        readBuf + HEADER_SIZE, // payload begins here
        registeredLength,
        &optLoadedModule_.value()
    );

    if (!decodeResult.isSuccess())
    {
        LOG_CLASS_WARNING("::getIfFresh() -> decodeInto failed for module %d", idx);
        readOffset_[idx] = writeOffset_[idx];
        optLoadedModule_.reset();
        return nullptr;
    }

    // --- 4) Avanzar readOffset ---
    readOffset_[idx] += (HEADER_SIZE + registeredLength + FOOTER_SIZE);

    LOG_CLASS_INFO("::getIfFresh() -> Loaded module %d from offset=%llu", idx, readOffset_[idx]);

    return &optLoadedModule_.value();
}


void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    const int idx = code;
    if (idx < _acousea_ModuleCode_MIN + 1 | idx > _acousea_ModuleCode_MAX)
    {
        LOG_CLASS_WARNING("::invalidateModule() -> Module code %d out of bounds", static_cast<int>(code));
        return; // Código inválido
    }

    // Marcamos como “no fresco”
    readOffset_[idx] = writeOffset_[idx];

    // Si había un módulo cargado, lo invalidamos también
    optLoadedModule_.reset();

    LOG_CLASS_INFO( "::invalidateModule() -> Invalidated module %d (readOffset=%lu%lu, writeOffset=%lu%lu)",
        idx,
        static_cast<unsigned long>(readOffset_[idx] >> 32),
        static_cast<unsigned long>(readOffset_[idx] & 0xFFFFFFFFULL),
        static_cast<unsigned long>(writeOffset_[idx] >> 32),
        static_cast<unsigned long>(writeOffset_[idx] & 0xFFFFFFFFULL)
    );
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
