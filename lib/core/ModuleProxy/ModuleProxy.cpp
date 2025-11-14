#include "ModuleProxy.hpp"
#include "Logger/Logger.h"


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
    auto pkt = buildRequestPacket(code);
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
    auto pkt = buildSetPacket(code, module);
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


acousea_CommunicationPacket ModuleProxy::buildRequestPacket(acousea_ModuleCode code)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
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
    return pkt;
}

// ===================================================
// ============ buildSetPacket (corregido) ===========
// ===================================================

acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode code, const acousea_ModuleWrapper& module)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
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

// ===================================================
// =============== ModuleCache impl ==================
// ===================================================

bool ModuleProxy::storeModule(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    uint16_t idx = static_cast<uint16_t>(code) - 1;
    entries[idx].emplace(wrapper);
    return false; // No hay espacio
}

const acousea_ModuleWrapper* ModuleProxy::getIfFresh(acousea_ModuleCode code) const
{
    const auto& entry = entries[static_cast<uint16_t>(code) - 1];
    if (entry.has_value())
    {
        return &entry.value();
    }
    return nullptr;
}


const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrRequestFromDevice(
    const acousea_ModuleCode code, const DeviceAlias alias
)
{
    const auto optModulePtr = getIfFresh(code);

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
    const auto optModulePtr = getIfFresh(code);

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


void ModuleProxy::invalidateModule(const acousea_ModuleCode code)
{
    entries[static_cast<uint16_t>(code) - 1] = (std::nullopt);
}

void ModuleProxy::invalidateAll()
{
    for (auto& e : entries)
    {
        e = std::nullopt;
    }
}
