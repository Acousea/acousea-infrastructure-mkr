#ifndef ICLISTEN_SERVICE_H
#define ICLISTEN_SERVICE_H

#include <optional>
#include <memory>
#include <variant>
#include <Result/Result.h>
#include <Router.h>
#include "ErrorHandler/ErrorHandler.h"
#include "ClassName.h"
#include "nodeDevice.pb.h"
#include "pb_decode.h"

class ICListenService
{
    CLASS_NAME(ICListenService)

public:

    explicit ICListenService(Router& router);

    void init() const;

    // -------------------------- GETTERS --------------------------
    void fetchStatus() const;

    void fetchLoggingConfig() const;

    void fetchStreamingConfig() const;

    void fetchRecordingStats() const;

    void fetchHFConfiguration() const;

    // -------------------------- SETTERS --------------------------
    void sendLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config) const;

    void sendStreamingConfig(const acousea_ICListenStreamingConfig& ic_listen_streaming_config) const;


    template <typename T>
    struct CachedValue
    {
        CachedValue(const std::optional<T>& value, bool is_fresh)
            : value(value),
              isFresh(is_fresh)
        {
        }

        // Default constructor
        CachedValue() : CachedValue(std::nullopt, false)
        {
        }

        std::optional<T> value{};
        bool isFresh{false};

        void store(const T& v)
        {
            value = v;
            isFresh = true;
        }

        void invalidate()
        {
            isFresh = false;
        }

        [[nodiscard]] bool hasValue() const
        {
            return value.has_value();
        }

        [[nodiscard]] const T& get() const
        {
            return *value;
        }
    };

    class Cache
    {
    public:
        Cache() = default;
        // Getters
        [[nodiscard]] CachedValue<acousea_ICListenStatus> getICListenStatus();

        [[nodiscard]] CachedValue<acousea_ICListenLoggingConfig> getICListenLoggingConfig();

        [[nodiscard]] CachedValue<acousea_ICListenStreamingConfig> getICListenStreamingConfig();

        [[nodiscard]] CachedValue<acousea_ICListenRecordingStats> getICListenRecordingStats();

        [[nodiscard]] CachedValue<acousea_ICListenHF> getICListenCompleteConfiguration();

        // Setters
        void storeICListenStatus(const acousea_ICListenStatus& ic_listen_status);

        void storeICListenLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config);

        void storeICListenStreamingConfig(const acousea_ICListenStreamingConfig& ic_listen_streaming_config);

        void storeICListenRecordingStats(const acousea_ICListenRecordingStats& ic_listen_recording_stats);

        void storeICListenHFConfiguration(const acousea_ICListenHF& hfConfiguration);

        // --- Invalidadores (const; modifican miembros mutable) ---
        void invalidateStatus() const          { icListenStatus.invalidate(); }
        void invalidateLogging() const         { icListenLoggingConfig.invalidate(); }
        void invalidateStreaming() const       { icListenStreamingConfig.invalidate(); }
        void invalidateRecordingStats() const  { icListenRecordingStats.invalidate(); }
        void invalidateAll() const {
            icListenStatus.invalidate();
            icListenLoggingConfig.invalidate();
            icListenStreamingConfig.invalidate();
            icListenRecordingStats.invalidate();
        }


    private:
        mutable CachedValue<acousea_ICListenStatus> icListenStatus;
        mutable CachedValue<acousea_ICListenLoggingConfig> icListenLoggingConfig;
        mutable CachedValue<acousea_ICListenStreamingConfig> icListenStreamingConfig;
        mutable CachedValue<acousea_ICListenRecordingStats> icListenRecordingStats;
    };

public:
    // Métodos para acceder directamente a Cache y Requester
    [[nodiscard]] Cache* getCache() const
    {
        return cache.get();
    }

private:
    typedef enum _sendICListenConfigModuleCode
    {
        ICLISTEN_HF_LOGGING_CONFIG = acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG,
        ICLISTEN_HF_STREAMING_CONFIG = acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG,
    } SendICListenConfigModuleCode;

    using SendICListenConfigModuleValue = std::variant<acousea_ICListenLoggingConfig,
                                                       acousea_ICListenStreamingConfig>;
    static acousea_CommunicationPacket buildFetchICListenConfigPacket(acousea_ModuleCode code);

    static acousea_CommunicationPacket buildSendICListenConfigPacket(
        SendICListenConfigModuleCode code, SendICListenConfigModuleValue iclistenConfigValue
    );

private:
    Router& router;
    std::unique_ptr<Cache> cache;

private:
    static Result<std::vector<uint8_t>> encodeICListenHF(const acousea_ICListenHF& hfConfiguration);
    static Result<acousea_ICListenHF> decodeICListenHF(const std::vector<uint8_t>& buffer);
};

#include "pb_encode.h"

// Helper genérico: compara dos mensajes nanopb por su codificación
#include "pb.h"        // asegura que pb_msgdesc_t esté visible
#include "pb_encode.h" // pb_get_encoded_size / pb_encode

template <typename T>
static bool nanopb_equal(const T& a, const T& b, const pb_msgdesc_t* fields)
{
    size_t sa = 0, sb = 0;
    if (!pb_get_encoded_size(&sa, fields, &a)) return false;
    if (!pb_get_encoded_size(&sb, fields, &b)) return false;
    if (sa != sb) return false;

    std::vector<uint8_t> ba(sa), bb(sb);

    {
        pb_ostream_t oa = pb_ostream_from_buffer(ba.data(), ba.size());
        if (!pb_encode(&oa, fields, &a)) return false;
    }
    {
        pb_ostream_t ob = pb_ostream_from_buffer(bb.data(), bb.size());
        if (!pb_encode(&ob, fields, &b)) return false;
    }
    return ba == bb;
}


#endif //ICLISTEN_SERVICE_H
