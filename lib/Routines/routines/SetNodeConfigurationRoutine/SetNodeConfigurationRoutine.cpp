#include "SetNodeConfigurationRoutine.h"

#include <utility>

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    std::optional<std::shared_ptr<ICListenService>> icListenService
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(std::move(icListenService)){
}


Result<acousea_CommunicationPacket> SetNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& inPacket){
    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    if (!inPacket.has_value()){
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + ": No packet provided");
    }

    if (inPacket.value().which_body != acousea_CommunicationPacket_command_tag){
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + ": Packet is not of type command");
    }
    if (inPacket.value().body.command.which_command != acousea_CommandBody_setConfiguration_tag){
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + ": Packet command is not of type setNodeConfiguration");
    }

    const auto [modules_count, modules] = inPacket.value().body.command.command.setConfiguration;

    for (size_t i = 0; i < modules_count; ++i){
        const auto& module = modules[i];
        if (!module.has_value){
            Logger::logError("Module with key: " + std::to_string(module.key) + " has no value. Skipping.");
            continue;
        }

        switch (module.key){
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_MODULE: {
            if (const auto result = setOperationModes(nodeConfig, module); result.isError()){
                return Result<acousea_CommunicationPacket>::failure(result.getError());
            }
            break;
        }
        case acousea_ModuleCode::acousea_ModuleCode_REPORTING_TYPES_MODULE: {
            if (const auto result = setReportTypesModule(nodeConfig, module); result.isError()){
                return Result<acousea_CommunicationPacket>::failure(result.getError());
            }

            break;
        }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE: {
            if (const auto result = setReportingPeriods(nodeConfig, module); result.isError()){
                return Result<acousea_CommunicationPacket>::failure(result.getError());
            }

            break;
        }

        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF: {
            const auto setIcListenConfigResult = setICListenConfiguration(module);
            if (setIcListenConfigResult.isError()){
                return Result<acousea_CommunicationPacket>::failure(setIcListenConfigResult.getError());
            }
            if (setIcListenConfigResult.isPending()){
                return Result<acousea_CommunicationPacket>::pending(setIcListenConfigResult.getError());
            }
            Logger::logInfo(getClassNameString() + "New ICListen configuration sent to device.");
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

    acousea_CommunicationPacket outPacket = inPacket.value();
    outPacket.which_body = acousea_CommunicationPacket_response_tag;
    outPacket.body.response.which_response = acousea_ResponseBody_setConfiguration_tag;
    outPacket.body.response.response.setConfiguration = inPacket.value().body.command.command.setConfiguration;


    return Result<acousea_CommunicationPacket>::success(outPacket);
}

Result<void> SetNodeConfigurationRoutine::setOperationModes(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry
){
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_operationModes_tag){
        const auto errorStr = "OperationModesModule with key: " + std::to_string(moduleEntry.key) +
            " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }
    const auto operationModesModule = moduleEntry.value.module.operationModes;
    nodeConfig.operationModesModule = operationModesModule;
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setReportTypesModule(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry){
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_reportTypes_tag){
        const auto errorStr = "ReportTypesModule with key: " + std::to_string(moduleEntry.key) +
            " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }
    const auto reportTypesModule = moduleEntry.value.module.reportTypes;
    nodeConfig.reportTypesModule = reportTypesModule;
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setReportingPeriods(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry
){
    if (!moduleEntry.has_value){
        const auto errorStr = "ReportingModule with key: " + std::to_string(moduleEntry.key) +
            " has no value. Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }

    switch (moduleEntry.value.which_module){
    case acousea_ModuleWrapper_loraReporting_tag: {
        const auto reportingModule = moduleEntry.value.module.loraReporting;
        nodeConfig.loraModule = reportingModule;
        break;
    }
    case acousea_ModuleWrapper_iridiumReporting_tag: {
        const auto reportingModule = moduleEntry.value.module.iridiumReporting;
        nodeConfig.iridiumModule = reportingModule;
        break;
    }
    case acousea_ModuleWrapper_gsmMqttReporting_tag: {
        const auto reportingModule = moduleEntry.value.module.gsmMqttReporting;
        nodeConfig.gsmMqttModule = reportingModule;
        break;
    }
    default: {
        const auto errorStr = "ReportingModule with key: " + std::to_string(moduleEntry.key) +
            " has invalid type (NOT LORA NOR IRIDIUM). Skipping.";
        Logger::logError(errorStr);
        return Result<void>::failure(errorStr);
    }
    }
    return Result<void>::success();
}

Result<void> SetNodeConfigurationRoutine::setICListenConfiguration(
    const acousea_SetNodeConfigurationPayload_ModulesEntry& entry) const{
    // If the node does not have an ICListenService, we cannot process ICListen configurations
    if (!icListenService.has_value()){
        Logger::logError(getClassNameString() + ": Node does not have ICListenService. Cannot send configuration.");
        return Result<void>::failure("No ICListenService available");
    }
    if (!entry.has_value){
        Logger::logError("ICListenModule with key: " + std::to_string(entry.key) + " has no value. Skipping.");
        return Result<void>::failure("No value in ICListen module entry");
    }

    const auto hfService = icListenService.value();

    switch (entry.value.which_module){
    case acousea_ModuleWrapper_icListenLoggingConfig_tag: {
        const auto loggingConfig = entry.value.module.icListenLoggingConfig;
        hfService->sendLoggingConfig(loggingConfig);
    }

    case acousea_ModuleWrapper_icListenStreamingConfig_tag: {
        const auto streamingConfig = entry.value.module.icListenStreamingConfig;
        hfService->sendStreamingConfig(streamingConfig);
    }

    case acousea_ModuleWrapper_icListenHF_tag: {
        const auto hfConfig = entry.value.module.icListenHF;
        if (hfConfig.has_loggingConfig){
            Logger::logInfo("Sending ICListen HF Logging Config");
            hfService->sendLoggingConfig(hfConfig.loggingConfig);
        }
        if (hfConfig.has_streamingConfig){
            Logger::logInfo("Sending ICListen HF Streaming Config");
            hfService->sendStreamingConfig(hfConfig.streamingConfig);
        }
    }

    default: {
        Logger::logError(
            "ReportingModule with key: " + std::to_string(entry.key) +
            " has invalid type (NOT LORA NOR IRIDIUM). Skipping.");
    }
    }


    // Check if the set configuration is all fresh
    if (!hfService->getCache()->getICListenLoggingConfig().isFresh ||
        !hfService->getCache()->getICListenStreamingConfig().isFresh){
        Logger::logInfo(getClassNameString() + ": ICListen configuration is NOT fresh.");
        return Result<void>::pending("ICListen configuration is not fresh");
    }

    Logger::logInfo(getClassNameString() + ": ICListen configuration is fresh.");


    return Result<void>::success();
}
