#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP

#include "Router.h"
#include "bindings/nodeDevice.pb.h"

#include <optional>
#include <unordered_map>

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
        ICListen,
        VR2C
    };

    static constexpr const char *toString(DeviceAlias alias) noexcept;

    // ============================================================
    // Constructors
    // ============================================================
    explicit ModuleProxy(Router &router);

    ModuleProxy(Router &router, const std::unordered_map<DeviceAlias, IPort::PortType> &devicePortMap);

    // ===================== Send / Receive =====================
    bool requestModule(acousea_ModuleCode code, DeviceAlias alias) const;

    bool sendModule(acousea_ModuleCode code, const acousea_ModuleWrapper &module, DeviceAlias alias) const;

    // ===================== Caché =====================
    class CachedValue
    {
    public:
        CachedValue() = default;

        explicit CachedValue(const acousea_ModuleWrapper &v, bool hasValue = true, bool fresh = true)
            : value(v), hasValue(hasValue), isFresh(fresh)
        {
        }

        void store(const acousea_ModuleWrapper &v)
        {
            value = v;
            hasValue = isFresh = true;
        }

        void invalidate() { isFresh = false; }

        static CachedValue empty()
        {
            return {}; // usa el constructor por defecto
        }

        [[nodiscard]] bool valid() const { return hasValue; }
        [[nodiscard]] bool fresh() const { return hasValue && isFresh; }
        [[nodiscard]] const acousea_ModuleWrapper &get() const { return value; }

    private:
        acousea_ModuleWrapper value{};
        bool hasValue{false};
        bool isFresh{false};
    };

    class ModuleCache
    {
    public:
        void store(acousea_ModuleCode code, const acousea_ModuleWrapper &wrapper);
        [[nodiscard]] CachedValue get(acousea_ModuleCode code) const ;
        [[nodiscard]]  std::optional<acousea_ModuleWrapper> getIfFresh(acousea_ModuleCode code) const;
        void invalidate(acousea_ModuleCode code);
        void invalidateAll();
        [[nodiscard]] bool isFresh(acousea_ModuleCode code) const;
        [[nodiscard]] ModuleCache clone() const;
        void swap(ModuleCache &other) noexcept;
        friend void swap(ModuleCache& a, ModuleCache& b) noexcept;

    private:
        std::map<acousea_ModuleCode, CachedValue> cache;
    };

    [[nodiscard]] ModuleCache &getCache() { return cache; }
    [[nodiscard]] const ModuleCache &getCache() const { return cache; }

private:
    Router &router;
    ModuleCache cache;

    // ===================== Mapeo alias -> puerto =====================
    const std::unordered_map<DeviceAlias, IPort::PortType> devicePortMap{
        {DeviceAlias::ICListen, IPort::PortType::SerialPort},
        {DeviceAlias::VR2C, IPort::PortType::SerialPort}};

    std::optional<IPort::PortType> resolvePort(DeviceAlias alias) const;

    // ===================== Construcción de paquetes =====================
    [[nodiscard]] static acousea_CommunicationPacket buildRequestPacket(acousea_ModuleCode code);

    friend class ModuleProxyTest_SendModuleBuildsValidPacket_Test;
    [[nodiscard]] static acousea_CommunicationPacket buildSetPacket(acousea_ModuleCode code,
                                                                    const acousea_ModuleWrapper &module);
};

#endif // ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
