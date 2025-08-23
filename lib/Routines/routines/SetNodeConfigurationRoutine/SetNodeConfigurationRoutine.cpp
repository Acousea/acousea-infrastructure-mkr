#include "SetNodeConfigurationRoutine.h"

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(NodeConfigurationRepository& nodeConfigurationRepository)
    : IRoutine(getClassNameString()), nodeConfigurationRepository(nodeConfigurationRepository){
}

Result<acousea_CommunicationPacket> SetNodeConfigurationRoutine::execute(const acousea_CommunicationPacket& packet){
    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    if (!packet.has_payload){
        return Result<acousea_CommunicationPacket>::failure("Packet does not contain NewNodeConfigurationPayload");
    }

    if (packet.payload.which_payload != acousea_PayloadWrapper_setConfiguration_tag){
        return Result<acousea_CommunicationPacket>::failure(
            "Packet payload is not of type acousea_PayloadWrapper_setConfiguration_tag");
    }

    const acousea_SetNodeConfigurationPayload configurationPayload = packet.payload.payload.setConfiguration;


    for (const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry& module : configurationPayload.modulesToChange){
        if (!module.has_value){
            Logger::logError("Module with key: " + std::to_string(module.key) + " has no value. Skipping.");
            continue;
        }
        switch (module.key){
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_GRAPH_MODULE: {
            setOperationModes(nodeConfig, module);
            break;
        }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE: {
            setReportingPeriods(nodeConfig, module);
            break;
        }

        default:
            return Result<acousea_CommunicationPacket>::failure("Invalid TagType in NewNodeConfigurationPayload");
        }
    }
    nodeConfigurationRepository.saveConfiguration(nodeConfig);
    return Result<acousea_CommunicationPacket>::success(packet);
}

void SetNodeConfigurationRoutine::setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                                    const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry&
                                                    item){
    if (!item.has_value || item.value.which_module != acousea_ModuleWrapper_operationModesGraph_tag){
        Logger::logError(
            "OperationModesGraphModule with key: " + std::to_string(item.key) + " has no value. Skipping.");
        return;
    }
    const auto operationModesModule = item.value.module.operationModesGraph;
    nodeConfig.operationGraphModule = operationModesModule;
}

void SetNodeConfigurationRoutine::setReportingPeriods(acousea_NodeConfiguration& nodeConfig,
                                                      const acousea_SetNodeConfigurationPayload_ModulesToChangeEntry&
                                                      entry){
    if (!entry.has_value){
        Logger::logError("ReportingModule with key: " + std::to_string(entry.key) + " has no value. Skipping.");
        return;
    }

    if (entry.value.which_module != acousea_ModuleWrapper_loraReporting_tag && entry.value.which_module !=
        acousea_ModuleWrapper_iridiumReporting_tag){
        Logger::logError(
            "ReportingModule with key: " + std::to_string(entry.key) +
            " has invalid type (NOT LORA NOR IRIDIUM). Skipping.");
        return;
    }

    switch (entry.value.which_module){
    case acousea_ModuleWrapper_loraReporting_tag: {
        const auto reportingModule = entry.value.module.loraReporting;
        nodeConfig.loraModule = reportingModule;
    }
    case acousea_ModuleWrapper_iridiumReporting_tag: {
        const auto reportingModule = entry.value.module.iridiumReporting;
        nodeConfig.iridiumModule = reportingModule;
    }
    default: {
        Logger::logError(
            "ReportingModule with key: " + std::to_string(entry.key) +
            " has invalid type (NOT LORA NOR IRIDIUM). Skipping.");
        return;
    }
    }
}
