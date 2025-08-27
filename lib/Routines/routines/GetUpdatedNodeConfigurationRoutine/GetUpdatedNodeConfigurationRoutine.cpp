#include "GetUpdatedNodeConfigurationRoutine.hpp"

GetUpdatedNodeConfigurationRoutine::GetUpdatedNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    std::optional<std::shared_ptr<ICListenService>> icListenService,
    IGPS* gps,
    IBatteryController* battery,
    RTCController* rtcController
) : IRoutine<_acousea_CommunicationPacket>(getClassNameString()),
    nodeConfigurationRepository(nodeConfigurationRepository),
    icListenService(std::move(icListenService)),
    gps(gps), battery(battery), rtcController(rtcController)
{
}

Result<acousea_CommunicationPacket> GetUpdatedNodeConfigurationRoutine::execute(
    const std::optional<_acousea_CommunicationPacket>& optPacket
)
{
    if (!optPacket.has_value())
    {
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + "No packet provided to process");
    }
    const acousea_CommunicationPacket& packet = optPacket.value();
    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    if (!packet.has_payload)
    {
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + "Packet does not contain any new requested configuration"
        );
    }

    if (packet.payload.which_payload != acousea_PayloadWrapper_requestedConfiguration_tag)
    {
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() +
            "Packet payload is not of type acousea_PayloadWrapper_requestedConfiguration_tag");
    }

    acousea_GetUpdatedNodeConfigurationPayload requestedConfigurationPayload = packet.payload.payload.
        requestedConfiguration;

    acousea_CommunicationPacket responsePacket = acousea_CommunicationPacket_init_default;
    // Routing
    responsePacket.has_routing = true;
    responsePacket.routing = acousea_RoutingChunk_init_default;
    responsePacket.routing.sender = packet.routing.receiver;
    responsePacket.routing.receiver = packet.routing.sender;
    responsePacket.routing.ttl = 0;

    // Payload
    responsePacket.has_payload = true;
    responsePacket.payload.which_payload = acousea_PayloadWrapper_setConfiguration_tag;

    acousea_SetNodeConfigurationPayload setConfiguration = acousea_SetNodeConfigurationPayload_init_default;
    for (auto& configItem : requestedConfigurationPayload.requestedModules)
    {
        switch (configItem)
        {
        case acousea_ModuleCode_AMBIENT_MODULE:
            {
                Logger::logError(getClassNameString() + "FIXME: REQUESTED AMBIENT MODULE (NOT IMPLEMENTED YET)");
                break;
            }

        case acousea_ModuleCode_NETWORK_MODULE:
            {
                Logger::logError(getClassNameString() + "FIXME: REQUESTED NETWORK MODULE (NOT IMPLEMENTED YET)");
                break;
            }
        case acousea_ModuleCode_STORAGE_MODULE:
            {
                Logger::logError(getClassNameString() + "FIXME: REQUESTED STORAGE MODULE (NOT IMPLEMENTED YET)");
                break;
            }

        case acousea_ModuleCode_BATTERY_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry batteryEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
                batteryEntry.has_value = true;
                batteryEntry.key = acousea_ModuleCode_BATTERY_MODULE;
                batteryEntry.value.which_module = acousea_ModuleWrapper_battery_tag;

                acousea_BatteryModule batteryModule = acousea_BatteryModule_init_default;
                batteryModule.batteryStatus = battery->status();
                batteryModule.batteryPercentage = battery->percentage();

                batteryEntry.value.module.battery = batteryModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = batteryEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_LOCATION_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry locationEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
                locationEntry.has_value = true;
                locationEntry.key = acousea_ModuleCode_LOCATION_MODULE;
                locationEntry.value.which_module = acousea_ModuleWrapper_location_tag;

                acousea_LocationModule locationModule = acousea_LocationModule_init_default;
                auto [latitude, longitude] = gps->read();
                locationModule.latitude = latitude;
                locationModule.longitude = longitude;
                locationEntry.value.module.location = locationModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = locationEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }


        case acousea_ModuleCode_OPERATION_MODES_MODULE:
        case acousea_ModuleCode_OPERATION_MODES_GRAPH_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry opModesGraphEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
                opModesGraphEntry.has_value = true;
                opModesGraphEntry.key = acousea_ModuleCode_OPERATION_MODES_GRAPH_MODULE;
                opModesGraphEntry.value.which_module = acousea_ModuleWrapper_operationModesGraph_tag;
                if (!nodeConfig.has_operationGraphModule)
                {
                    Logger::logError(
                        getClassNameString() +
                        "Node configuration does not have operation modes graph module configured");
                    break;
                }
                acousea_OperationModesGraphModule opModesGraphModule = nodeConfig.operationGraphModule;
                opModesGraphEntry.value.module.operationModesGraph = opModesGraphModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = opModesGraphEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_LORA_REPORTING_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry loraEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
                loraEntry.has_value = true;
                loraEntry.key = acousea_ModuleCode_LORA_REPORTING_MODULE;
                loraEntry.value.which_module = acousea_ModuleWrapper_loraReporting_tag;
                if (!nodeConfig.has_loraModule)
                {
                    Logger::logError(getClassNameString() + "Node configuration does not have LoRa module configured");
                    break;
                }
                acousea_LoRaReportingModule loraModule = nodeConfig.loraModule;
                loraEntry.value.module.loraReporting = loraModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = loraEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry iridiumEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                iridiumEntry.has_value = true;
                iridiumEntry.key = acousea_ModuleCode_IRIDIUM_REPORTING_MODULE;
                iridiumEntry.value.which_module = acousea_ModuleWrapper_iridiumReporting_tag;
                if (!nodeConfig.has_iridiumModule)
                {
                    Logger::logError(
                        getClassNameString() + "Node configuration does not have Iridium module configured");
                    break;
                }
                acousea_IridiumReportingModule iridiumModule = nodeConfig.iridiumModule;
                iridiumEntry.value.module.iridiumReporting = iridiumModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = iridiumEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_RTC_MODULE:
            {
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry rtcEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;
                rtcEntry.has_value = true;
                rtcEntry.key = acousea_ModuleCode_RTC_MODULE;
                rtcEntry.value.which_module = acousea_ModuleWrapper_rtc_tag;
                acousea_RTCModule rtcModule = acousea_RTCModule_init_default;
                rtcModule.epochSeconds = rtcController->getEpoch();
                rtcEntry.value.module.rtc = rtcModule;
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = rtcEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }

        case acousea_ModuleCode_ICLISTEN_STATUS:
            {
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() + "ICListen service is not available");
                    break;
                }
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry icListenStatusEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                icListenStatusEntry.has_value = true;
                icListenStatusEntry.key = acousea_ModuleCode_ICLISTEN_STATUS;
                icListenStatusEntry.value.which_module = acousea_ModuleWrapper_icListenStatus_tag;

                Result<acousea_ICListenStatus> statusResult = icListenService.value()->getCache()->
                                                                              getICListenStatus();
                if (!statusResult.isError())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Failed to retrieve ICListen status: " + statusResult.getError()
                    );
                }

                icListenStatusEntry.value.module.icListenStatus = statusResult.getValue();
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = icListenStatusEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
            {
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() + "ICListen service is not available");
                    break;
                }
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry icListenLoggingEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                icListenLoggingEntry.has_value = true;
                icListenLoggingEntry.key = acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG;
                icListenLoggingEntry.value.which_module = acousea_ModuleWrapper_icListenLoggingConfig_tag;

                Result<acousea_ICListenLoggingConfig> loggingConfigResult =
                    icListenService.value()->getCache()->getICListenLoggingConfig();
                if (loggingConfigResult.isError())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Failed to retrieve ICListen logging config: " + loggingConfigResult.
                        getError());
                }
                icListenLoggingEntry.value.module.icListenLoggingConfig = loggingConfigResult.getValue();
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = icListenLoggingEntry;
                setConfiguration.modulesToChange_count++;

                break;
            }
        case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
            {
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() + "ICListen service is not available");
                    break;
                }
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry icListenStreamingEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                icListenStreamingEntry.has_value = true;
                icListenStreamingEntry.key = acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG;
                icListenStreamingEntry.value.which_module = acousea_ModuleWrapper_icListenStreamingConfig_tag;

                Result<acousea_ICListenStreamingConfig> streamingConfigResult =
                    icListenService.value()->getCache()->getICListenStreamingConfig();
                if (streamingConfigResult.isError())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Failed to retrieve ICListen streaming config: " + streamingConfigResult.
                        getError());
                }
                icListenStreamingEntry.value.module.icListenStreamingConfig = streamingConfigResult.getValue();
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = icListenStreamingEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_RECORDING_STATS:
            {
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() + "ICListen service is not available");
                    break;
                }
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry icListenRecordingStatsEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                icListenRecordingStatsEntry.has_value = true;
                icListenRecordingStatsEntry.key = acousea_ModuleCode_ICLISTEN_RECORDING_STATS;
                icListenRecordingStatsEntry.value.which_module = acousea_ModuleWrapper_icListenRecordingStats_tag;

                Result<acousea_ICListenRecordingStats> recordingStatsResult =
                    icListenService.value()->getCache()->getICListenRecordingStats();
                if (recordingStatsResult.isError())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Failed to retrieve ICListen recording stats: " + recordingStatsResult.
                        getError());
                }
                icListenRecordingStatsEntry.value.module.icListenRecordingStats = recordingStatsResult.getValue();
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = icListenRecordingStatsEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                if (!icListenService.has_value())
                {
                    Logger::logError(getClassNameString() + "ICListen service is not available");
                    break;
                }
                acousea_SetNodeConfigurationPayload_ModulesToChangeEntry icListenHfEntry =
                    acousea_SetNodeConfigurationPayload_ModulesToChangeEntry_init_default;

                icListenHfEntry.has_value = true;
                icListenHfEntry.key = acousea_ModuleCode_ICLISTEN_HF;
                icListenHfEntry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;

                Result<acousea_ICListenHF> hfResult = icListenService.value()->getCache()->
                                                                      getICListenCompleteConfiguration();
                if (hfResult.isError())
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + "Failed to retrieve ICListen HF: " + hfResult.getError());
                }
                icListenHfEntry.value.module.icListenHF = hfResult.getValue();
                setConfiguration.modulesToChange[setConfiguration.modulesToChange_count] = icListenHfEntry;
                setConfiguration.modulesToChange_count++;
                break;
            }
        default:
            Logger::logError(getClassNameString() + "Unknown module code requested: " + std::to_string(configItem));
            break;
        }
    }
    // IMPORTANT: Assign the setConfiguration payload to the response packet
    responsePacket.payload.payload.setConfiguration = setConfiguration;

    return Result<acousea_CommunicationPacket>::success(responsePacket);
}
