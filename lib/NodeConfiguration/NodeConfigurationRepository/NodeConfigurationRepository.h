#ifndef REPORTING_PERIOD_MANAGER_H
#define REPORTING_PERIOD_MANAGER_H


#include "../NodeConfiguration/NodeConfiguration.h"


class NodeConfigurationRepository {
public:
    NodeConfigurationRepository(SDManager &sdManager, const char *filePath);

    bool begin();

    bool reset();

    void printData() const;

    [[nodiscard]] NodeConfiguration getNodeConfiguration() const;

    bool saveConfiguration(const NodeConfiguration &configuration);

private:
    SDManager &sdManager;
    const char *filePath;
};

#endif // REPORTING_PERIOD_MANAGER_H
