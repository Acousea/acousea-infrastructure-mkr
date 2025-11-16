#ifndef STORENODECONFIGURATIONROUTINE_H
#define STORENODECONFIGURATIONROUTINE_H

#include "IRoutine.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "RollbackAgent/RollbackAgent.hpp"


class StoreNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    ModuleProxy& moduleProxy;

public:
    CLASS_NAME(StoreNodeConfigurationRoutine)


    explicit StoreNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        ModuleProxy& moduleProxy
    );

    Result<acousea_CommunicationPacket*> execute(acousea_CommunicationPacket* optPacket) override;

private:
    bool _storeModules(const acousea_NodeDevice_ModulesEntry* modules, uint16_t modules_count);
};


#endif //STORENODECONFIGURATIONROUTINE_H
