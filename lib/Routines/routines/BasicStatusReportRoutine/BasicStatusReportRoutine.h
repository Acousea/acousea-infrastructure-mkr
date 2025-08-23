#ifndef SIMPLEREPORTRROUTINE_H
#define SIMPLEREPORTRROUTINE_H

#include "IRoutine.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"
#include "generated/nodeDevice.pb.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


class BasicStatusReportRoutine : public IRoutine<VoidType> {
    IGPS *gps;
    IBatteryController *battery;
    RTCController *rtc;
    NodeConfigurationRepository &nodeConfigurationRepository;

public:
    CLASS_NAME(BasicSummaryReportRoutine)

    BasicStatusReportRoutine(IGPS *gps, IBatteryController *battery, RTCController *rtc,
                             NodeConfigurationRepository &nodeConfigurationRepository);

    Result<acousea_CommunicationPacket> execute() override;
};

#endif // SIMPLEREPORTRROUTINE_H
