#include "SetNodeConfigurationRoutine.h"

#include <utility>

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    std::optional<std::shared_ptr<ICListenService>> icListenService
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(std::move(icListenService))
{
}


Result<acousea_CommunicationPacket> SetNodeConfigurationRoutine::execute(
    const std::optional<_acousea_CommunicationPacket>& optPacket)
{
    if (!optPacket.has_value())
    {
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + ": No packet provided");
    }
    const acousea_CommunicationPacket packet = optPacket.value();

    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    if (!packet.has_payload)
    {
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + ": Packet does not contain payload");
    }

    if (packet.payload.which_payload != acousea_PayloadWrapper_setConfiguration_tag)
    {
        return Result<acousea_CommunicationPacket>::failure(
            "Packet payload is not of type acousea_PayloadWrapper_setConfiguration_tag");
    }

    const acousea_SetNodeConfigurationPayload configurationPayload = packet.payload.payload.setConfiguration;

    for (size_t moduleIdx = 0; moduleIdx < configurationPayload.modulesToChange_count; ++moduleIdx)
    {
        const auto& module = configurationPayload.modulesToChange[moduleIdx];
        if (!module.has_value)
        {
            Logger::logError("Module with key: " + std::to_string(module.key) + " has no value. Skipping.");
            continue;
        }

        switch (module.key)
        {
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_MODULE:
            {
                if (const auto result = setOperationModes(nodeConfig, module); result.isError())
                {
                    return Result<acousea_CommunicationPacket>::failure(result.getError());
                }
                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_GRAPH_MODULE:
            {
                if (const auto result = setOperationModesGraph(nodeConfig, module); result.isError())
                {
                    return Result<acousea_CommunicationPacket>::failure(result.getError());
                }
                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE:
            {
                if (const auto result = setReportingPeriods(nodeConfig, module); result.isError())
                {
                    return Result<acousea_CommunicationPacket>::failure(result.getError());
                }

                break;
            }

        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF:
            {
                // If the node does not have an ICListenService, we cannot process ICListen configurations
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() +
                        ": Node does not have ICListenService. Cannot process ICListen configuration."
                    );
                    break;
                }
                const auto result = setNewICListenConfiguration(packet.routing.sender, module);
                if (result.isError())
                {
                    return Result<acousea_CommunicationPacket>::failure(result.getError());
                }
                if (result.isPending())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Waiting for ICListen confirmation");
                }
                Logger::logInfo(getClassNameString() + "New ICListen configuration processed.");
                break;
            }

        default:
            return Result<acousea_CommunicationPacket>::failure("Invalid TagType in SetNodeConfigurationPayload. Key=" +
                std::to_string(module.key)
            );
        }
    }

    // Important:  Store the updated configuration
    nodeConfigurationRepository.saveConfiguration(nodeConfig);

    return Result<acousea_CommunicationPacket>::success(packet);
}

