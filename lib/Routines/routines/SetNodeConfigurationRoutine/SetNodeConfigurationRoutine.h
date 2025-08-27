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

    Result<acousea_CommunicationPacket> execute(const std::optional<_acousea_CommunicationPacket>& optPacket) override;
    Result<void> setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                   const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& item);

private:
    [[nodiscard]] static Result<void> setOperationModesGraph(acousea_NodeConfiguration& nodeConfig,
                                          const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& item);
    [[nodiscard]] static Result<void> setReportingPeriods(acousea_NodeConfiguration& nodeConfig,
                                            const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry);


    // ---------------------- ICListen specific methods ----------------------
    [[nodiscard]] Result<void> setNewICListenConfiguration(uint8_t sender,
                                                           const
                                                           acousea_SetNodeConfigurationPayload_ModulesToChangeEntry&
                                                           module) const;
    void sendNewConfigurationToICListen(const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry) const;
    void storeIcListenConfiguration(const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry) const;
};

#endif // CHANGE_MODE_ROUTINE_H
