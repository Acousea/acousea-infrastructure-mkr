#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "ICListenService/ICListenService.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Logger/Logger.h"


class SetNodeConfigurationRoutine : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    std::optional<std::shared_ptr<ICListenService>> icListenService;

public:
    CLASS_NAME(SetNodeConfigurationRoutine)

    explicit SetNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        std::optional<std::shared_ptr<ICListenService>> icListenService
    );

    Result<acousea_CommunicationPacket> execute(const acousea_CommunicationPacket& packet) override;

private:
    static void setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                  const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& item);
    static void setReportingPeriods(acousea_NodeConfiguration& nodeConfig,
                                    const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry);
    void sendNewConfigurationToICListen(const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry) const;
    void storeIcListenConfiguration(const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry) const;
};

#endif // CHANGE_MODE_ROUTINE_H
