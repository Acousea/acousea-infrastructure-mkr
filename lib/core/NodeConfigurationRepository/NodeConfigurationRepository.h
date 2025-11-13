#ifndef NODE_CONFIGURATION_REPOSITORY_H
#define NODE_CONFIGURATION_REPOSITORY_H

#include "bindings/nodeDevice.pb.h"
#include "StorageManager/StorageManager.hpp"
#include "ClassName.h"

class NodeConfigurationRepository
{
public:
    CLASS_NAME(NodeConfigurationRepository)

    explicit NodeConfigurationRepository(StorageManager& sdManager);

    void init();

    void reset();

    static void printNodeConfiguration(const acousea_NodeConfiguration& cfg);

    [[nodiscard]] acousea_NodeConfiguration& getNodeConfiguration() const;

    bool saveConfiguration(const acousea_NodeConfiguration& configuration);

private:
    [[nodiscard]] static acousea_NodeConfiguration& makeDefault();

private:
    StorageManager& storageManager;
    inline static const char* configFilePath = "nodeconf"; // MAX 8 chars for 8.3 filenames

#ifdef UNIT_TESTING
private:
    friend class TestableNodeConfigurationRepository;

#endif
};

#endif // NODE_CONFIGURATION_REPOSITORY_H
