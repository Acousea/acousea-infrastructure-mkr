#ifndef REPORTING_PERIOD_MANAGER_H
#define REPORTING_PERIOD_MANAGER_H


#include "generated/nodeDevice.pb.h"
#include "ErrorHandler/ErrorHandler.h"
#include "StorageManager/StorageManager.hpp"
#include "Logger/Logger.h"
#include "Result/Result.h"
#include <pb_encode.h>
#include <pb_decode.h>


class NodeConfigurationRepository{
public:
    NodeConfigurationRepository(StorageManager& sdManager, const char* filePath);

    void init();

    void reset();

    void printNodeConfiguration(acousea_NodeConfiguration configuration) const;

    [[nodiscard]] acousea_NodeConfiguration getNodeConfiguration() const;
    bool saveConfiguration(const acousea_NodeConfiguration& configuration);

private:
    [[nodiscard]] static acousea_NodeConfiguration makeDefault();
    static Result<std::vector<uint8_t>> encodeProto(const acousea_NodeConfiguration& m);
    static Result<acousea_NodeConfiguration> decodeProto(const std::vector<uint8_t>& bytes);

private:
    StorageManager& storageManager;
    const char* configFilePath;
};

#endif // REPORTING_PERIOD_MANAGER_H
