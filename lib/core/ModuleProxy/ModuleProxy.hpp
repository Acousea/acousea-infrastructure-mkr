#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP

#include "Router.h"
#include "bindings/nodeDevice.pb.h"

#include <optional>
#include <unordered_map>

#include "ProtoUtils/ProtoUtils.hpp"


// #define MODULE_PROXY_CACHE_IN_RAM_ENABLED

#ifndef MODULE_PROXY_CACHE_IN_RAM_ENABLED
#include "StorageManager/StorageManager.hpp"
#include "RTCController.hpp"
#endif

/**
 * @brief Proporciona una interfaz genérica para solicitar y enviar configuraciones
 *        de módulos al backend (Raspberry Pi u otro nodo), manteniendo una caché local.
 */
class ModuleProxy
{
    CLASS_NAME(ModuleProxy)

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
                         const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap,
                         StorageManager& storageManager,
                         RTCController& rtcController
    );
    bool begin();
    bool requestMultipleModules(const acousea_ModuleCode* codes, pb_size_t count, DeviceAlias alias);
#endif


    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrRequestFromDevice(
        acousea_ModuleCode code,
        DeviceAlias alias
    );

    const acousea_ModuleWrapper* getIfFresh(acousea_ModuleCode code);
    void invalidateMultiple(const acousea_ModuleCode* codes, pb_size_t count);

    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrSetOnDevice(acousea_ModuleCode code,
                                                                       const acousea_ModuleWrapper& module,
                                                                       DeviceAlias alias);
    bool storeMultipleModules(const acousea_ModuleWrapper* const* wrappers, pb_size_t count);

    // ===================== Send / Receive =====================
    [[nodiscard]] bool requestModule(acousea_ModuleCode code, DeviceAlias alias);

    [[nodiscard]] bool sendModule(const acousea_ModuleWrapper& module, DeviceAlias alias);

    [[nodiscard]] bool storeModule(const acousea_ModuleWrapper& wrapper);
    bool isModuleFresh(acousea_ModuleCode code) const;

private:
    Router& router;
    // ===================== Mapeo alias -> puerto =====================
    const std::unordered_map<DeviceAlias, IPort::PortType> devicePortMap{
        {DeviceAlias::PIDevice, IPort::PortType::SerialPort},
        {DeviceAlias::VR2C, IPort::PortType::SerialPort}
    };
#ifdef MODULE_PROXY_CACHE_IN_RAM_ENABLED
    static constexpr const auto MAX_MODULES = ProtoUtils::ACOUSEA_MAX_MODULE_COUNT;
    std::optional<acousea_ModuleWrapper> entries[MAX_MODULES] = {}; // Initialized to std::nullopt
#else
    static constexpr uint8_t START_BYTE = 0xAA;
    static constexpr uint8_t END_BYTE = 0x55;
    uint64_t writeOffset_[_acousea_ModuleCode_MAX + 1]{}; // Current write offsets for each port (1-based index)
    uint64_t readOffset_[_acousea_ModuleCode_MAX + 1]{}; // Current read offsets for each port (1-based index)
    StorageManager& storage_;
    RTCController& rtc_;
    std::optional<acousea_ModuleWrapper> optLoadedModule_ = std::nullopt; // Módulo cargado actualmente en memoria
#endif


    [[nodiscard]] IPort::PortType resolvePort(DeviceAlias alias) const noexcept;

    // ===================== Construcción de paquetes =====================
    [[nodiscard]] static acousea_CommunicationPacket& buildRequestModulePacket(
        const acousea_ModuleCode* codes, pb_size_t count);

    [[nodiscard]] static acousea_CommunicationPacket& buildSetModulePacket(
        acousea_ModuleCode code, const acousea_ModuleWrapper& module);

    void invalidateModule(acousea_ModuleCode code);
    void invalidateAll();

#ifdef UNIT_TESTING
    friend class ModuleProxyTest_SendModuleBuildsValidPacket_Test;
#endif
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
