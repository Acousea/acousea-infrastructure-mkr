#include "ModuleManager.hpp"

#include <cinttypes>
#include <cstdio>
#include <Logger/Logger.h>

#include "pb_encode.h"
#include "SharedMemory/SharedMemory.hpp"

namespace
{
    void buildModuleCodesListString(const acousea_ModuleCode* codes, const pb_size_t count,
                                    char* outBuf, const size_t outBufSize)
    {
        size_t pos = 0;
        if (outBufSize == 0) return;
        outBuf[0] = '\0';

        for (pb_size_t i = 0; i < count && pos < outBufSize - 1; ++i)
        {
            const unsigned int code = static_cast<unsigned int>(codes[i]);

            // Add separator except first
            if (i > 0 && pos + 2 < outBufSize - 1)
            {
                outBuf[pos++] = ',';
                outBuf[pos++] = ' ';
            }

            const int written = snprintf(outBuf + pos, outBufSize - pos, "%u", code);
            if (written <= 0) break;

            pos += static_cast<size_t>(written);
        }

        // Null terminate exactly where content ends
        if (pos < outBufSize) outBuf[pos] = '\0';
        else outBuf[outBufSize - 1] = '\0';
    }
}

void ModuleManager::invalidateModules(
    const acousea_ModuleCode* requestedModules,
    const pb_size_t requestedModulesSize)
{
    moduleProxy.invalidateMultiple(requestedModules, requestedModulesSize);
}

Result<void> ModuleManager::requestUpdatedModules(const acousea_ModuleCode* requestedModules,
                                                  const pb_size_t requestedModulesSize)
{
    const bool requestOk = moduleProxy.requestMultipleModules(
        requestedModules,
        requestedModulesSize,
        ModuleProxy::DeviceAlias::PIDevice
    );

    auto* stringNotFreshModulesBuffer = reinterpret_cast<char*>(SharedMemory::tmpBuffer());
    constexpr size_t BUF_SIZE = SharedMemory::tmpBufferSize();
    // Must call buildModuleCodesListString internally since the tmpBuffer is used by requestMultipleModules
    if (!requestOk)
    {
        buildModuleCodesListString(requestedModules, requestedModulesSize, stringNotFreshModulesBuffer, BUF_SIZE);
        return RESULT_CLASS_VOID_FAILUREF(
            "Failed to request the following modules from device: %s", stringNotFreshModulesBuffer);
    }

    buildModuleCodesListString(requestedModules, requestedModulesSize, stringNotFreshModulesBuffer, BUF_SIZE);
    LOG_CLASS_WARNING(
        "::requestUpdatedModules() -> The following modules were requested and are NOT fresh from now: %s",
        stringNotFreshModulesBuffer);

    return RESULT_VOID_SUCCESS();
}

