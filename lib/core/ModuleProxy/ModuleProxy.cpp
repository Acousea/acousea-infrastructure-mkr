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
    getCache().invalidateModule(code);

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
    getCache().invalidateModule(code);

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
// =============== CachedValue impl ==================
// ===================================================

ModuleProxy::CachedValue::CachedValue() = default;

ModuleProxy::CachedValue::CachedValue(const acousea_ModuleWrapper& v, bool hasValue, bool fresh)
    : value(v), isFresh_(fresh)
{
}

void ModuleProxy::CachedValue::store(const acousea_ModuleWrapper& v)
{
    value = v;
    isFresh_ = true;
}

void ModuleProxy::CachedValue::invalidate()
{
    isFresh_ = false;
}

ModuleProxy::CachedValue ModuleProxy::CachedValue::empty()
{
    return {};
}

bool ModuleProxy::CachedValue::isFresh() const
{
    return isFresh_;
}

const acousea_ModuleWrapper& ModuleProxy::CachedValue::get() const
{
    return value;
}

// ===================================================
// =============== ModuleCache impl ==================
// ===================================================

bool ModuleProxy::ModuleCache::store(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    // Si ya existe, sobrescribe
    for (auto& e : entries)
    {
        if (e.occupied && static_cast<acousea_ModuleCode>(e.value.get().which_module) == code)
        {
            e.value.store(wrapper);
            return true;
        }
    }

    // Busca un slot libre
    for (auto& e : entries)
    {
        if (!e.occupied)
        {
            e.value.store(wrapper);
            e.occupied = true;
            return true;
        }
    }

    return false; // No hay espacio
}

ModuleProxy::CachedValue ModuleProxy::ModuleCache::get(acousea_ModuleCode code) const
{
    for (const auto& e : entries)
        if (e.occupied && static_cast<acousea_ModuleCode>(e.value.get().which_module) == code)
            return e.value;
    return CachedValue::empty();
}


const acousea_ModuleWrapper* ModuleProxy::ModuleCache::getIfFresh(acousea_ModuleCode code) const
{
    for (const auto& e : entries)
        if (e.occupied && static_cast<acousea_ModuleCode>(e.value.get().which_module) == code && e.value.isFresh())
            return &e.value.get();
    return nullptr;
}

const acousea_ModuleWrapper* ModuleProxy::getIfFreshOrRequestFromDevice(
    const acousea_ModuleCode code, const DeviceAlias alias
)
{
    const auto optModule = getCache().getIfFresh(code);

    if (optModule != nullptr)
    {
        return optModule;
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
    const auto optModulePtr = getCache().getIfFresh(code);

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


void ModuleProxy::ModuleCache::invalidateModule(const acousea_ModuleCode code)
{
    for (auto& e : entries)
        if (e.occupied && static_cast<acousea_ModuleCode>(e.value.get().which_module) == code)
            e.value.invalidate();
}

void ModuleProxy::ModuleCache::invalidateAll()
{
    for (auto& e : entries)
        if (e.occupied)
            e.value.invalidate();
}

bool ModuleProxy::ModuleCache::isFresh(acousea_ModuleCode code) const
{
    for (const auto& e : entries)
        if (e.occupied && static_cast<acousea_ModuleCode>(e.value.get().which_module) == code)
            return e.value.isFresh();
    return false;
}

ModuleProxy::ModuleCache ModuleProxy::ModuleCache::clone() const
{
    ModuleCache copy;
    for (size_t i = 0; i < MAX_MODULES; ++i)
        copy.entries[i] = entries[i];
    return copy;
}

void ModuleProxy::ModuleCache::swap(ModuleCache& other) noexcept
{
    for (size_t i = 0; i < MAX_MODULES; ++i)
        std::swap(entries[i], other.entries[i]);
}

void swap(ModuleProxy::ModuleCache& a, ModuleProxy::ModuleCache& b) noexcept
{
    a.swap(b);
}
