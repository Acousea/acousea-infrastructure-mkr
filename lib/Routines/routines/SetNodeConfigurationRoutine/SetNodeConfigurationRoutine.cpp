#include "SetNodeConfigurationRoutine.h"

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(getClassNameString()), nodeConfigurationRepository(nodeConfigurationRepository) {}

Result<Packet> SetNodeConfigurationRoutine::execute(const Packet &packet) {
    NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    NewNodeConfigurationPayload configurationPayload = packet.getPayloadAs<NewNodeConfigurationPayload>();

    for (const auto &item: configurationPayload.getModules()) {
        switch (ModuleCode::enumFromValue(item.getType())) {
            case ModuleCode::TYPES::OPERATION_MODES_GRAPH: {
                setOperationModes(nodeConfig, item);
                break;
            }
            case ModuleCode::TYPES::REPORTING: {
                setReportingPeriods(nodeConfig, item);
                break;
            }
            default:
                return Result<Packet>::failure("Invalid TagType in NewNodeConfigurationPayload");
        }

    }
    nodeConfigurationRepository.saveConfiguration(nodeConfig);
    return Result<Packet>::success(packet);
}

void SetNodeConfigurationRoutine::setOperationModes(NodeConfiguration &nodeConfig, const SerializableModule &item) {
    auto &operationModesModule = (OperationModesGraphModule &) item;
    nodeConfig.setOperationGraphModule(operationModesModule);
}

void SetNodeConfigurationRoutine::setReportingPeriods(NodeConfiguration &nodeConfig, const SerializableModule &item) {
    auto &reportingModule = (ReportingModule &) item;
    switch (reportingModule.getTechnologyType()) {
        case ReportingModule::TechnologyType::LORA: {
            nodeConfig.setLoraModule((LoRaReportingModule &) reportingModule);
            break;
        }
        case ReportingModule::TechnologyType::IRIDIUM: {
            nodeConfig.setIridiumModule((IridiumReportingModule &) reportingModule);
            break;
        }
    }
}
