#ifndef SUMMARYROUTINE_H
#define SUMMARYROUTINE_H

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Packet.h"
#include "IBatteryController.h"
#include "IGPS.h"
#include <Packets/reports/CompleteStatusReportPacket.h>
#include <ICListenService/ICListenService.h>

class ICListenService;
/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sendFrom
 */
class CompleteStatusReportRoutine : public IRoutine<VoidType> {
    IGPS *gps;
    IBatteryController *battery;
    NodeConfigurationRepository &nodeConfigurationRepository;
    ICListenService &icListenService;

public:
    CLASS_NAME(CompleteSummaryReportRoutine)

    CompleteStatusReportRoutine(
        IGPS *gps,
        IBatteryController *battery,
        NodeConfigurationRepository &nodeConfigurationRepository,
        ICListenService &icListenService
    );

    Result<Packet> execute() override;
};

#endif // SUMMARYROUTINE_H
