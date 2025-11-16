#include "ModuleManager.hpp"

#include <cinttypes>
#include <Logger/Logger.h>

#include "SharedMemory/SharedMemory.hpp"

Result<void> ModuleManager::getModules(
    acousea_NodeDevice_ModulesEntry* outModulesArr,
    pb_size_t& outModulesArrSize,
    const acousea_ModuleCode* requestedModules,
    const pb_size_t requestedModulesSize)
{
    LOG_CLASS_FREE_MEMORY("::GetModules(start) -> requestedModulesSize=%d", requestedModulesSize);
    const acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    // Reset output size
    acousea_ModuleCode notFreshModuleCodes[_acousea_ModuleCode_MAX];
    pb_size_t notFreshModulesCount = 0;

    for (uint16_t i = 0; i < requestedModulesSize; i++)
    {
        const acousea_ModuleCode& currentModuleCode = requestedModules[i];
        auto& currentEntry = outModulesArr[outModulesArrSize];

        LOG_CLASS_INFO("Processing requested module code: %d, index %d, max index=%d, total requested=%d",
                       static_cast<int>(currentModuleCode),
                       i, requestedModulesSize - 1, requestedModulesSize
        );

        currentEntry = acousea_NodeDevice_ModulesEntry_init_default;
        currentEntry.has_value = true;
        currentEntry.key = currentModuleCode;

        switch (currentModuleCode)
        {
        case acousea_ModuleCode_AMBIENT_MODULE:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "Ambient module data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
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
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }

                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "Storage module data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_BATTERY_MODULE:
            {
                const auto batteryStatus = battery.status();
                const auto batteryPercentage = battery.voltageSOC_rounded();

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

                currentEntry.value.which_module = acousea_ModuleWrapper_gsmMqttReporting_tag;
                currentEntry.value.module.gsmMqttReporting = nodeConfig.gsmMqttModule;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_RTC_MODULE:
            {
                const auto epochSeconds = rtc.getEpoch();

                currentEntry.value.which_module = acousea_ModuleWrapper_rtc_tag;
                currentEntry.value.module.rtc = acousea_RTCModule_init_default;
                currentEntry.value.module.rtc.epochSeconds = epochSeconds;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }

        case acousea_ModuleCode_ICLISTEN_STATUS:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "ICListen status data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "ICListen logging config data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "ICListen streaming config data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_RECORDING_STATS:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "ICListen recording stats data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                if (!moduleProxy.isModuleFresh(currentModuleCode))
                {
                    notFreshModuleCodes[notFreshModulesCount++] = currentModuleCode;
                    break;
                }
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFresh(currentModuleCode);
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_FAILUREF(
                        "%s", "ICListen HF complete data should be fresh at this point, but it's not");
                }
                currentEntry.value = *optWrapper;
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


    if (notFreshModulesCount > 0)
    {

        auto* hexStringBuf = reinterpret_cast<char*>(SharedMemory::tmpBuffer());
        constexpr size_t HEX_STRING_BUF_SIZE = SharedMemory::tmpBufferSize();

        Logger::vectorToHexString(
            reinterpret_cast<const unsigned char*>(notFreshModuleCodes), notFreshModulesCount * sizeof(acousea_ModuleCode),
            hexStringBuf, HEX_STRING_BUF_SIZE
        );

        LOG_CLASS_WARNING("::GetModules() -> The following modules are not fresh yet: %s", hexStringBuf);
        moduleProxy.requestMultipleModules(
            notFreshModuleCodes,
            notFreshModulesCount,
            ModuleProxy::DeviceAlias::PIDevice
        );

        return RESULT_CLASS_VOID_INCOMPLETEF(
            "The following modules are not fresh yet: %s", hexStringBuf
        );
    }
    LOG_CLASS_FREE_MEMORY("::GetModules(end) -> outModulesArrSize=%d", outModulesArrSize);
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
