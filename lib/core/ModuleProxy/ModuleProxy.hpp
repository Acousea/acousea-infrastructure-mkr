#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP

#include "Router.h"
#include "bindings/nodeDevice.pb.h"

#include <optional>
#include <unordered_map>

#include "ProtoUtils/ProtoUtils.hpp"


#define MODULE_PROXY_CACHE_IN_RAM_ENABLED

#ifndef MODULE_PROXY_CACHE_IN_RAM_ENABLED
#include "StorageManager/StorageManager.hpp"
#endif

/**
 * @brief Proporciona una interfaz genérica para solicitar y enviar configuraciones
 *        de módulos al backend (Raspberry Pi u otro nodo), manteniendo una caché local.
 */
class ModuleProxy
{
public:
    // ===================== Alias for devices associated with prots =====================
    enum class DeviceAlias
    {
        PIDevice,
        VR2C
    };

    static constexpr const char* toString(DeviceAlias alias) noexcept;

    // ============================================================
    // Constructors
    // ============================================================
#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
    ModuleProxy(Router& router, const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap);
#else
    explicit ModuleProxy(Router& router,
                         StorageManager& storageManager,
                         const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap);
#endif


    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrRequestFromDevice(
        acousea_ModuleCode code,
        DeviceAlias alias
    );

    [[nodiscard]] const acousea_ModuleWrapper* getIfFresh(acousea_ModuleCode code) const;

    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrSetOnDevice(acousea_ModuleCode code,
                                                                       const acousea_ModuleWrapper& module,
                                                                       DeviceAlias alias);

    // ===================== Send / Receive =====================
    [[nodiscard]] bool requestModule(acousea_ModuleCode code, DeviceAlias alias);

    [[nodiscard]] bool sendModule(acousea_ModuleCode code, const acousea_ModuleWrapper& module, DeviceAlias alias);

    [[nodiscard]] bool storeModule(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper);

private:
    Router& router;

#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
    static constexpr const auto MAX_MODULES = ProtoUtils::ACOUSEA_MAX_MODULE_COUNT;
    std::optional<acousea_ModuleWrapper> entries[MAX_MODULES] = {}; // Initialized to std::nullopt
#else
    StorageManager& storage;
#endif

    // ===================== Mapeo alias -> puerto =====================
    const std::unordered_map<DeviceAlias, IPort::PortType> devicePortMap{
        {DeviceAlias::PIDevice, IPort::PortType::SerialPort},
        {DeviceAlias::VR2C, IPort::PortType::SerialPort}
    };

    [[nodiscard]] IPort::PortType resolvePort(DeviceAlias alias) const noexcept;

    // ===================== Construcción de paquetes =====================
    [[nodiscard]] static acousea_CommunicationPacket buildRequestPacket(acousea_ModuleCode code);

    [[nodiscard]] static acousea_CommunicationPacket buildSetPacket(acousea_ModuleCode code,
                                                                    const acousea_ModuleWrapper& module);

    void invalidateModule(acousea_ModuleCode code);
    void invalidateAll();

#ifdef UNIT_TESTING
    friend class ModuleProxyTest_SendModuleBuildsValidPacket_Test;
#endif
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