Result<void> SetNodeConfigurationRoutine::setOperationModes(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& item)
{
    if (!item.has_value || item.value.which_module != acousea_ModuleWrapper_operationModes_tag)
    {
        const auto errorStr = "OperationModesModule with key: " + std::to_string(item.key) +
            " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }
    const auto operationModesModule = item.value.module.operationModes;
    nodeConfig.operationModesModule = operationModesModule;
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setOperationModesGraph(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& item)
{
    if (!item.has_value || item.value.which_module != acousea_ModuleWrapper_operationModesGraph_tag)
    {
        const auto errorStr = "OperationModesGraphModule with key: " + std::to_string(item.key) +
            " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }
    const auto operationModesModule = item.value.module.operationModesGraph;
    nodeConfig.operationGraphModule = operationModesModule;
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setReportingPeriods(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry
)
{
    if (!entry.has_value)
    {
        const auto errorStr = "ReportingModule with key: " + std::to_string(entry.key) + " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }

    switch (entry.value.which_module)
    {
    case acousea_ModuleWrapper_loraReporting_tag:
        {
            const auto reportingModule = entry.value.module.loraReporting;
            nodeConfig.loraModule = reportingModule;
            break;
        }
    case acousea_ModuleWrapper_iridiumReporting_tag:
        {
            const auto reportingModule = entry.value.module.iridiumReporting;
            nodeConfig.iridiumModule = reportingModule;
            break;
        }
    default:
        {
            const auto errorStr = "ReportingModule with key: " + std::to_string(entry.key) +
                " has invalid type (NOT LORA NOR IRIDIUM). Skipping.";
            Logger::logError(errorStr);
            return Result<void>::failure(errorStr);
        }
    }
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setNewICListenConfiguration(
    const uint8_t sender, const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& module) const
{
    // TO CHECK IF THE CONFIGURATION IS CONFIRMED BY THE ICLISTEN, WE CHECK THE SENDER.
    // -  If the sender is the ICListen it should have BROADCAST address, we store the ICLISTEN configuration
    // -  If the sender is not the ICListen (i.e. a remote user) changing the configuration remotely):
    //     1. We then send the new configuration to the ICLISTEN device.
    //     2. We clear the ICLISTEN configuration from the response packet to avoid incorrectly indicating
    //     that the ICLISTEN has confirmed the change.
    if (sender == Router::broadcastAddress)
    {
        storeIcListenConfiguration(module);
        return Result<void>::success();
    }

    sendNewConfigurationToICListen(module);
    return Result<void>::pending("Waiting for ICListen confirmation");
}

void SetNodeConfigurationRoutine::storeIcListenConfiguration(
    const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry
) const
{
    if (!icListenService.has_value())
    {
        Logger::logError(getClassNameString() + ": Node does not have ICListenService. Cannot store configuration.");
        return;
    }

    if (!entry.has_value)
    {
        Logger::logError("ICListenModule with key: " + std::to_string(entry.key) + " has no value. Skipping.");
        return;
    }

    switch (entry.value.which_module)
    {
    case acousea_ModuleWrapper_icListenLoggingConfig_tag:
        {
            Logger::logInfo(getClassNameString() + ": Storing ICListen Logging Config from device.");
            icListenService.value()->getCache()->storeICListenLoggingConfig(entry.value.module.icListenLoggingConfig);
            break;
        }
    case acousea_ModuleWrapper_icListenStreamingConfig_tag:
        {
            // Example: store streaming config
            icListenService.value()->getCache()->storeICListenStreamingConfig(
                entry.value.module.icListenStreamingConfig);
            Logger::logInfo(getClassNameString() + ": Storing ICListen Streaming Config from device.");
            break;
        }
    case acousea_ModuleWrapper_icListenHF_tag:
        {
            // Example: store HF config
            Logger::logInfo(getClassNameString() + ": Storing ICListen HF Config from device.");
            const acousea_ICListenLoggingConfig loggingConfig = entry.value.module.icListenHF.loggingConfig;
            const acousea_ICListenStreamingConfig streamingConfig = entry.value.module.icListenHF.streamingConfig;
            icListenService.value()->getCache()->storeICListenLoggingConfig(loggingConfig);
            icListenService.value()->getCache()->storeICListenStreamingConfig(streamingConfig);
            break;
        }
    default:
        Logger::logError(
            getClassNameString() + ": ICListenModule with key: " + std::to_string(entry.key) +
            " has invalid type. Skipping.");
        break;
    }
}

void SetNodeConfigurationRoutine::sendNewConfigurationToICListen(
    const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& entry
) const
{
    if (!icListenService.has_value())
    {
        Logger::logError(getClassNameString() + ": Node does not have ICListenService. Cannot send configuration.");
        return;
    }
    if (!entry.has_value)
    {
        Logger::logError("ICListenModule with key: " + std::to_string(entry.key) + " has no value. Skipping.");
        return;
    }

    if (entry.value.which_module != acousea_ModuleWrapper_icListenLoggingConfig_tag &&
        entry.value.which_module != acousea_ModuleWrapper_icListenStreamingConfig_tag &&
        entry.value.which_module != acousea_ModuleWrapper_icListenHF_tag)
    {
        Logger::logError(
            "ICListenModule with key: " + std::to_string(entry.key) +
            " has invalid type (NOT LOGGING, NOT STREAMING, NOT HF). Skipping.");
        return;
    }

    switch (entry.value.which_module)
    {
    case acousea_ModuleWrapper_icListenLoggingConfig_tag:
        {
            const auto loggingConfig = entry.value.module.icListenLoggingConfig;
            icListenService.value()->getRequester()->sendLoggingConfig(loggingConfig);
        }

    case acousea_ModuleWrapper_icListenStreamingConfig_tag:
        {
            const auto streamingConfig = entry.value.module.icListenStreamingConfig;
            icListenService.value()->getRequester()->sendStreamingConfig(streamingConfig);
        }

    case acousea_ModuleWrapper_icListenHF_tag:
        {
            const auto hfConfig = entry.value.module.icListenHF;
            if (hfConfig.has_loggingConfig)
            {
                Logger::logInfo("Sending ICListen HF Logging Config");
                icListenService.value()->getRequester()->sendLoggingConfig(hfConfig.loggingConfig);
            }
            if (hfConfig.has_streamingConfig)
            {
                Logger::logInfo("Sending ICListen HF Streaming Config");
                icListenService.value()->getRequester()->sendStreamingConfig(hfConfig.streamingConfig);
            }
        }

    default:
        {
            Logger::logError(
                "ReportingModule with key: " + std::to_string(entry.key) +
                " has invalid type (NOT LORA NOR IRIDIUM). Skipping.");
        }
    }
}
