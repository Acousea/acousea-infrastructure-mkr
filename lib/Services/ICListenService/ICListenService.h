#ifndef ICLISTEN_SERVICE_H
#define ICLISTEN_SERVICE_H


#include "Result.h"
#include <Router.h>


#include "ClassName.h"
#include "nodeDevice.pb.h"


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
    private:
        T value{};
        bool hasValue{false};
        bool isFresh{false};

    public:
        CachedValue() = default;

        explicit CachedValue(const T& v, const bool fresh = true)
            : value(v), hasValue(true), isFresh(fresh)
        {
        }

        void store(const T& v)
        {
            value = v;
            hasValue = isFresh = true;
        }

        void invalidate() { isFresh = false; }

        [[nodiscard]] const T& get() const { return value; }
        [[nodiscard]] bool valid() const { return hasValue; }
        [[nodiscard]] bool fresh() const { return isFresh; }
    };


    class Cache
    {
    public:
        Cache() = default;
        // Getters
        [[nodiscard]] CachedValue<acousea_ICListenStatus> getICListenStatus() const;

        [[nodiscard]] CachedValue<acousea_ICListenLoggingConfig> getICListenLoggingConfig() const;

        [[nodiscard]] CachedValue<acousea_ICListenStreamingConfig> getICListenStreamingConfig() const;

        [[nodiscard]] CachedValue<acousea_ICListenRecordingStats> getICListenRecordingStats() const;

        [[nodiscard]] CachedValue<acousea_ICListenHF> getICListenCompleteConfiguration() const;

        // Setters
        void storeICListenStatus(const acousea_ICListenStatus& ic_listen_status);

        void storeICListenLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config);

        void storeICListenStreamingConfig(const acousea_ICListenStreamingConfig& ic_listen_streaming_config);

        void storeICListenRecordingStats(const acousea_ICListenRecordingStats& ic_listen_recording_stats);

        void storeICListenHFConfiguration(const acousea_ICListenHF& hfConfiguration);

        // --- Invalidadores (const; modifican miembros mutable) ---
        void invalidateStatus() const;
        void invalidateLogging() const;
        void invalidateStreaming() const;
        void invalidateRecordingStats() const;

        void invalidateAll() const;

    private:
        mutable CachedValue<acousea_ICListenStatus> icListenStatus;
        mutable CachedValue<acousea_ICListenLoggingConfig> icListenLoggingConfig;
        mutable CachedValue<acousea_ICListenStreamingConfig> icListenStreamingConfig;
        mutable CachedValue<acousea_ICListenRecordingStats> icListenRecordingStats;
    };

public:
    // Métodos para acceder directamente a Cache y Requester
    [[nodiscard]] Cache& getCache() { return cache; }
    // [[nodiscard]] const Cache& getCache() const { return cache; }
    // [[nodiscard]] Cache getCache() const  { return cache; }


private:
    Router& router;
    Cache cache;

private:
    static acousea_CommunicationPacket buildFetchICListenConfigPacket(acousea_ModuleCode code);
    static Result<std::vector<uint8_t>> encodeICListenHF(const acousea_ICListenHF& hfConfiguration);
    static Result<acousea_ICListenHF> decodeICListenHF(const std::vector<uint8_t>& buffer);
};


#endif //ICLISTEN_SERVICE_H
