#include "CompleteStatusReportRoutine.h"

#include <algorithm>
#include <cstdio>

#include "Logger/Logger.h"
#include "SharedMemory/SharedMemory.hpp"

CompleteStatusReportRoutine::CompleteStatusReportRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                                                         ModuleProxy& moduleProxy,
                                                         IGPS& gps,
                                                         IBatteryController& battery,
                                                         RTCController& rtc
)
    : IRoutine(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleProxy(moduleProxy),
      gps(gps),
      battery(battery),
      rtc(rtc)
{
}


Result<acousea_CommunicationPacket*> CompleteStatusReportRoutine::execute(
    acousea_CommunicationPacket* const /*optPacket*/) // input unused, always nullptr
{
    LOG_CLASS_WARNING("Executing CompleteStatusReportRoutine...");

    LOG_FREE_MEMORY("PRENODECONFIG");
    // --- Obtener configuración actual ---
    const acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    LOG_FREE_MEMORY("POSTNODECONFIG");

    const auto reportTypeResult = getCurrentReportingConfiguration(nodeConfig);
    if (reportTypeResult.isError())
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "Cannot get current reporting configuration: %s",
                                     reportTypeResult.getError());
    }
    LOG_FREE_MEMORY("POSTREPORTTYPE");
    const acousea_ReportType& reportType = reportTypeResult.getValueConst();

    // --- Log included modules ---
    char moduleIds[128];
    size_t pos = 0;
    pos += snprintf(moduleIds + pos, sizeof(moduleIds) - pos, "[");
    for (uint16_t i = 0; i < reportType.includedModules_count && pos < sizeof(moduleIds) - 4; i++)
    {
        pos += snprintf(moduleIds + pos, sizeof(moduleIds) - pos, "%d", reportType.includedModules[i]);
        if (i < reportType.includedModules_count - 1) pos += snprintf(moduleIds + pos, sizeof(moduleIds) - pos, ", ");
    }
    snprintf(moduleIds + pos, sizeof(moduleIds) - pos, "]");
    LOG_CLASS_INFO("Building report packet with %d modules: %s", reportType.includedModules_count, moduleIds);

    LOG_FREE_MEMORY("POSTMODULEIDS");

    // --- CommunicationPacket ---
    SharedMemory::resetCommunicationPacket();
    acousea_CommunicationPacket& pkt = SharedMemory::communicationPacketRef();
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = 1;
    pkt.routing.receiver = 0;
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_report_tag;
    // Inicializar directamente la rama elegida
    pkt.body.report = acousea_ReportBody_init_default;
    pkt.body.report.which_report = acousea_ReportBody_statusPayload_tag;
    acousea_StatusReportPayload& status = pkt.body.report.report.statusPayload;
    status.modules_count = 0;

    LOG_FREE_MEMORY("POSTSTATUSINIT");
    for (uint16_t i = 0; i < reportType.includedModules_count; i++)
    {
        const auto code = reportType.includedModules[i];
        auto& entry = status.modules[status.modules_count++];
        entry.has_value = true;
        entry.key = code;

        switch (code)
        {
        case acousea_ModuleCode_BATTERY_MODULE:
            {
                entry.value.which_module = acousea_ModuleWrapper_battery_tag;
                entry.value.module.battery.batteryPercentage = battery.voltageSOC_rounded();
                entry.value.module.battery.batteryStatus = battery.status();
                break;
            }
        case acousea_ModuleCode_LOCATION_MODULE:
            {
                entry.value.which_module = acousea_ModuleWrapper_location_tag;
                auto [lat, lon] = gps.read();
                entry.value.module.location.latitude = lat;
                entry.value.module.location.longitude = lon;
                break;
            }
        case acousea_ModuleCode_ICLISTEN_HF:
            {
                const auto optHF = moduleProxy.getCache().getIfFresh(
                    acousea_ModuleCode_ICLISTEN_HF);

                // Si no está fresco → la rutina queda en estado pending
                if (!optHF)
                {
                    return RESULT_CLASS_PENDINGF(acousea_CommunicationPacket*, ": ICListenHF module data not fresh");
                }

                entry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;
                entry.value = *optHF;
                break;
            }
        default:
            LOG_CLASS_WARNING(": Module %d in ReportType not supported yet", code);
            status.modules_count--; // remove unused slot
            break;
        }
    }


    LOG_CLASS_WARNING("Report built with %d modules.", status.modules_count);


    LOG_CLASS_INFO("Returning report packet with %d modules", status.modules_count);

    return RESULT_SUCCESS(acousea_CommunicationPacket*, &pkt);
}


Result<acousea_ReportType> CompleteStatusReportRoutine::getCurrentReportingConfiguration(
    const acousea_NodeConfiguration& nodeConfig)
{
    // Get the current operation mode
    if (!nodeConfig.has_operationModesModule)
    {
        return RESULT_CLASS_FAILUREF(acousea_ReportType, "No operation modes module in configuration.");
    }

    if (!nodeConfig.has_reportTypesModule)
    {
        return RESULT_CLASS_FAILUREF(acousea_ReportType, "No report types module in configuration.");
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
        return RESULT_CLASS_FAILUREF(acousea_ReportType, "Current operation mode ID %ld not found in configuration.",
                                     currentOperationModeId);
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
        return RESULT_CLASS_FAILUREF(acousea_ReportType, "Report type ID %ld not found in configuration.",
                                     currentReportTypeId);
    }

    return RESULT_SUCCESS(acousea_ReportType, *reportType);
}
