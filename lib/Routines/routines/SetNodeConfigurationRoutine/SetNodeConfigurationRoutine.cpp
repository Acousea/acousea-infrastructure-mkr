#include "SetNodeConfigurationRoutine.h"

#include <cinttypes>

#include "Logger/Logger.h"

SetNodeConfigurationRoutine::SetNodeConfigurationRoutine(
    NodeConfigurationRepository &nodeConfigurationRepository,
    ModuleProxy &moduleProxy)
    : IRoutine(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy)
{
}

Result<acousea_CommunicationPacket> SetNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket> &inPacket)
{
    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    if (!inPacket.has_value())
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, ": No packet provided");
    }

    if (inPacket.value().which_body != acousea_CommunicationPacket_command_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, " Packet is not of type command");
    }
    if (inPacket.value().body.command.which_command != acousea_CommandBody_setConfiguration_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, " Packet command is not of type setNodeConfiguration");
    }

    const auto [modules_count, modules] = inPacket.value().body.command.command.setConfiguration;

    for (size_t i = 0; i < modules_count; ++i)
    {
        const auto &module = modules[i];
        if (!module.has_value)
        {
            return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                   "Module with key: %" PRId32 " has no value. Skipping.", module.key);
        }

        switch (module.key)
        {
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_MODULE:
        {
            if (const auto result = setOperationModes(nodeConfig, module); result.isError())
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "%s", result.getError());
            }
            break;
        }
        case acousea_ModuleCode::acousea_ModuleCode_REPORTING_TYPES_MODULE:
        {
            if (const auto result = setReportTypesModule(nodeConfig, module); result.isError())
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "%s", result.getError());
            }

            break;
        }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE:
        {
            if (const auto result = setReportingPeriods(nodeConfig, module); result.isError())
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "%s", result.getError());
            }

            break;
        }

        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF:
        {
            const auto setIcListenConfigResult = setICListenConfiguration(module);
            if (setIcListenConfigResult.isError())
            {
                return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "%s", setIcListenConfigResult.getError());
            }
            if (setIcListenConfigResult.isPending())
            {
                return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "%s", setIcListenConfigResult.getError());
            }
            LOG_CLASS_ERROR("New ICListen configuration sent to device.");
            break;
        }

        default:
            return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                   "Invalid TagType in SetNodeConfigurationPayload. Key=%" PRId32, module.key);
        }
    }

    // Important:  Store the updated configuration only after processing all modules successfully
    nodeConfigurationRepository.saveConfiguration(nodeConfig);

    acousea_CommunicationPacket outPacket = inPacket.value();
    outPacket.which_body = acousea_CommunicationPacket_response_tag;
    outPacket.body.response.which_response = acousea_ResponseBody_setConfiguration_tag;
    outPacket.body.response.response.setConfiguration = inPacket.value().body.command.command.setConfiguration;

    return RESULT_SUCCESS(acousea_CommunicationPacket, outPacket);
}

Result<void> SetNodeConfigurationRoutine::setOperationModes(
    acousea_NodeConfiguration &nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry &moduleEntry)
{
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_operationModes_tag)
    {
        return RESULT_CLASS_VOID_FAILUREF("OperationModesModule with key: %" PRId32 " has no value. Skipping.",
                                    moduleEntry.key);
    }
    const auto operationModesModule = moduleEntry.value.module.operationModes;
    nodeConfig.operationModesModule = operationModesModule;
    return RESULT_VOID_SUCCESS();
}

Result<void> SetNodeConfigurationRoutine::setReportTypesModule(
    acousea_NodeConfiguration &nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry &moduleEntry)
{
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_reportTypes_tag)
    {
        return RESULT_CLASS_VOID_FAILUREF("ReportTypesModule with key: %" PRId32 " has no value. Skipping.", moduleEntry.key);
    }
    const auto reportTypesModule = moduleEntry.value.module.reportTypes;
    nodeConfig.reportTypesModule = reportTypesModule;
    return RESULT_VOID_SUCCESS();
}

Result<void> SetNodeConfigurationRoutine::setReportingPeriods(
    acousea_NodeConfiguration &nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry &moduleEntry)
{
    if (!moduleEntry.has_value)
    {
        return RESULT_CLASS_VOID_FAILUREF("ReportingModule with key: %" PRId32 " has no value. Skipping.", moduleEntry.key);
    }

    switch (moduleEntry.value.which_module)
    {
    case acousea_ModuleWrapper_loraReporting_tag:
    {
        const auto reportingModule = moduleEntry.value.module.loraReporting;
        nodeConfig.loraModule = reportingModule;
        break;
    }
    case acousea_ModuleWrapper_iridiumReporting_tag:
    {
        const auto reportingModule = moduleEntry.value.module.iridiumReporting;
        nodeConfig.iridiumModule = reportingModule;
        break;
    }
    case acousea_ModuleWrapper_gsmMqttReporting_tag:
    {
        const auto reportingModule = moduleEntry.value.module.gsmMqttReporting;
        nodeConfig.gsmMqttModule = reportingModule;
        break;
    }
    default:
    {
        return RESULT_CLASS_VOID_FAILUREF("ReportingModule with key: %" PRId32
                                    " has invalid type (NOT LORA NOR IRIDIUM). Skipping.",
                                    moduleEntry.key);
    }
    }
    return RESULT_VOID_SUCCESS();
}

Result<void> SetNodeConfigurationRoutine::setICListenConfiguration(
    const acousea_SetNodeConfigurationPayload_ModulesEntry &entry) const
{
    if (!entry.has_value)
    {
        return RESULT_CLASS_VOID_FAILUREF("ICListen module entry has no value.");
    }

    // Reenviar directamente el módulo recibido
    moduleProxy.sendModule(static_cast<acousea_ModuleCode>(entry.key), entry.value,
                           ModuleProxy::DeviceAlias::PIDevice);

    // Comprobar frescura de los módulos relacionados
    const auto loggingFresh = moduleProxy.getCache().getIfFresh(acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG);
    const auto streamingFresh = moduleProxy.getCache().getIfFresh(acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG);

    if (!loggingFresh || !streamingFresh)
    {
        return RESULT_CLASS_VOID_PENDINGF("ICListen configuration not fresh");
    }

    LOG_CLASS_INFO(": ICListen configuration updated and fresh.");
    return RESULT_VOID_SUCCESS();
}
