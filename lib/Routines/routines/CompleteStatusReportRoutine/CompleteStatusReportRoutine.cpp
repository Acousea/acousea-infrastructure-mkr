#include "CompleteStatusReportRoutine.h"

#include <algorithm>

CompleteStatusReportRoutine::CompleteStatusReportRoutine(NodeConfigurationRepository& nodeConfigurationRepository,

                                                         ModuleProxy& moduleProxy,
                                                         IGPS* gps,
                                                         IBatteryController* battery,
                                                         RTCController* rtc
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),

      moduleProxy(moduleProxy),
      gps(gps),
      battery(battery),
      rtc(rtc)
{
}


Result<acousea_CommunicationPacket> CompleteStatusReportRoutine::execute(
    const std::optional<acousea_CommunicationPacket>& none)
{
    // --- Obtener configuración actual ---
    const acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    const auto reportTypeResult = getCurrentReportingConfiguration(nodeConfig);
    if (reportTypeResult.isError())
    {
        return Result<acousea_CommunicationPacket>::failure(
            getClassNameString() + "Cannot get current reporting configuration: " + reportTypeResult.getError()
        );
    }
    const acousea_ReportType& reportType = reportTypeResult.getValueConst();

    // -------- Módulos --------
    acousea_StatusReportPayload status = acousea_StatusReportPayload_init_default;
    status.modules_count = 0;

    // --- Log included modules ---
    std::string moduleIds = "[";
    for (pb_size_t i = 0; i < reportType.includedModules_count; i++)
    {
        moduleIds += std::to_string(reportType.includedModules[i]);
        if (i < reportType.includedModules_count - 1) moduleIds += ", ";
    }
    moduleIds += "]";

    Logger::logInfo("Building report packet with " +
        std::to_string(reportType.includedModules_count) +
        " modules: " + moduleIds);

    // --- Fill modules ---
    for (pb_size_t i = 0; i < reportType.includedModules_count; i++)
    {
        switch (auto moduleCode = reportType.includedModules[i])
        {
        case acousea_ModuleCode_BATTERY_MODULE:
            {
                acousea_StatusReportPayload_ModulesEntry entry =
                    acousea_StatusReportPayload_ModulesEntry_init_default;
                entry.has_value = true;
                entry.key = acousea_ModuleCode_BATTERY_MODULE;
                entry.value.which_module = acousea_ModuleWrapper_battery_tag;
                entry.value.module.battery.batteryPercentage = battery->voltageSOC_rounded();
                entry.value.module.battery.batteryStatus = battery->status();
                status.modules[status.modules_count++] = entry;
                break;
            }
        case acousea_ModuleCode_LOCATION_MODULE:
            {
                acousea_StatusReportPayload_ModulesEntry entry =
                    acousea_StatusReportPayload_ModulesEntry_init_default;
                entry.has_value = true;
                entry.key = acousea_ModuleCode_LOCATION_MODULE;
                entry.value.which_module = acousea_ModuleWrapper_location_tag;
                auto [lat, lon] = gps->read();
                entry.value.module.location.latitude = lat;
                entry.value.module.location.longitude = lon;
                status.modules[status.modules_count++] = entry;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                const auto optHF = moduleProxy.getCache().getIfFresh(
                    acousea_ModuleCode_ICLISTEN_HF);

                // Si no está fresco → la rutina queda en estado pending
                if (!optHF)
                {
                    return Result<acousea_CommunicationPacket>::pending(
                        getClassNameString() + ": ICListenHF configuration not fresh"
                    );
                }


                acousea_StatusReportPayload_ModulesEntry entry =
                    acousea_StatusReportPayload_ModulesEntry_init_default;
                entry.has_value = true;
                entry.key = acousea_ModuleCode_ICLISTEN_HF;
                entry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;
                entry.value = *optHF;
                status.modules[status.modules_count++] = entry;
                break;
            }
        default:
            Logger::logWarning(getClassNameString() + ": Module " + std::to_string(moduleCode) +
                " in ReportType not supported yet");
            break;
        }
    }

    // --- ReportBody ---
    acousea_ReportBody reportBody = acousea_ReportBody_init_default;
    reportBody.which_report = acousea_ReportBody_statusPayload_tag;
    reportBody.report.statusPayload = status;


    // --- CommunicationPacket ---
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = 1;
    pkt.routing.receiver = 0;
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_report_tag;
    pkt.body.report = reportBody;

    return Result<acousea_CommunicationPacket>::success(pkt);
}


Result<acousea_ReportType> CompleteStatusReportRoutine::getCurrentReportingConfiguration(
    const acousea_NodeConfiguration nodeConfig)
{
    // Get the current operation mode
    if (!nodeConfig.has_operationModesModule)
    {
        return Result<
            acousea_ReportType>::failure(getClassNameString() + "No operation modes module in configuration.");
    }

    if (!nodeConfig.has_reportTypesModule)
    {
        return Result<acousea_ReportType>::failure(getClassNameString() + "No report types module in configuration.");
    }

    const auto currentOperationModeId = nodeConfig.operationModesModule.activeModeId;
    const auto currentOperationModeIt = std::find_if(
        nodeConfig.operationModesModule.modes,
        nodeConfig.operationModesModule.modes + nodeConfig.operationModesModule.modes_count,
        [currentOperationModeId](const acousea_OperationMode& mode)
        {
            return mode.id == currentOperationModeId;
        }
    );

    if (currentOperationModeIt == nodeConfig.operationModesModule.modes + nodeConfig.operationModesModule.modes_count)
    {
        return Result<acousea_ReportType>::failure(
            getClassNameString() + "Current operation mode ID " + std::to_string(currentOperationModeId) +
            " not found in configuration."
        );
    }

    const auto currentReportTypeId = currentOperationModeIt->reportTypeId;

    // Find the report type
    const acousea_ReportType* reportType = std::find_if(
        nodeConfig.reportTypesModule.reportTypes, // Begin
        nodeConfig.reportTypesModule.reportTypes + nodeConfig.reportTypesModule.reportTypes_count, // End
        [currentReportTypeId](const acousea_ReportType& rt)
        {
            return rt.id == currentReportTypeId;
        }
    );

    if (reportType == nodeConfig.reportTypesModule.reportTypes + nodeConfig.reportTypesModule.reportTypes_count)
    {
        return Result<acousea_ReportType>::failure(
            getClassNameString() + "Report type ID " + std::to_string(currentReportTypeId) +
            " not found in configuration."
        );
    }

    return Result<acousea_ReportType>::success(*reportType);
}
