#ifndef REPORTING_PERIOD_MANAGER_H
#define REPORTING_PERIOD_MANAGER_H


#include "NodeConfiguration/NodeConfiguration.h"
#include "ErrorHandler/ErrorHandler.h"


class NodeConfigurationRepository {
public:
    NodeConfigurationRepository(SDManager &sdManager, const char *filePath);

    void init();

    void reset();

    [[nodiscard]] NodeConfiguration getNodeConfiguration() const;

    bool saveConfiguration(const NodeConfiguration &configuration);

private:
    SDManager &sdManager;
    const char *filePath;
};

#endif // REPORTING_PERIOD_MANAGER_H
