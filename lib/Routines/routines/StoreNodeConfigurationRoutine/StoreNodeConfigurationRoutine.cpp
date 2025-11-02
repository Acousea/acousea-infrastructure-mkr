#include "StoreNodeConfigurationRoutine.h"

#include "Logger/Logger.h"

// Plantilla genérica oculta en este fichero (no exportada)
template <typename EntryT>
void StoreNodeConfigurationRoutine::processModules(const EntryT* modules, pb_size_t count)
{
    for (pb_size_t i = 0; i < count; ++i)
        handleModule(modules[i].key, modules[i].has_value, &modules[i].value);
}


StoreNodeConfigurationRoutine::StoreNodeConfigurationRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    ModuleProxy& moduleProxy
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy)
{
}

Result<acousea_CommunicationPacket> StoreNodeConfigurationRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& optPacket)
{
    if (!optPacket.has_value())
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + ": No packet provided");

    const acousea_CommunicationPacket& packet = optPacket.value();
    if (packet.which_body != acousea_CommunicationPacket_response_tag)
        return Result<acousea_CommunicationPacket>::failure(getClassNameString() + ": Packet is not a response");

    switch (packet.body.response.which_response)
    {
    case acousea_ResponseBody_setConfiguration_tag:
        processModules(packet.body.response.response.setConfiguration.modules,
                       packet.body.response.response.setConfiguration.modules_count);
        break;

    case acousea_ResponseBody_updatedConfiguration_tag:
        processModules(packet.body.response.response.updatedConfiguration.modules,
                       packet.body.response.response.updatedConfiguration.modules_count);
        break;

    default:
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + ": Packet response does not contain configuration modules. Type=" +
            std::to_string(packet.body.response.which_response));
    }

    return Result<acousea_CommunicationPacket>::success(packet);
}


void StoreNodeConfigurationRoutine::handleModule(int32_t key, bool hasValue, const acousea_ModuleWrapper* value) const
{
    if (!hasValue)
    {
        Logger::logError(
            getClassNameString() + ": Module with key " + std::to_string(key) + " has no value. Skipping.");
        return;
    }

    const auto code = static_cast<acousea_ModuleCode>(key);

    // Almacenar el módulo genéricamente en la caché
    moduleProxy.getCache().store(code, *value);

    Logger::logInfo(getClassNameString() + ": Stored module " + std::to_string(key) + " (" +
        std::to_string(value->which_module) + ") in cache.");
}

template void StoreNodeConfigurationRoutine::processModules(const acousea_SetNodeConfigurationPayload_ModulesEntry*,
                                                            pb_size_t);

template void StoreNodeConfigurationRoutine::processModules(const acousea_UpdatedNodeConfigurationPayload_ModulesEntry*,
                                                            pb_size_t);
