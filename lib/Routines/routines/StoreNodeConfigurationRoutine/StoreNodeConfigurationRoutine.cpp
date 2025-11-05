#include "StoreNodeConfigurationRoutine.h"

#include <cinttypes>

#include "Logger/Logger.h"

// Plantilla genérica oculta en este fichero (no exportada)
template <typename EntryT>
Result<void> processModules(
    ModuleProxy::ModuleCache& workingCache,
    const EntryT* modules, const pb_size_t modules_count)
{
    for (pb_size_t i = 0; i < modules_count; ++i)
    {
        const auto& entry = modules[i];
        if (!entry.has_value)
        {
            return RESULT_VOID_FAILUREF("Module with key %" PRId32 " has no value", entry.key);
        }

        const auto code = static_cast<acousea_ModuleCode>(entry.key);
        workingCache.store(code, entry.value);
    }

    return RESULT_VOID_SUCCESS();
}

StoreNodeConfigurationRoutine::StoreNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    ModuleProxy& moduleProxy)
    : IRoutine(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy)
{
}

Result<acousea_CommunicationPacket> StoreNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& optPacket)
{
    if (!optPacket.has_value())
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "No packet provided");
    }

    const acousea_CommunicationPacket& packet = optPacket.value();
    if (packet.which_body != acousea_CommunicationPacket_response_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "Packet is not a response");
    }

    auto workingCache = moduleProxy.getCache().clone();
    Result<void> processResult = RESULT_VOID_SUCCESS();

    switch (packet.body.response.which_response)
    {
    case acousea_ResponseBody_setConfiguration_tag:
        processResult = processModules(
            workingCache,
            packet.body.response.response.setConfiguration.modules,
            packet.body.response.response.setConfiguration.modules_count);
        break;

    case acousea_ResponseBody_updatedConfiguration_tag:
        processResult = processModules(
            workingCache,
            packet.body.response.response.updatedConfiguration.modules,
            packet.body.response.response.updatedConfiguration.modules_count
        );
        break;

    default:
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket,
                                     "Packet response does not contain configuration modules. Type=%d",
                                     packet.body.response.which_response);
    }

    if (processResult.isError())
    {
        LOG_CLASS_ERROR(": Cache changes discarded due to error: %s", processResult.getError());
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket, "Error processing modules: %s",
                                     processResult.getError());
    }

    moduleProxy.getCache().swap(workingCache);
    LOG_CLASS_INFO("Stored modules from node configuration packet in cache.");
    return RESULT_SUCCESS(acousea_CommunicationPacket, packet);
}
