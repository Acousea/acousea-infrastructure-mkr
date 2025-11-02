#include "ModuleProxy.hpp"
#include "Logger/Logger.h"

// ===================================================
// ================ Constructor ======================
// ===================================================

ModuleProxy::ModuleProxy(Router& router)
    : router(router)
{
}

// ===================================================
// ============== Envío de comandos ==================
// ===================================================

void ModuleProxy::requestModule(acousea_ModuleCode code) const
{
    auto pkt = buildRequestPacket(code);
    router.sendFrom(Router::broadcastAddress).sendSerial(pkt);
}

template <typename ModuleT>
void ModuleProxy::sendModule(acousea_ModuleCode code, const ModuleT& module) const
{
    const auto pkt = buildSetPacket(code, module);
    router.sendFrom(Router::broadcastAddress).sendSerial(pkt);
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

template <typename ModuleT>
acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode code, const ModuleT& module)
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
    entry.value = acousea_ModuleWrapper_init_default;

    // ===============================
    //  Asignar al campo correcto
    // ===============================
    if constexpr (std::is_same_v<ModuleT, acousea_ICListenLoggingConfig>) {
        entry.value.which_module = acousea_ModuleWrapper_icListenLoggingConfig_tag;
        entry.value.module.icListenLoggingConfig = module;
    }
    else if constexpr (std::is_same_v<ModuleT, acousea_ICListenStreamingConfig>) {
        entry.value.which_module = acousea_ModuleWrapper_icListenStreamingConfig_tag;
        entry.value.module.icListenStreamingConfig = module;
    }
    else if constexpr (std::is_same_v<ModuleT, acousea_ICListenHF>) {
        entry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;
        entry.value.module.icListenHF = module;
    }
    else if constexpr (std::is_same_v<ModuleT, acousea_ICListenRecordingStats>) {
        entry.value.which_module = acousea_ModuleWrapper_icListenRecordingStats_tag;
        entry.value.module.icListenRecordingStats = module;
    }
    else if constexpr (std::is_same_v<ModuleT, acousea_BatteryModule>) {
        entry.value.which_module = acousea_ModuleWrapper_battery_tag;
        entry.value.module.battery = module;
    }
    else if constexpr (std::is_same_v<ModuleT, acousea_LocationModule>) {
        entry.value.which_module = acousea_ModuleWrapper_location_tag;
        entry.value.module.location = module;
    }
    else {
        static_assert([] { return false; }(), "Unsupported ModuleT type for buildSetPacket");
    }

    return pkt;
}

// ===================================================
// ===================== Cache =======================
// ===================================================

void ModuleProxy::ModuleCache::store(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper)
{
    cache[code].store(wrapper);
}

std::optional<acousea_ModuleWrapper> ModuleProxy::ModuleCache::get(acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    if (it == cache.end() || !it->second.valid()) return std::nullopt;
    return it->second.get();
}

void ModuleProxy::ModuleCache::invalidate(acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    if (it != cache.end()) it->second.invalidate();
}

void ModuleProxy::ModuleCache::invalidateAll() const
{
    for (auto& kv : cache)
        kv.second.invalidate();
}

bool ModuleProxy::ModuleCache::fresh(acousea_ModuleCode code) const
{
    const auto it = cache.find(code);
    return (it != cache.end()) && it->second.fresh();
}

// ===================================================
// ============ Instanciaciones explícitas ===========
// ===================================================

template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_ICListenLoggingConfig&);
template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_ICListenStreamingConfig&);
template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_ICListenHF&);
template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_ICListenRecordingStats&);
template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_BatteryModule&);
template acousea_CommunicationPacket ModuleProxy::buildSetPacket(acousea_ModuleCode, const acousea_LocationModule&);
