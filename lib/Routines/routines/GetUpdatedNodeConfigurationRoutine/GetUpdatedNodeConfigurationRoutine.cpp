#include "GetUpdatedNodeConfigurationRoutine.hpp"
#include "Logger/Logger.h"

GetUpdatedNodeConfigurationRoutine::GetUpdatedNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    ModuleProxy& moduleProxy,
    IGPS& gps,
    IBatteryController& battery,
    RTCController& rtcController)
    : IRoutine<acousea_CommunicationPacket>(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy),
      gps(gps), battery(battery), rtcController(rtcController)
{
}

Result<acousea_CommunicationPacket> GetUpdatedNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& optPacket)
{
    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    if (!optPacket.has_value())
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "No packet provided to process");
    }
    const acousea_CommunicationPacket& packet = optPacket.value();

    if (packet.which_body != acousea_CommunicationPacket_command_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "Packet is not of type command");
    }

    if (packet.body.command.which_command != acousea_CommandBody_requestedConfiguration_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                     "Packet command is not of type requestedConfiguration");
    }

    acousea_GetUpdatedNodeConfigurationPayload requestedConfigurationPayload = packet.body.command.command.
        requestedConfiguration;

    acousea_CommunicationPacket responsePacket = acousea_CommunicationPacket_init_default;
    // Routing
    responsePacket.has_routing = true;
    responsePacket.routing = acousea_RoutingChunk_init_default;
    responsePacket.routing.sender = packet.routing.receiver;
    responsePacket.routing.receiver = packet.routing.sender;
    responsePacket.routing.ttl = 0;

    // Payload
    responsePacket.which_body = acousea_CommunicationPacket_response_tag;

    acousea_UpdatedNodeConfigurationPayload updatedConfiguration = acousea_UpdatedNodeConfigurationPayload_init_default;

    for (pb_size_t i = 0; i < requestedConfigurationPayload.requestedModules_count; ++i)
    {
        switch (const acousea_ModuleCode configItem = requestedConfigurationPayload.requestedModules[i])
        {
        case acousea_ModuleCode_AMBIENT_MODULE:
            {
                LOG_CLASS_ERROR("FIXME: REQUESTED AMBIENT MODULE (NOT IMPLEMENTED YET)");
                break;
            }

        case acousea_ModuleCode_NETWORK_MODULE:
            {
                LOG_CLASS_ERROR("FIXME: REQUESTED NETWORK MODULE (NOT IMPLEMENTED YET)");
                break;
            }
        case acousea_ModuleCode_STORAGE_MODULE:
            {
                LOG_CLASS_ERROR("FIXME: REQUESTED STORAGE MODULE (NOT IMPLEMENTED YET)");
                break;
            }

        case acousea_ModuleCode_BATTERY_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry batteryEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                batteryEntry.has_value = true;
                batteryEntry.key = acousea_ModuleCode_BATTERY_MODULE;
                batteryEntry.value.which_module = acousea_ModuleWrapper_battery_tag;

                acousea_BatteryModule batteryModule = acousea_BatteryModule_init_default;
                batteryModule.batteryStatus = battery.status();
                batteryModule.batteryPercentage = battery.voltageSOC_rounded();

                batteryEntry.value.module.battery = batteryModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = batteryEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_LOCATION_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry locationEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                locationEntry.has_value = true;
                locationEntry.key = acousea_ModuleCode_LOCATION_MODULE;
                locationEntry.value.which_module = acousea_ModuleWrapper_location_tag;

                acousea_LocationModule locationModule = acousea_LocationModule_init_default;
                auto [latitude, longitude] = gps.read();
                locationModule.latitude = latitude;
                locationModule.longitude = longitude;
                locationEntry.value.module.location = locationModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = locationEntry;
                updatedConfiguration.modules_count++;
                break;
            }

        case acousea_ModuleCode_REPORTING_TYPES_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry reportingTypesEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                reportingTypesEntry.has_value = true;
                reportingTypesEntry.key = acousea_ModuleCode_REPORTING_TYPES_MODULE;
                reportingTypesEntry.value.which_module = acousea_ModuleWrapper_reportTypes_tag;
                if (!nodeConfig.has_reportTypesModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have report types module configured");
                    break;
                }
                acousea_ReportTypesModule reportTypesModule = nodeConfig.reportTypesModule;
                reportingTypesEntry.value.module.reportTypes = reportTypesModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = reportingTypesEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_OPERATION_MODES_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry opModesGraphEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                opModesGraphEntry.has_value = true;
                opModesGraphEntry.key = acousea_ModuleCode_OPERATION_MODES_MODULE;
                opModesGraphEntry.value.which_module = acousea_ModuleWrapper_operationModes_tag;
                if (!nodeConfig.has_operationModesModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have operation modes module configured");
                    break;
                }
                acousea_OperationModesModule operationModesModule = nodeConfig.operationModesModule;
                opModesGraphEntry.value.module.operationModes = operationModesModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = opModesGraphEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_LORA_REPORTING_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry loraEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                loraEntry.has_value = true;
                loraEntry.key = acousea_ModuleCode_LORA_REPORTING_MODULE;
                loraEntry.value.which_module = acousea_ModuleWrapper_loraReporting_tag;
                if (!nodeConfig.has_loraModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have LoRa module configured");
                    break;
                }
                acousea_LoRaReportingModule loraModule = nodeConfig.loraModule;
                loraEntry.value.module.loraReporting = loraModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = loraEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry iridiumEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                iridiumEntry.has_value = true;
                iridiumEntry.key = acousea_ModuleCode_IRIDIUM_REPORTING_MODULE;
                iridiumEntry.value.which_module = acousea_ModuleWrapper_iridiumReporting_tag;
                if (!nodeConfig.has_iridiumModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have Iridium module configured");
                    break;
                }
                acousea_IridiumReportingModule iridiumModule = nodeConfig.iridiumModule;
                iridiumEntry.value.module.iridiumReporting = iridiumModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = iridiumEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry gsmMqttEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                gsmMqttEntry.has_value = true;
                gsmMqttEntry.key = acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE;
                gsmMqttEntry.value.which_module = acousea_ModuleWrapper_gsmMqttReporting_tag;
                if (!nodeConfig.has_gsmMqttModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have GSM-MQTT module configured");
                    break;
                }
                acousea_GsmMqttReportingModule mqttModule = nodeConfig.gsmMqttModule;
                gsmMqttEntry.value.module.gsmMqttReporting = mqttModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = gsmMqttEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_RTC_MODULE:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry rtcEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;
                rtcEntry.has_value = true;
                rtcEntry.key = acousea_ModuleCode_RTC_MODULE;
                rtcEntry.value.which_module = acousea_ModuleWrapper_rtc_tag;
                acousea_RTCModule rtcModule = acousea_RTCModule_init_default;
                rtcModule.epochSeconds = rtcController.getEpoch();
                rtcEntry.value.module.rtc = rtcModule;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = rtcEntry;
                updatedConfiguration.modules_count++;
                break;
            }

        case acousea_ModuleCode_ICLISTEN_STATUS:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry icListenStatusEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                icListenStatusEntry.has_value = true;
                icListenStatusEntry.key = acousea_ModuleCode_ICLISTEN_STATUS;
                icListenStatusEntry.value.which_module = acousea_ModuleWrapper_icListenStatus_tag;

                const auto optICListenStatus_ModuleWrapper = moduleProxy.getCache()
                                                                        .getIfFresh(
                                                                            acousea_ModuleCode_ICLISTEN_STATUS);
                if (!optICListenStatus_ModuleWrapper)
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "ICListen status is not fresh");
                icListenStatusEntry.value = *optICListenStatus_ModuleWrapper;

                updatedConfiguration.modules[updatedConfiguration.modules_count] = icListenStatusEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry icListenLoggingEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                icListenLoggingEntry.has_value = true;
                icListenLoggingEntry.key = acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG;
                icListenLoggingEntry.value.which_module = acousea_ModuleWrapper_icListenLoggingConfig_tag;

                const auto optICListenLogging_ModuleWrapper = moduleProxy.getCache()
                                                                         .getIfFresh(
                                                                             acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG);
                if (!optICListenLogging_ModuleWrapper)
                {
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "ICListen logging config is not fresh");
                }
                icListenLoggingEntry.value = *optICListenLogging_ModuleWrapper;

                updatedConfiguration.modules[updatedConfiguration.modules_count] = icListenLoggingEntry;
                updatedConfiguration.modules_count++;

                break;
            }
        case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry icListenStreamingEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                icListenStreamingEntry.has_value = true;
                icListenStreamingEntry.key = acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG;
                icListenStreamingEntry.value.which_module = acousea_ModuleWrapper_icListenStreamingConfig_tag;

                const auto optICListenStreaming_ModuleWrapper = moduleProxy.getCache()
                                                                           .getIfFresh(
                                                                               acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG);
                if (!optICListenStreaming_ModuleWrapper)
                {
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "ICListen streaming config is not fresh");
                }
                icListenStreamingEntry.value = *optICListenStreaming_ModuleWrapper;

                updatedConfiguration.modules[updatedConfiguration.modules_count] = icListenStreamingEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_RECORDING_STATS:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry icListenRecordingStatsEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                icListenRecordingStatsEntry.has_value = true;
                icListenRecordingStatsEntry.key = acousea_ModuleCode_ICLISTEN_RECORDING_STATS;
                icListenRecordingStatsEntry.value.which_module = acousea_ModuleWrapper_icListenRecordingStats_tag;

                const auto optICListenRecordingStats_ModuleWrapper = moduleProxy.getCache()
                    .getIfFresh(acousea_ModuleCode_ICLISTEN_RECORDING_STATS);
                if (!optICListenRecordingStats_ModuleWrapper)
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "ICListen recording stats is not fresh");

                icListenRecordingStatsEntry.value = *optICListenRecordingStats_ModuleWrapper;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = icListenRecordingStatsEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                acousea_UpdatedNodeConfigurationPayload_ModulesEntry icListenHfEntry =
                    acousea_UpdatedNodeConfigurationPayload_ModulesEntry_init_default;

                icListenHfEntry.has_value = true;
                icListenHfEntry.key = acousea_ModuleCode_ICLISTEN_HF;
                icListenHfEntry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;

                const auto optICListenHf_ModuleWrapper = moduleProxy.getCache()
                                                                    .getIfFresh(acousea_ModuleCode_ICLISTEN_HF);
                if (!optICListenHf_ModuleWrapper)
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket, "ICListen HF is not fresh");

                icListenHfEntry.value = *optICListenHf_ModuleWrapper;
                updatedConfiguration.modules[updatedConfiguration.modules_count] = icListenHfEntry;
                updatedConfiguration.modules_count++;
                break;
            }
        default:
            LOG_CLASS_ERROR("Unknown module code requested: %d", configItem);
            break;
        }
    }
    // IMPORTANT: Assign the setConfiguration payload to the response packet
    responsePacket.body.response.which_response = acousea_ResponseBody_updatedConfiguration_tag;
    responsePacket.body.response.response.updatedConfiguration = updatedConfiguration;

    return RESULT_SUCCESS(acousea_CommunicationPacket, responsePacket);
}