Result<void> ModuleManager::getModules(
    acousea_NodeDevice_ModulesEntry* outModulesArr,
    pb_size_t& outModulesArrSize,
    const acousea_ModuleCode* requestedModules,
    const pb_size_t requestedModulesSize)
{
    LOG_CLASS_FREE_MEMORY("::GetModules(start) -> requestedModulesSize=%d", requestedModulesSize);
    const acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

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
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "Ambient module data is not fresh yet");
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
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "Storage module data is not fresh yet");
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
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen status data is not fresh yet");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
            {
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen logging config data is not fresh yet");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
            {
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen streaming config data is not fresh yet");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_RECORDING_STATS:
            {
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen recording stats data is not fresh yet");
                }
                currentEntry.value = *optWrapper;
                outModulesArr[outModulesArrSize++] = currentEntry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                const acousea_ModuleWrapper* optWrapper = moduleProxy.getIfFreshOrRequestFromDevice(
                    currentModuleCode, ModuleProxy::DeviceAlias::PIDevice
                );
                if (!optWrapper)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF(
                        "%s", "ICListen HF data is not fresh yet");
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

        const auto& moduleCode = static_cast<acousea_ModuleCode>(module.key);
        const auto& moduleWrapper = module.value;

        switch (moduleCode)
        {
        case acousea_ModuleCode::acousea_ModuleCode_OPERATION_MODES_MODULE:
            {
                if (const auto result = _setOperationModes(nodeConfig, module);
                    result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }
                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_REPORTING_TYPES_MODULE:
            {
                if (const auto result = _setReportTypesModule(nodeConfig, module);
                    result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }

                break;
            }
        case acousea_ModuleCode::acousea_ModuleCode_IRIDIUM_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_LORA_REPORTING_MODULE:
        case acousea_ModuleCode::acousea_ModuleCode_GSM_MQTT_REPORTING_MODULE:
            {
                if (const auto result = _setReportingPeriods(nodeConfig, module);
                    result.isError())
                {
                    return RESULT_CLASS_VOID_FAILUREF("%s", result.getError());
                }

                break;
            }

        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG:
        case acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF:
            {
                // First we try to load the module (if not loaded it requests it from device)
                const auto setICListenWrapperPtr = moduleProxy.getIfFreshOrRequestFromDevice(
                    moduleCode, ModuleProxy::DeviceAlias::PIDevice
                );

                if (setICListenWrapperPtr == nullptr)
                {
                    return RESULT_CLASS_VOID_INCOMPLETEF("%s", "ICListen module configuration not set on device yet.");
                }
                LOG_CLASS_INFO("ICListen configuration module with key: %" PRId32 " loaded from device.",
                               module.key);

                // Then we compare if the module being set is different from the one already set
                if (_areModulesEqual(*setICListenWrapperPtr, moduleWrapper))
                {
                    LOG_CLASS_INFO("ICListen configuration module with key: %" PRId32
                                   " is set correctly on device.",
                                   module.key);
                    break;
                }
                LOG_CLASS_INFO("ICListen configuration module with key: %" PRId32
                               " differs from the one set on device. Updating...",
                               module.key);

                // If they're different we set it on the device (send internally invalidates the module until confirmed)
                if (const bool sendOk = moduleProxy.sendModule(moduleWrapper, ModuleProxy::DeviceAlias::PIDevice);
                    !sendOk)
                {
                    return RESULT_CLASS_VOID_FAILUREF("Failed to send ICListen module with key: %" PRId32
                                                      " to device.", module.key);
                }

                LOG_CLASS_INFO(
                    "ICListen configuration module with key: %" PRId32
                    " sent successfully. Waiting for confirmation...",
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


bool ModuleManager::_areModulesEqual(
    const acousea_ModuleWrapper& moduleA,
    const acousea_ModuleWrapper& moduleB)
{
    // We make sure during compilation that we have reserved enough space in SharedMemory::tmpBuffer to compare two modules
    static_assert(
        SharedMemory::tmpBufferSize() >= 2 * acousea_ModuleWrapper_size,
        "SharedMemory::tmpBuffer is too small for encoding two ModuleWrapper messages"
    );

    SharedMemory::clearTmpBuffer(); // Fundamental to clear the buffer before use to avoid garbage data during comparison

    if (moduleA.which_module != moduleB.which_module)
    {
        return false;
    }

    uint8_t* buf = SharedMemory::tmpBuffer();
    constexpr size_t HALF = SharedMemory::tmpBufferSize() / 2;

    pb_ostream_t sa = pb_ostream_from_buffer(buf, HALF);
    pb_ostream_t sb = pb_ostream_from_buffer(buf + HALF, HALF);

    if (!pb_encode(&sa, acousea_ModuleWrapper_fields, &moduleA)) return false;
    if (!pb_encode(&sb, acousea_ModuleWrapper_fields, &moduleB)) return false;

    return sa.bytes_written == sb.bytes_written &&
        memcmp(buf, buf + HALF, sa.bytes_written) == 0;
}
