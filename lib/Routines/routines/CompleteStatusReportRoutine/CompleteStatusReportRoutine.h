#ifndef COMPLETE_STATUS_REPORT_ROUTINE_H
#define COMPLETE_STATUS_REPORT_ROUTINE_H

#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"
#include "ModuleProxy/ModuleProxy.hpp"
#include "bindings/nodeDevice.pb.h"


/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sendFrom
 */
class CompleteStatusReportRoutine final : public IRoutine<acousea_CommunicationPacket>
{
    NodeConfigurationRepository& nodeConfigurationRepository;
    ModuleProxy& moduleProxy;
    IGPS* gps;
    IBatteryController* battery;
    RTCController* rtc;

public:
    CLASS_NAME(CompleteSummaryReportRoutine)

    CompleteStatusReportRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                                ModuleProxy& moduleProxy,
                                IGPS* gps,
                                IBatteryController* battery,
                                RTCController* rtc);

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket>& input) override;

private:
    static Result<acousea_ReportType> getCurrentReportingConfiguration(const acousea_NodeConfiguration& nodeConfig);
};

#endif //  COMPLETE_STATUS_REPORT_ROUTINE_H
