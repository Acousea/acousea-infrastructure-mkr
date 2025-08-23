#ifndef ICLISTEN_SERVICE_H
#define ICLISTEN_SERVICE_H

#include <optional>
#include <memory>
#include <Result/Result.h>

#include <Router.h>

class ICListenService {
public:
    // Requester: Handles sequential requests
    class Requester {
        Router &router;

    public:
        Requester(Router &router) : router(router) {
        }


        // -------------------------- GETTERS --------------------------
        void fetchStatus() const;

        void fetchLoggingConfig() const;

        void fetchStreamingConfig() const;

        void fetchRecordingStats() const;

        void fetchHFConfiguration() const;

    private:
        static acousea_CommunicationPacket buildFetchICListenConfigPacket(acousea_ModuleCode code);
    };

    class Cache {
    public:
        // Getters
        [[nodiscard]] Result<acousea_ICListenStatus> retrieveICListenStatus() const;

        [[nodiscard]] Result<acousea_ICListenLoggingConfig> retrieveICListenLoggingConfig() const;

        [[nodiscard]] Result<acousea_ICListenStreamingConfig> retrieveICListenStreamingConfig() const;

        [[nodiscard]] Result<acousea_ICListenRecordingStats> retrieveICListenRecordingStats() const;

        [[nodiscard]] Result<acousea_ICListenHF> getHFCompleteConfiguration() const;

        // Setters
        void storeICListenStatus(const acousea_ICListenStatus &ic_listen_status) const;

        void storeICListenLoggingConfig(const acousea_ICListenLoggingConfig &ic_listen_logging_config) const;

        void storeICListenStreamingConfig(const acousea_ICListenStreamingConfig &ic_listen_streaming_config) const;

        void storeICListenRecordingStats(const acousea_ICListenRecordingStats &ic_listen_recording_stats) const;

        void storeHFConfiguration(const acousea_ICListenHF &hfConfiguration) const;

    private:
        mutable std::optional<acousea_ICListenStatus> icListenStatus;
        mutable std::optional<acousea_ICListenLoggingConfig> icListenLoggingConfig;
        mutable std::optional<acousea_ICListenStreamingConfig> icListenStreamingConfig;
        mutable std::optional<acousea_ICListenRecordingStats> icListenRecordingStats;
    };

    explicit ICListenService(Router &router)
        : cache(std::make_shared<Cache>()), requester(std::make_shared<Requester>(router)) {
    }

    // Métodos para acceder directamente a Cache y Requester
    [[nodiscard]] std::shared_ptr<Cache> getCache() const {
        return cache;
    }

    [[nodiscard]] std::shared_ptr<Requester> getRequester() const {
        return requester;
    }

private:
    std::shared_ptr<Cache> cache;
    std::shared_ptr<Requester> requester;
};


#endif //ICLISTEN_SERVICE_H
