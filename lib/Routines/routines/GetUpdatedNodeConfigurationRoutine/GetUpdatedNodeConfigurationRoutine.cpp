#include "GetUpdatedNodeConfigurationRoutine.hpp"

#include "SharedMemory/SharedMemory.hpp"
#include "Logger/Logger.h"

GetUpdatedNodeConfigurationRoutine::GetUpdatedNodeConfigurationRoutine(
    ModuleManager& moduleManager)
    : IRoutine<acousea_CommunicationPacket>(getClassNameCString()),
      moduleManager(moduleManager)
{
}


Result<acousea_CommunicationPacket*> GetUpdatedNodeConfigurationRoutine::execute(
    acousea_CommunicationPacket* const optPacket) // Const pointer, not const data!
{
    if (!optPacket) // Check for null pointer
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "No packet provided to process");
    }
    const acousea_CommunicationPacket& inPacket = *optPacket;

    if (inPacket.which_body != acousea_CommunicationPacket_command_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "Packet is not of type command");
    }

    if (inPacket.body.command.which_command != acousea_CommandBody_requestedConfiguration_tag)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "Packet command is not of type requestedConfiguration");
    }


    // Copy the requested modules beforehand (the packet later is overwritten to build the response)
    const acousea_GetUpdatedNodeConfigurationPayload reqConfigPayloadCopy = inPacket.body.command.command.
        requestedConfiguration;

    const acousea_ModuleCode* reqModules = reqConfigPayloadCopy.requestedModules;
    const pb_size_t& modulesCount = reqConfigPayloadCopy.requestedModules_count;

    // ----------------  Prepare response packet ----------------
    SharedMemory::resetCommunicationPacket();
    acousea_CommunicationPacket& responsePacket = SharedMemory::communicationPacketRef();
    // Routing
    responsePacket.has_routing = true;
    responsePacket.routing = acousea_RoutingChunk_init_default;
    responsePacket.routing.sender = inPacket.routing.receiver;
    responsePacket.routing.receiver = inPacket.routing.sender;
    responsePacket.routing.ttl = 0;

    // Payload
    responsePacket.which_body = acousea_CommunicationPacket_response_tag;
    responsePacket.body.response = acousea_ResponseBody_init_default;
    responsePacket.body.response.which_response = acousea_ResponseBody_updatedConfiguration_tag;
    responsePacket.body.response.response.updatedConfiguration = acousea_UpdatedNodeConfigurationPayload_init_default;

    auto& updatedConfigPayloadRef = responsePacket.body.response.response.
                                                   updatedConfiguration;

    auto* outModulesArray = reinterpret_cast<acousea_NodeDevice_ModulesEntry*>(updatedConfigPayloadRef.modules);
    pb_size_t& outModulesCount = updatedConfigPayloadRef.modules_count = 0;

    const auto resOk = moduleManager.getModules(outModulesArray, outModulesCount, reqModules, modulesCount);

    LOG_CLASS_INFO("GetUpdatedNodeConfigurationRoutine::execute() -> Retrieved %d updated modules", outModulesCount);


    return resOk.isSuccess()
               ? RESULT_SUCCESS(acousea_CommunicationPacket*, &responsePacket)
               : RESULT_CLASS_FAILUREF(
                   acousea_CommunicationPacket*,
                   "Failed to get updated node configuration modules: %s",
                   resOk.getError()
               );
}
