#ifndef ICLISTEN_SERVICE_H
#define ICLISTEN_SERVICE_H

#include <memory>
#include <variant>
#include <Result/Result.h>
#include <Router.h>
#include "ErrorHandler/ErrorHandler.h"
#include "ClassName.h"
#include "nodeDevice.pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

class ICListenService
{
    CLASS_NAME(ICListenService)

public:
    // Requester: Handles sequential requests
    class Requester
    {
        Router& router;

    public:
        explicit Requester(Router& router) : router(router)
        {
        }


        // -------------------------- GETTERS --------------------------
        void fetchStatus() const;

        void fetchLoggingConfig() const;

        void fetchStreamingConfig() const;

        void fetchRecordingStats() const;

        void fetchHFConfiguration() const;

        // -------------------------- SETTERS --------------------------
        void sendLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config) const;

        void sendStreamingConfig(const acousea_ICListenStreamingConfig& ic_listen_streaming_config) const;

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
            SendICListenConfigModuleCode code, SendICListenConfigModuleValue value
        );
    };

    class Cache
    {
    public:
        explicit Cache(StorageManager* storage_manager);

        // Getters
        [[nodiscard]] Result<acousea_ICListenStatus> retrieveICListenStatus();

        [[nodiscard]] Result<acousea_ICListenLoggingConfig> retrieveICListenLoggingConfig();

        [[nodiscard]] Result<acousea_ICListenStreamingConfig> retrieveICListenStreamingConfig();

        [[nodiscard]] Result<acousea_ICListenRecordingStats> retrieveICListenRecordingStats();

        [[nodiscard]] Result<acousea_ICListenHF> retrieveICListenCompleteConfiguration();

        // Setters
        void storeICListenStatus(const acousea_ICListenStatus& ic_listen_status);

        void storeICListenLoggingConfig(const acousea_ICListenLoggingConfig& ic_listen_logging_config);

        void storeICListenStreamingConfig(const acousea_ICListenStreamingConfig& ic_listen_streaming_config);

        void storeICListenRecordingStats(const acousea_ICListenRecordingStats& ic_listen_recording_stats);

        void storeICListenHFConfiguration(const acousea_ICListenHF& hfConfiguration);

    public:
        static constexpr auto ICLISTEN_CONFIG_STORAGE_FILE = "iclisten_config";

    private:
        void persistHFConfiguration();

    private:
        StorageManager* storageManager; // cada Cache tiene su propio puntero
        mutable std::optional<acousea_ICListenStatus> icListenStatus;
        mutable std::optional<acousea_ICListenLoggingConfig> icListenLoggingConfig;
        mutable std::optional<acousea_ICListenStreamingConfig> icListenStreamingConfig;
        mutable std::optional<acousea_ICListenRecordingStats> icListenRecordingStats;
    };


    ICListenService(Router& router, StorageManager* storageManager);

    // Métodos para acceder directamente a Cache y Requester
    [[nodiscard]] std::shared_ptr<Cache> getCache() const
    {
        return cache;
    }

    [[nodiscard]] std::shared_ptr<Requester> getRequester() const
    {
        return requester;
    }

private:
    std::shared_ptr<Cache> cache;
    std::shared_ptr<Requester> requester;

private:
    static Result<std::vector<uint8_t>> encodeICListenHF(const acousea_ICListenHF& hfConfiguration);
    static Result<acousea_ICListenHF> decodeICListenHF(const std::vector<uint8_t>& buffer);
};


#endif //ICLISTEN_SERVICE_H
