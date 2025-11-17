#ifndef GETUPDATEDNODECONFIGURATIONROUTINE_HPP
#define GETUPDATEDNODECONFIGURATIONROUTINE_HPP

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"
#include "ModuleManager/ModuleManager.hpp"
#include "ModuleProxy/ModuleProxy.hpp"


class GetUpdatedNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{

private:
    bool _didRequestUpdatedModules = false;

private:
    ModuleManager& moduleManager;

public:
    CLASS_NAME(GetUpdatedNodeConfigurationRoutine)

    explicit GetUpdatedNodeConfigurationRoutine(ModuleManager& moduleManager);

    Result<acousea_CommunicationPacket*> execute(acousea_CommunicationPacket* optPacket) override;

    void reset() override;

};


#endif //GETUPDATEDNODECONFIGURATIONROUTINE_HPP
