#include "StoreNodeConfigurationRoutine.h"

#include "Logger/Logger.h"

StoreNodeConfigurationRoutine::StoreNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    const std::optional<ICListenService*> icListenService
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(icListenService)
{
}


Result<acousea_CommunicationPacket> StoreNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& optPacket)
{
    if (!optPacket.has_value())
    {
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + "No packet provided");
    }
    const acousea_CommunicationPacket packet = optPacket.value();

    acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    if (packet.which_body != acousea_CommunicationPacket_response_tag)
    {
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + "Packet is not a response");
    }


    if (packet.body.response.which_response == acousea_ResponseBody_setConfiguration_tag)
    {
        processModules(packet.body.response.response.setConfiguration.modules,
                      packet.body.response.response.setConfiguration.modules_count);

    }
    else if (packet.body.response.which_response == acousea_ResponseBody_updatedConfiguration_tag)
    {
        processModules(packet.body.response.response.updatedConfiguration.modules,
                      packet.body.response.response.updatedConfiguration.modules_count);
    }
    else
    {
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() +
            "Packet response body does not contain any configuration modules. Wrong type: " +
            std::to_string(packet.body.response.which_response)
        );
    }

    return Result<acousea_CommunicationPacket>::success(packet);
}

// Overload para SetConfiguration
void StoreNodeConfigurationRoutine::processModules(
    const acousea_SetNodeConfigurationPayload_ModulesEntry* modules, const pb_size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        const auto& module = modules[i];
        handleModule(module.key, module.has_value, &module.value);
    }
}

// Overload para UpdatedConfiguration
void StoreNodeConfigurationRoutine::processModules(
    const acousea_UpdatedNodeConfigurationPayload_ModulesEntry* modules, const pb_size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        const auto& module = modules[i];
        handleModule(module.key, module.has_value, &module.value);
    }
}


void StoreNodeConfigurationRoutine::handleModule(int32_t key, const bool hasValue, const acousea_ModuleWrapper* value)
{
    if (!hasValue)
    {
        Logger::logError("Module with key " + std::to_string(key) + " has no value. Skipping.");
        return;
    }

    if (key == acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG ||
        key == acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG ||
        key == acousea_ModuleCode::acousea_ModuleCode_ICLISTEN_HF)
    {
        storeIcListenConfiguration(key, value);
        Logger::logInfo(getClassNameString() + " New ICListen configuration processed.");
    }
    else
    {
        Logger::logError(getClassNameString() +
            " Invalid TagType to store configuration. Key=" + std::to_string(key));
    }
}


void StoreNodeConfigurationRoutine::storeIcListenConfiguration(
    uint32_t key,
    const acousea_ModuleWrapper* value
)
{
    if (!icListenService.has_value())
    {
        Logger::logError(getClassNameString() + ": Node does not have ICListenService. Cannot store configuration.");
        return;
    }

    switch (value->which_module)
    {
    case acousea_ModuleWrapper_icListenRecordingStats_tag:
        {
            Logger::logInfo(getClassNameString() + ": Storing ICListen Recording Stats from device.");
            icListenService.value()->getCache().storeICListenRecordingStats(value->module.icListenRecordingStats);
            break;
        }
    case acousea_ModuleWrapper_icListenLoggingConfig_tag:
        {
            Logger::logInfo(getClassNameString() + ": Storing ICListen Logging Config from device.");
            icListenService.value()->getCache().storeICListenLoggingConfig(value->module.icListenLoggingConfig);
            break;
        }
    case acousea_ModuleWrapper_icListenStreamingConfig_tag:
        {
            icListenService.value()->getCache().storeICListenStreamingConfig(value->module.icListenStreamingConfig);
            Logger::logInfo(getClassNameString() + ": Storing ICListen Streaming Config from device.");
            break;
        }

    case acousea_ModuleWrapper_icListenStatus_tag:
        {
            Logger::logInfo(getClassNameString() + ": Storing ICListen Status from device.");
            icListenService.value()->getCache().storeICListenStatus(value->module.icListenStatus);
            break;
        }
    case acousea_ModuleWrapper_icListenHF_tag:
        {
            // Example: store HF config
            Logger::logInfo(getClassNameString() + ": Storing ICListen HF Config from device.");
            icListenService.value()->getCache().storeICListenHFConfiguration(value->module.icListenHF);
            break;
        }
    default:
        Logger::logError(
            getClassNameString() + ": ICListenModule with key: " + std::to_string(key) +
            " has invalid type. Skipping.");
        break;
    }
}
