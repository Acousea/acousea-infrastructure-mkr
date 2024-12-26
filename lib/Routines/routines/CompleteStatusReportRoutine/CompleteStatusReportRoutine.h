#ifndef SUMMARYROUTINE_H
#define SUMMARYROUTINE_H

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Packet.h"
#include "Packets/ErrorPacket.h"
#include "RTCController.h"
#include "IBatteryController.h"
#include "IGPS.h"

/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sendFrom
 */
class CompleteStatusReportRoutine : public IRoutine<VoidType> {
    IGPS *gps;
    IBatteryController *battery;
    RTCController *rtc;
    NodeConfigurationRepository &nodeConfigurationRepository;

public:
    CLASS_NAME(CompleteSummaryReportRoutine)

    CompleteStatusReportRoutine(NodeConfigurationRepository &nodeConfigurationRepository);

    CompleteStatusReportRoutine(const std::string &name, IGPS *gps, IBatteryController *battery, RTCController *rtc,
                                 NodeConfigurationRepository &nodeConfigurationRepository);

    Result<Packet> execute() override;

};

#endif // SUMMARYROUTINE_H
