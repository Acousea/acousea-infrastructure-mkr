//
// Created by admin on 24/08/25.
//

#ifndef GETUPDATEDNODECONFIGURATIONROUTINE_HPP
#define GETUPDATEDNODECONFIGURATIONROUTINE_HPP


#include "IRoutine.h"
#include "ICListenService/ICListenService.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Logger/Logger.h"
#include "Result/Result.h"
#include "IGPS.h"
#include "IBatteryController.h""
#include "RTCController.hpp"


class GetUpdatedNodeConfigurationRoutine : public IRoutine<acousea_CommunicationPacket>
{
private:
    NodeConfigurationRepository& nodeConfigurationRepository;
    std::optional<std::shared_ptr<ICListenService>> icListenService;
    IGPS* gps;
    IBatteryController* battery;
    RTCController* rtcController;

public:
    CLASS_NAME(GetUpdatedNodeConfigurationRoutine)

    explicit GetUpdatedNodeConfigurationRoutine(
        NodeConfigurationRepository& nodeConfigurationRepository,
        std::optional<std::shared_ptr<ICListenService>> icListenService,
        IGPS* gps,
        IBatteryController* battery,
        RTCController* rtcController
    );

    Result<acousea_CommunicationPacket> execute(const acousea_CommunicationPacket& packet) override;
};


#endif //GETUPDATEDNODECONFIGURATIONROUTINE_HPP
