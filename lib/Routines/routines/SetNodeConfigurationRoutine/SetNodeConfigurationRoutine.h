#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "ICListenService/ICListenService.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


class SetNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    std::optional<ICListenService*> icListenService;

public:
    CLASS_NAME(SetNodeConfigurationRoutine)

    explicit SetNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        std::optional<ICListenService*> icListenService
    );

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& inPacket) override;

private:
    static Result<void> setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                          const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry
    );
    [[nodiscard]] static Result<void> setReportTypesModule(acousea_NodeConfiguration& nodeConfig,
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
