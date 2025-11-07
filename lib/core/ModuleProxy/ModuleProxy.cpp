#include "ModuleProxy.hpp"
#include "Logger/Logger.h"

// ===================================================
// ================ Constructor ======================
// ===================================================

constexpr const char* ModuleProxy::toString(const ModuleProxy::DeviceAlias alias) noexcept
{
    switch (alias)
    {
    case ModuleProxy::DeviceAlias::ICListen: return "ICListen";
    case ModuleProxy::DeviceAlias::VR2C: return "VR2C";
    default: return "Unknown";
    }
}


ModuleProxy::ModuleProxy(Router& router)
    : router(router)
{
}

ModuleProxy::ModuleProxy(Router& router, const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap)
    : router(router), devicePortMap(devicePortMap)
{
}

// ===================================================
// ============== Envío de comandos ==================
// ===================================================

bool ModuleProxy::requestModule(const acousea_ModuleCode code, const DeviceAlias alias) const
{
    const auto pkt = buildRequestPacket(code);
    const auto portType = resolvePort(alias);
    if (!portType.has_value())
    {
        Logger::logWarning("ModuleProxy::requestModule() -> Could not resolve port for alias");
        return false;
    }


    LOG_CLASS_INFO("Requesting module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(*portType), toString(alias)
    );

    return router
           .from(Router::broadcastAddress)
           .through(*portType)
           .send(pkt);
}

bool ModuleProxy::sendModule(const acousea_ModuleCode code, const acousea_ModuleWrapper& module,
                             const DeviceAlias alias) const
{
    const auto pkt = buildSetPacket(code, module);
    const auto portType = resolvePort(alias);
    if (!portType.has_value())
    {
        LOG_CLASS_WARNING("ModuleProxy::sendModule() -> Could not resolve port for alias %s", toString(alias));
        return false;
    }

    LOG_CLASS_INFO("::sendModule() -> Sending module %d through %s for alias %s", static_cast<int>(code),
                   IPort::portTypeToCString(*portType), toString(alias)
    );


    return router
           .from(Router::broadcastAddress)
           .through(*portType)
           .send(pkt);
}

std::optional<IPort::PortType> ModuleProxy::resolvePort(const DeviceAlias alias) const
{
    const auto it = devicePortMap.find(alias);
    if (it == devicePortMap.end())
    {
        LOG_CLASS_WARNING("ModuleProxy::resolvePort() -> Node has no port associated with device alias: %s",
                          toString(alias));
        return std::nullopt;
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
// ===================== Cache =======================
// ===================================================

void ModuleProxy::ModuleCache::store(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    cache[code].store(wrapper);
}

ModuleProxy::CachedValue ModuleProxy::ModuleCache::get(acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    if (it == cache.end() || !it->second.valid()) return CachedValue::empty();
    return it->second;
}

std::optional<acousea_ModuleWrapper> ModuleProxy::ModuleCache::getIfFresh(acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    if (it == cache.end() || !it->second.fresh()) return std::nullopt;
    return it->second.get();
}

void ModuleProxy::ModuleCache::invalidate(acousea_ModuleCode code)
{
    const auto it = cache.find(code);
    if (it != cache.end()) it->second.invalidate();
}

void ModuleProxy::ModuleCache::invalidateAll()
{
    for (auto& kv : cache)
        kv.second.invalidate();
}

bool ModuleProxy::ModuleCache::isFresh(const acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    return (it != cache.end()) && it->second.fresh();
}

ModuleProxy::ModuleCache ModuleProxy::ModuleCache::clone() const
{
    ModuleCache copy;
    copy.cache = this->cache;
    return copy;
}


void ModuleProxy::ModuleCache::swap(ModuleCache& other) noexcept
{
    cache.swap(other.cache);
}

void swap(ModuleProxy::ModuleCache& a, ModuleProxy::ModuleCache& b) noexcept
{
    a.swap(b);
}
