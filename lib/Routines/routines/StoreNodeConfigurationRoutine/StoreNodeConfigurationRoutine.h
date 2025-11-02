#ifndef STORENODECONFIGURATIONROUTINE_H
#define STORENODECONFIGURATIONROUTINE_H

#include "IRoutine.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"

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

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& optPacket) override;

private:
    template <typename EntryT>
    void processModules(const EntryT*, pb_size_t);
    void handleModule(int32_t key, bool hasValue, const acousea_ModuleWrapper* value) const;
};


#endif //STORENODECONFIGURATIONROUTINE_H
