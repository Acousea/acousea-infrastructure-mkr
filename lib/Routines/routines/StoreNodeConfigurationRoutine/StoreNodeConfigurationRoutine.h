#ifndef STORENODECONFIGURATIONROUTINE_H
#define STORENODECONFIGURATIONROUTINE_H

#include "IRoutine.h"
#include "ICListenService/ICListenService.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"

class StoreNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    std::optional<ICListenService*> icListenService;

public:
    CLASS_NAME(StoreNodeConfigurationRoutine)

    explicit StoreNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        std::optional<ICListenService*> icListenService
    );

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& optPacket) override;
    void processModules(const acousea_SetNodeConfigurationPayload_ModulesEntry* modules, pb_size_t count);
    void processModules(const acousea_UpdatedNodeConfigurationPayload_ModulesEntry* modules, pb_size_t count);

private:
    void storeIcListenConfiguration(uint32_t key, const acousea_ModuleWrapper* value);
    void handleModule(int32_t key, bool hasValue, const acousea_ModuleWrapper* value
    );
    // ---------------------- ICListen specific methods ----------------------
};


#endif //STORENODECONFIGURATIONROUTINE_H
