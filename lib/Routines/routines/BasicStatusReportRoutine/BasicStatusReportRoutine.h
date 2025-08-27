#ifndef SIMPLEREPORTRROUTINE_H
#define SIMPLEREPORTRROUTINE_H

#include "IRoutine.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"
#include "generated/nodeDevice.pb.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


class BasicStatusReportRoutine : public IRoutine<acousea_CommunicationPacket>
{
    NodeConfigurationRepository& nodeConfigurationRepository;
    IGPS* gps;
    IBatteryController* battery;
    RTCController* rtc;

public:
    CLASS_NAME(BasicSummaryReportRoutine)

    BasicStatusReportRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                             IGPS* gps,
                             IBatteryController* battery,
                             RTCController* rtc
    );

    Result<acousea_CommunicationPacket> execute(const std::optional<_acousea_CommunicationPacket>& none) override;
};

#endif // SIMPLEREPORTRROUTINE_H
