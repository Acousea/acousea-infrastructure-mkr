#ifndef ICLISTEN_SERVICE_H
#define ICLISTEN_SERVICE_H

#include <optional>
#include <memory>

#include <Payload/Modules/pamModules/ICListen/ICListenHF.h>
#include <Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLoggingConfig.h>
#include <Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h>
#include <Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h>
#include <Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h>
#include <Result/Result.h>
#include <Packets/iclisten/FetchICListenConfigPacket.h>
#include <Packets/iclisten/SetICListenConfigPacket.h>

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

        // -------------------------- SETTERS --------------------------
        void setICListenStatus(const ICListenStatus &ic_listen_status) const;

        void setICListenLoggingConfig(const ICListenLoggingConfig &ic_listen_logging_config) const;

        void setICListenStreamingConfig(const ICListenStreamingConfig &ic_listen_streaming_config) const;

        void setICListenRecordingStats(const ICListenRecordingStats &ic_listen_recording_stats) const;

        void setICListenHFConfiguration(const ICListenHF &ic_listen_hf) const;
    };

    class Cache {
    public:
        // Getters
        [[nodiscard]] Result<ICListenStatus> retrieveICListenStatus() const;

        [[nodiscard]] Result<ICListenLoggingConfig> retrieveICListenLoggingConfig() const;

        [[nodiscard]] Result<ICListenStreamingConfig> retrieveICListenStreamingConfig() const;

        [[nodiscard]] Result<ICListenRecordingStats> retrieveICListenRecordingStats() const;

        [[nodiscard]] Result<ICListenHF> getHFCompleteConfiguration() const;

        // Setters
        void storeICListenStatus(const ICListenStatus &ic_listen_status) const;

        void storeICListenLoggingConfig(const ICListenLoggingConfig &ic_listen_logging_config) const;

        void storeICListenStreamingConfig(const ICListenStreamingConfig &ic_listen_streaming_config) const;

        void storeICListenRecordingStats(const ICListenRecordingStats &ic_listen_recording_stats) const;

        void storeHFConfiguration(const ICListenHF &hfConfiguration) const;

    private:
        mutable std::optional<ICListenStatus> icListenStatus;
        mutable std::optional<ICListenLoggingConfig> icListenLoggingConfig;
        mutable std::optional<ICListenStreamingConfig> icListenStreamingConfig;
        mutable std::optional<ICListenRecordingStats> icListenRecordingStats;
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
