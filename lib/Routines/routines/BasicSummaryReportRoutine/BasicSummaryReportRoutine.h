#ifndef SIMPLEREPORTRROUTINE_H
#define SIMPLEREPORTRROUTINE_H

#include "IRoutine.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.h"
#include "Packet.h"
#include "Packets/BasicReportPacket.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"


class BasicSummaryReportRoutine : public IRoutine<VoidType> {
    IGPS *gps;
    IBatteryController *battery;
    RTCController *rtc;
    NodeConfigurationRepository &nodeConfigurationRepository;

public:
    CLASS_NAME(BasicSummaryReportRoutine)

    BasicSummaryReportRoutine(IGPS *gps, IBatteryController *battery, RTCController *rtc,
                              NodeConfigurationRepository &nodeConfigurationRepository);

    Result<Packet> execute() override;
};

#endif // SIMPLEREPORTRROUTINE_H
