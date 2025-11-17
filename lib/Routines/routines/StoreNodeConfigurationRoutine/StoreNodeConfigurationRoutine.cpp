#include "StoreNodeConfigurationRoutine.h"

#include <cinttypes>


#include "Logger/Logger.h"
#include "RollbackAgent/RollbackAgent.hpp"


StoreNodeConfigurationRoutine::StoreNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    ModuleProxy& moduleProxy)
    : IRoutine(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy)
{
}

Result<acousea_CommunicationPacket*> StoreNodeConfigurationRoutine::execute(
    acousea_CommunicationPacket* optPacket)
{
    if (!optPacket) // Check for null pointer
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "No packet provided");
    }

    acousea_CommunicationPacket& inPacket = *optPacket;

    if (inPacket.which_body != acousea_CommunicationPacket_response_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "Packet is not a response");
    }

    bool processOk = false;

    switch (inPacket.body.response.which_response)
    {
    case acousea_ResponseBody_setConfiguration_tag:
        processOk = _storeModules(
            reinterpret_cast<const acousea_NodeDevice_ModulesEntry*>(
                inPacket.body.response.response.setConfiguration.modules
            ),
            inPacket.body.response.response.setConfiguration.modules_count);
        break;

    case acousea_ResponseBody_updatedConfiguration_tag:
        processOk = _storeModules(
            reinterpret_cast<const acousea_NodeDevice_ModulesEntry*>(
                inPacket.body.response.response.updatedConfiguration.modules
            ),
            inPacket.body.response.response.updatedConfiguration.modules_count
        );
        break;

    default:
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "Packet response does not contain configuration modules. Type=%d",
                                     inPacket.body.response.which_response);
    }

    if (!processOk)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "Error processing modules: could not register actions");
    }

    LOG_CLASS_INFO("Stored modules from node configuration packet in cache.");

    return RESULT_SUCCESS(acousea_CommunicationPacket*, nullptr); // Do not return any packet
}

bool StoreNodeConfigurationRoutine::_storeModules(
    const acousea_NodeDevice_ModulesEntry* modules, const uint16_t modules_count
)
{
    for (uint16_t i = 0; i < modules_count; ++i)
    {
        const acousea_NodeDevice_ModulesEntry& moduleEntry = modules[i];
        if (!moduleEntry.has_value)
        {
            LOG_CLASS_INFO("Skipping module with tag %lu since it has no value", moduleEntry.key);
            continue;
        }
        if (const bool storeOk = moduleProxy.storeModule(moduleEntry.value); !storeOk)
        {
            LOG_CLASS_ERROR("Failed to store module with tag %lu", moduleEntry.key);
            return false;
        }
        LOG_CLASS_INFO("Stored module with tag %lu successfully", moduleEntry.key);
    }
    return true;
}

void StoreNodeConfigurationRoutine::reset()
{
    LOG_CLASS_INFO("::reset() -> Routine state reset.");
}
