#include "ModuleManager.hpp"

#include <cinttypes>
#include <Logger/Logger.h>

Result<void> ModuleManager::getModules(
    acousea_NodeDevice_ModulesEntry* outModulesArr,
    pb_size_t& outModulesArrSize,
    const acousea_ModuleCode* requestedModules,
    const pb_size_t requestedModulesSize)
{
    LOG_CLASS_FREE_MEMORY("::GetModules(start)");
    const acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    for (uint16_t i = 0; i < requestedModulesSize; i++)
    {
        const acousea_ModuleCode& currentModuleCode = requestedModules[i];
        auto& currentEntry = outModulesArr[outModulesArrSize];

        LOG_CLASS_INFO("Processing requested module code: %d, index %d of %d",
                       static_cast<int>(currentModuleCode),
                       i,
                       requestedModulesSize - 1
        );

        currentEntry.has_value = true;
        currentEntry.key = currentModuleCode;

        switch (currentModuleCode)
        {
        case acousea_ModuleCode_AMBIENT_MODULE:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_AMBIENT_MODULE,
                    acousea_ModuleWrapper_ambient_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );

                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "Ambient module data is not fresh yet (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_NETWORK_MODULE:
            {
                LOG_CLASS_ERROR("FIXME: REQUESTED NETWORK MODULE (NOT IMPLEMENTED YET)");

                break;
            }
        case acousea_ModuleCode_STORAGE_MODULE:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_STORAGE_MODULE,
                    acousea_ModuleWrapper_storage_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );

                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "Storage module data is not fresh yet (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_BATTERY_MODULE:
            {
                auto batteryStatus = battery.status();
                auto batteryPercentage = battery.voltageSOC_rounded();

                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_battery_tag;
                currentEntry.value.module.battery = acousea_BatteryModule_init_default;
                currentEntry.value.module.battery.batteryStatus = batteryStatus;
                currentEntry.value.module.battery.batteryPercentage = batteryPercentage;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_LOCATION_MODULE:
            {
                auto [latitude, longitude] = gps.read();

                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_location_tag;
                currentEntry.value.module.location = acousea_LocationModule_init_default;

                currentEntry.value.module.location.latitude = latitude;
                currentEntry.value.module.location.longitude = longitude;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_REPORTING_TYPES_MODULE:
            {
                if (!nodeConfig.has_reportTypesModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have report types module configured");
                    break;
                }
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_reportTypes_tag;
                currentEntry.value.module.reportTypes = nodeConfig.reportTypesModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_OPERATION_MODES_MODULE:
            {
                if (!nodeConfig.has_operationModesModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have operation modes module configured");
                    break;
                }
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_operationModes_tag;
                currentEntry.value.module.operationModes = nodeConfig.operationModesModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_LORA_REPORTING_MODULE:
            {
                if (!nodeConfig.has_loraModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have LoRa module configured");
                    break;
                }
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_loraReporting_tag;
                currentEntry.value.module.loraReporting = nodeConfig.loraModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
            {
                if (!nodeConfig.has_iridiumModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have Iridium module configured");
                    break;
                }
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_iridiumReporting_tag;
                currentEntry.value.module.iridiumReporting = nodeConfig.iridiumModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE:
            {
                if (!nodeConfig.has_gsmMqttModule)
                {
                    LOG_CLASS_ERROR("Node configuration does not have GSM-MQTT module configured");
                    break;
                }
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_gsmMqttReporting_tag;
                currentEntry.value.module.gsmMqttReporting = nodeConfig.gsmMqttModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_RTC_MODULE:
            {
                const auto epochSeconds = rtc.getEpoch();
                currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
                currentEntry.value.which_module = acousea_ModuleWrapper_rtc_tag;
                currentEntry.value.module.rtc = acousea_RTCModule_init_default;
                currentEntry.value.module.rtc.epochSeconds = epochSeconds;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_ICLISTEN_STATUS:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_ICLISTEN_STATUS,
                    acousea_ModuleWrapper_icListenStatus_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );

                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF("%s", "ICListen status is not fresh (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG,
                    acousea_ModuleWrapper_icListenLoggingConfig_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );

                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen logging config is not fresh (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG,
                    acousea_ModuleWrapper_icListenStreamingConfig_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );
                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen streaming config is not fresh (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_RECORDING_STATS:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_ICLISTEN_RECORDING_STATS,
                    acousea_ModuleWrapper_icListenRecordingStats_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );
                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen recording stats is not fresh (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                const bool fetchOk = _fetchModuleEntry(
                    currentEntry,
                    acousea_ModuleCode_ICLISTEN_HF,
                    acousea_ModuleWrapper_icListenHF_tag,
                    ModuleProxy::DeviceAlias::PIDevice
                );
                if (!fetchOk)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF("%s", "ICListen HF is not fresh (requested from device)");
                }

                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        default:
            LOG_CLASS_ERROR("Unknown or unsupported module code requested: %d", currentModuleCode);
            currentEntry.has_value = false;
            currentEntry.key = acousea_ModuleCode_MODULE_UNKNOWN;
            break;
        }
    }
    LOG_CLASS_FREE_MEMORY("::GetModules(end)");
    return RESULT_VOID_SUCCESS();
}


Result<void> ModuleManager::setModules(const pb_size_t modules_count,
                                       const acousea_SetNodeConfigurationPayload_ModulesEntry* modules)
{
    LOG_CLASS_FREE_MEMORY("::SetModules(start)");
    acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    for (size_t i = 0; i < modules_count; ++i)
    {
        const auto& module = modules[i];
        if (!module.has_value)
        {
            return RESULT_CLASS_VOID_FAILUREF("Module with key: %" PRId32 " has no value. Skipping.", module.key);
        }

        switch (module.key)
        {
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_MODULE:
            {
                if (const auto result = _setOperationModes(nodeConfig, module); result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }
                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_REPORTING_TYPES_MODULE:
            {
                if (const auto result = _setReportTypesModule(nodeConfig, module); result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }

                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE:
            {
                if (const auto result = _setReportingPeriods(nodeConfig, module); result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }

                break;
            }

        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF:
            {
                const auto setICListenWrapperPtr = moduleProxy.getIfFreshOrSetOnDevice(
                    static_cast<acousea_ModuleCode>(module.key),
                    module.value,
                    ModuleProxy::DeviceAlias::PIDevice
                );
                if (setICListenWrapperPtr == nullptr)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF("%s", "ICListen module configuration not set on device yet.");
                }
                LOG_CLASS_INFO("ICListen configuration module with key: %" PRId32 " set on device successfully.",
                               module.key);
                break;
            }

        default:
            {
                return RESULT_CLASS_VOID_FAILUREF("Invalid TagType in SetNodeConfigurationPayload. Key=%" PRId32,
                                                  module.key);
            }
        }
    }

    // Important:  Store the updated configuration only after processing all modules successfully
    nodeConfigurationRepository.saveConfiguration(nodeConfig);
    return RESULT_VOID_SUCCESS();
}

bool ModuleManager::_fetchModuleEntry(acousea_NodeDevice_ModulesEntry& outModuleEntry,
                                      acousea_ModuleCode code,
                                      const uint16_t whichTag,
                                      const ModuleProxy::DeviceAlias alias) const
{
    const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(code, alias);
    if (!optWrapper)
    {
        return false;
    }

    outModuleEntry.has_value = true;
    outModuleEntry.key = code;
    outModuleEntry.value = *optWrapper;
    outModuleEntry.value.which_module = whichTag;

    return true;
}


Result<void> ModuleManager::_setOperationModes(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry)
{
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_operationModes_tag)
    {
        return RESULT_CLASS_VOID_FAILUREF("OperationModesModule with key: %" PRId32 " has no value. Skipping.",
                                          moduleEntry.key);
    }
    const auto& operationModesModule = moduleEntry.value.module.operationModes;
    nodeConfig.operationModesModule = operationModesModule;
    return RESULT_VOID_SUCCESS();
}

Result<void> ModuleManager::_setReportTypesModule(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry)
{
    if (!moduleEntry.has_value || moduleEntry.value.which_module != acousea_ModuleWrapper_reportTypes_tag)
    {
        return RESULT_CLASS_VOID_FAILUREF("ReportTypesModule with key: %" PRId32 " has no value. Skipping.",
                                          moduleEntry.key);
    }
    const auto& reportTypesModule = moduleEntry.value.module.reportTypes;
    nodeConfig.reportTypesModule = reportTypesModule;
    return RESULT_VOID_SUCCESS();
}

Result<void> ModuleManager::_setReportingPeriods(
    acousea_NodeConfiguration& nodeConfig, const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry)
{
    if (!moduleEntry.has_value)
    {
        return RESULT_CLASS_VOID_FAILUREF("ReportingModule with key: %" PRId32 " has no value. Skipping.",
                                          moduleEntry.key);
    }

    switch (moduleEntry.value.which_module)
    {
    case acousea_ModuleWrapper_loraReporting_tag:
        {
            const auto& reportingModule = moduleEntry.value.module.loraReporting;
            nodeConfig.loraModule = reportingModule;
            break;
        }
    case acousea_ModuleWrapper_iridiumReporting_tag:
        {
            const auto& reportingModule = moduleEntry.value.module.iridiumReporting;
            nodeConfig.iridiumModule = reportingModule;
            break;
        }
    case acousea_ModuleWrapper_gsmMqttReporting_tag:
        {
            const auto& reportingModule = moduleEntry.value.module.gsmMqttReporting;
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
    LOG_CLASS_FREE_MEMORY("::SetModules(end)");
    return RESULT_VOID_SUCCESS();
}
