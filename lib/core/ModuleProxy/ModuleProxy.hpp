#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP

#include "Router.h"
#include "bindings/nodeDevice.pb.h"

#include <optional>
#include <unordered_map>

#include "ProtoUtils/ProtoUtils.hpp"

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
    explicit ModuleProxy(Router& router);

    ModuleProxy(Router& router, const std::unordered_map<DeviceAlias, IPort::PortType>& devicePortMap);

    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrRequestFromDevice(
        acousea_ModuleCode code,
        DeviceAlias alias
    );

    [[nodiscard]] const acousea_ModuleWrapper* getIfFreshOrSetOnDevice(acousea_ModuleCode code,
                                                                 const acousea_ModuleWrapper& module,
                                                                 DeviceAlias alias);

    // ===================== Send / Receive =====================
    bool requestModule(acousea_ModuleCode code, DeviceAlias alias);

    bool sendModule(acousea_ModuleCode code, const acousea_ModuleWrapper& module, DeviceAlias alias);

    // ===================== Caché =====================
    class CachedValue
    {
    public:
        CachedValue();
        explicit CachedValue(const acousea_ModuleWrapper& v, bool hasValue = true, bool fresh = true);
        void store(const acousea_ModuleWrapper& v);
        void invalidate();
        static CachedValue empty();
        [[nodiscard]] bool isFresh() const;
        [[nodiscard]] const acousea_ModuleWrapper& get() const;

    private:
        acousea_ModuleWrapper value{};
        bool isFresh_{false};
    };


    class ModuleCache
    {
    public:
        struct Entry
        {
            CachedValue value{};
            bool occupied{false};
        };


        [[nodiscard]] bool store(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper);
        [[nodiscard]] CachedValue get(acousea_ModuleCode code) const;
        [[nodiscard]] const acousea_ModuleWrapper* getIfFresh(acousea_ModuleCode code) const;
        void invalidateModule(acousea_ModuleCode code);
        void invalidateAll();
        [[nodiscard]] bool isFresh(acousea_ModuleCode code) const;
        [[nodiscard]] ModuleCache clone() const;
        void swap(ModuleCache& other) noexcept;
        friend void swap(ModuleCache& a, ModuleCache& b) noexcept;

    private:
        static constexpr const auto MAX_MODULES = ProtoUtils::ACOUSEA_MAX_MODULE_COUNT;
        Entry entries[MAX_MODULES]{};
    };

    [[nodiscard]] ModuleCache& getCache() { return cache; }
    [[nodiscard]] const ModuleCache& getCache() const { return cache; }

private:
    Router& router;
    ModuleCache cache;

    // ===================== Mapeo alias -> puerto =====================
    const std::unordered_map<DeviceAlias, IPort::PortType> devicePortMap{
        {DeviceAlias::PIDevice, IPort::PortType::SerialPort},
        {DeviceAlias::VR2C, IPort::PortType::SerialPort}
    };

    IPort::PortType resolvePort(DeviceAlias alias) const noexcept;

    // ===================== Construcción de paquetes =====================
    [[nodiscard]] static acousea_CommunicationPacket buildRequestPacket(acousea_ModuleCode code);

    friend class ModuleProxyTest_SendModuleBuildsValidPacket_Test;
    [[nodiscard]] static acousea_CommunicationPacket buildSetPacket(acousea_ModuleCode code,
                                                                    const acousea_ModuleWrapper& module);
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
