#ifndef NODE_CONFIGURATION_REPOSITORY_H
#define NODE_CONFIGURATION_REPOSITORY_H

#include "bindings/nodeDevice.pb.h"
#include "StorageManager/StorageManager.hpp"
#include "Result.h"
#include <string>
#include "ClassName.h"


class NodeConfigurationRepository
{
public:
    CLASS_NAME(NodeConfigurationRepository)

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

#ifdef UNIT_TESTING
private:
    friend class TestableNodeConfigurationRepository;

#endif
};

#endif // NODE_CONFIGURATION_REPOSITORY_H
