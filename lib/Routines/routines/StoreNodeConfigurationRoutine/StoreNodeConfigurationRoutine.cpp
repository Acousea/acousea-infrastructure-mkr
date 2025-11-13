#include "StoreNodeConfigurationRoutine.h"

#include <cinttypes>


#include "Logger/Logger.h"
#include "RollbackAgent/RollbackAgent.hpp"

// PUNTEROS en vez de referencias -> default-constructible y sin copias
struct StoreCtx
{
    ModuleProxy::ModuleCache* cache{nullptr};
    acousea_ModuleCode code{};
    const acousea_ModuleWrapper* wrapper{nullptr};
};

// ====================== Acción de commit ======================
void storeModuleAction(void* ctx)
{
    const auto* data = static_cast<StoreCtx*>(ctx);
    const bool storeOk = data->cache->store(data->code, *data->wrapper);
    if (!storeOk)
    {
        LOG_CLASS_ERROR("storeModuleAction() -> Failed to store module with key %" PRId32 " in cache",
                        static_cast<int32_t>(data->code));
        return;
    }
    LOG_CLASS_INFO("Processed module with key %" PRId32 " into working cache",
                   static_cast<int32_t>(data->code));
}

// ====================== Procesamiento genérico ======================

template <typename EntryT>
Result<void> processModules(
    RollbackAgent& agent,
    ModuleProxy::ModuleCache& cache,
    const EntryT* modules,
    const uint16_t modules_count)
{

    static StoreCtx contexts[RollbackAgent::MAX_ACTIONS]; // Max possible modules (static allocation to preserve
    size_t ctxIndex = 0;

    for (uint16_t i = 0; i < modules_count; ++i)
    {
        const auto& entry = modules[i];
        if (!entry.has_value)
        {
            return RESULT_VOID_FAILUREF("Module with key %" PRId32 " has no value", entry.key);
        }

        if (ctxIndex >= std::size(contexts))
        {
            return RESULT_VOID_FAILUREF("Too many modules in payload (%d)", modules_count);
        }

        StoreCtx& ctx = contexts[ctxIndex++];
        ctx.cache = &cache;
        ctx.code = static_cast<acousea_ModuleCode>(entry.key);
        ctx.wrapper = &entry.value;

        agent.registerAction(&storeModuleAction, &ctx);
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

    RollbackAgent rollbackAgent;
    auto& workingCache = moduleProxy.getCache();
    Result<void> processResult = RESULT_VOID_SUCCESS();

    switch (inPacket.body.response.which_response)
    {
    case acousea_ResponseBody_setConfiguration_tag:
        processResult = processModules(
            rollbackAgent,
            workingCache,
            inPacket.body.response.response.setConfiguration.modules,
            inPacket.body.response.response.setConfiguration.modules_count);
        break;

    case acousea_ResponseBody_updatedConfiguration_tag:
        processResult = processModules(
            rollbackAgent,
            workingCache,
            inPacket.body.response.response.updatedConfiguration.modules,
            inPacket.body.response.response.updatedConfiguration.modules_count
        );
        break;

    default:
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*,
                                     "Packet response does not contain configuration modules. Type=%d",
                                     inPacket.body.response.which_response);
    }

    if (processResult.isError())
    {
        LOG_CLASS_ERROR(": Cache changes discarded due to error: %s", processResult.getError());
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "Error processing modules: %s",
                                     processResult.getError());
    }

    // Commit all cache stores
    rollbackAgent.commit();

    LOG_CLASS_INFO("Stored modules from node configuration packet in cache.");

    return RESULT_SUCCESS(acousea_CommunicationPacket*, &inPacket); // Return the same packet
}
