#ifndef GETUPDATEDNODECONFIGURATIONROUTINE_HPP
#define GETUPDATEDNODECONFIGURATIONROUTINE_HPP

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"
#include "ModuleProxy/ModuleProxy.hpp"


class GetUpdatedNodeConfigurationRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    ModuleProxy& moduleProxy;
    IGPS* gps;
    IBatteryController* battery;
    RTCController* rtcController;

public:
    CLASS_NAME(GetUpdatedNodeConfigurationRoutine)

    GetUpdatedNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        ModuleProxy& moduleProxy,
        IGPS* gps,
        IBatteryController* battery,
        RTCController* rtcController
    );

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& optPacket) override;
};


#endif //GETUPDATEDNODECONFIGURATIONROUTINE_HPP
