#ifndef COMPLETE_STATUS_REPORT_ROUTINE_H
#define COMPLETE_STATUS_REPORT_ROUTINE_H

#include "IRoutine.h"
#include "bindings/nodeDevice.pb.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ModuleManager/ModuleManager.hpp"


/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sendFrom
 */
class StatusReportingRoutine final : public IRoutine<acousea_CommunicationPacket>
{
    NodeConfigurationRepository& nodeConfigurationRepository;
    ModuleManager& moduleManager;

private:
    bool _didRequestUpdatedModules = false;

public:
    CLASS_NAME(StatusReportingRoutine)

    StatusReportingRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                                ModuleManager& moduleManager);

    Result<acousea_CommunicationPacket*> execute(acousea_CommunicationPacket* const /*optPacket*/) override;

    void reset() override;

private:
    static const acousea_ReportType* getCurrentReportingConfiguration(const acousea_NodeConfiguration& nodeConfig);
};

#endif //  COMPLETE_STATUS_REPORT_ROUTINE_H
