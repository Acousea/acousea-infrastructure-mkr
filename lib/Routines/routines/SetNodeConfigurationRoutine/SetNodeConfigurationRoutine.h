#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "ICListenService/ICListenService.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Logger/Logger.h"


class SetNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
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

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& inPacket) override;

private:
    Result<void> setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                   const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry
    );
    [[nodiscard]] static Result<void> setOperationModesGraph(acousea_NodeConfiguration& nodeConfig,
                                                             const acousea_SetNodeConfigurationPayload_ModulesEntry&
                                                             moduleEntry
    );

    [[nodiscard]] static Result<void> setReportingPeriods(acousea_NodeConfiguration& nodeConfig,
                                                          const acousea_SetNodeConfigurationPayload_ModulesEntry&
                                                          moduleEntry
    );

    [[nodiscard]] Result<void> setICListenConfiguration(
        const acousea_SetNodeConfigurationPayload_ModulesEntry& entry) const;
};


#endif // CHANGE_MODE_ROUTINE_H
