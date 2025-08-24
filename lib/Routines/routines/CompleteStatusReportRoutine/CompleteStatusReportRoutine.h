#ifndef COMPLETE_STATUS_REPORT_ROUTINE_H
#define COMPLETE_STATUS_REPORT_ROUTINE_H

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "IBatteryController.h"
#include "IGPS.h"
#include <ICListenService/ICListenService.h>

/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sendFrom
 */
class CompleteStatusReportRoutine final : public IRoutine<VoidType> {
    NodeConfigurationRepository &nodeConfigurationRepository;
    std::optional<std::shared_ptr<ICListenService>> icListenService;
    IGPS *gps;
    IBatteryController *battery;

public:
    CLASS_NAME(CompleteSummaryReportRoutine)

    CompleteStatusReportRoutine(
        NodeConfigurationRepository &nodeConfigurationRepository,
        std::optional<std::shared_ptr<ICListenService>> icListenService,
        IGPS *gps,
        IBatteryController *battery
    );

    Result<acousea_CommunicationPacket> execute() override;
};

#endif //  COMPLETE_STATUS_REPORT_ROUTINE_H
