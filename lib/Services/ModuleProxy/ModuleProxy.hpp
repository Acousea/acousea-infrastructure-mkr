#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP


#include "Router.h"
#include "nodeDevice.pb.h"
#include <map>
#include <optional>

/**
 * @brief Proporciona una interfaz genérica para solicitar y enviar configuraciones
 *        de módulos al backend (Raspberry Pi u otro nodo), manteniendo una caché local.
 */
class ModuleProxy
{
public:
    explicit ModuleProxy(Router& router);

    // ===================== Envío / Solicitud =====================
    void requestModule(acousea_ModuleCode code) const;

    template <typename ModuleT>
    void sendModule(acousea_ModuleCode code, const ModuleT& module) const;

    // ===================== Caché =====================
    class CachedValue
    {
    public:
        CachedValue() = default;
        explicit CachedValue(const acousea_ModuleWrapper& v, bool fresh = true)
            : value(v), hasValue(true), isFresh(fresh)
        {}

        void store(const acousea_ModuleWrapper& v)
        {
            value = v;
            hasValue = isFresh = true;
        }

        void invalidate() { isFresh = false; }

        [[nodiscard]] bool valid() const { return hasValue; }
        [[nodiscard]] bool fresh() const { return hasValue && isFresh; }
        [[nodiscard]] const acousea_ModuleWrapper& get() const { return value; }

    private:
        acousea_ModuleWrapper value{};
        bool hasValue{false};
        bool isFresh{false};
    };


    class ModuleCache
    {
    public:
        void store(acousea_ModuleCode code, const acousea_ModuleWrapper& wrapper);
        std::optional<acousea_ModuleWrapper> get(acousea_ModuleCode code) const;
        void invalidate(acousea_ModuleCode code) const;
        void invalidateAll() const;
        bool fresh(acousea_ModuleCode code) const;

    private:
        mutable std::map<acousea_ModuleCode, CachedValue> cache;
    };

    [[nodiscard]] ModuleCache& getCache() { return cache; }
    [[nodiscard]] const ModuleCache& getCache() const { return cache; }

private:
    Router& router;
    ModuleCache cache;

    static acousea_CommunicationPacket buildRequestPacket(acousea_ModuleCode code);

    template <typename ModuleT>
    static acousea_CommunicationPacket buildSetPacket(acousea_ModuleCode code, const ModuleT& module);
};



#endif //ACOUSEA_INFRASTRUCTURE_MKR_MODULEPROXY_HPP
