#ifndef REPORTING_PERIOD_MANAGER_H
#define REPORTING_PERIOD_MANAGER_H


#include <string>
#include "generated/nodeDevice.pb.h"
#include "ErrorHandler/ErrorHandler.h"
#include "StorageManager/StorageManager.hpp"
#include "Logger/Logger.h"
#include "Result/Result.h"
#include <pb_encode.h>
#include <pb_decode.h>


class NodeConfigurationRepository{
public:
    explicit NodeConfigurationRepository(StorageManager& sdManager);

    void init();

    void reset();

    void printNodeConfiguration(const acousea_NodeConfiguration& cfg) const;


    [[nodiscard]] acousea_NodeConfiguration getNodeConfiguration() const;
    bool saveConfiguration(const acousea_NodeConfiguration& configuration);

private:
    [[nodiscard]] static acousea_NodeConfiguration makeDefault();
    static Result<std::vector<uint8_t>> encodeProto(const acousea_NodeConfiguration& m);
    static Result<acousea_NodeConfiguration> decodeProto(const std::vector<uint8_t>& bytes);

private:
    StorageManager& storageManager;
    inline static const char* configFilePath = "nodeconf"; // MAX 8 chars for 8.3 filenames
};

#endif // REPORTING_PERIOD_MANAGER_H
