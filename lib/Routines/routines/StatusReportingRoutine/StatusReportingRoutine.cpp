#include "StatusReportingRoutine.h"

#include <algorithm>
#include <cstdio>

#include "Logger/Logger.h"
#include "SharedMemory/SharedMemory.hpp"

StatusReportingRoutine::StatusReportingRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                                               ModuleManager& moduleManager)
    : IRoutine(getClassNameCString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      moduleManager(moduleManager)

{
}


Result<acousea_CommunicationPacket*> StatusReportingRoutine::execute(
    acousea_CommunicationPacket* const /*optPacket*/) // input unused, always nullptr
{
    LOG_FREE_MEMORY("PRENODECONFIG");
    // --- Obtener configuración actual ---
    const acousea_NodeConfiguration& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    LOG_FREE_MEMORY("POSTNODECONFIG");

    const auto reportTypePtr = getCurrentReportingConfiguration(nodeConfig);
    if (!reportTypePtr)
    {
        return RESULT_CLASS_FAILUREF(acousea_CommunicationPacket*, "%s",
                                     "Cannot get current reporting configuration: missing operation mode or report type");
    }
    LOG_FREE_MEMORY("POSTREPORTTYPE");
    const acousea_ReportType& reportType = *reportTypePtr;

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
    pkt.routing.receiver = Router::originAddress;
    pkt.routing.ttl = 5;

    pkt.which_body = acousea_CommunicationPacket_report_tag;
    // Inicializar directamente la rama elegida
    pkt.body.report = acousea_ReportBody_init_default;
    pkt.body.report.which_report = acousea_ReportBody_statusPayload_tag;
    pkt.body.report.report = acousea_StatusReportPayload_init_default;

    acousea_StatusReportPayload& statusReportPayload = pkt.body.report.report.statusPayload;
    statusReportPayload.reportTypeId = reportType.id; // Asignar el ID del tipo de reporte

    // auto* outModulesArr = status.modules;
    auto* outModulesArr = reinterpret_cast<acousea_NodeDevice_ModulesEntry*>(statusReportPayload.modules);
    auto& outModulesArrSize = statusReportPayload.modules_count;


    const auto& [reportId,
        reportName,
        reportIncludedModulesCount,
        reportIncludedModules] = reportType;

    const auto voidResult = moduleManager.getModules(
        outModulesArr,
        outModulesArrSize,
        reportIncludedModules,
        reportIncludedModulesCount
    );

    LOG_CLASS_WARNING("Resulting report has %d of %d requested modules.",
                      outModulesArrSize, reportIncludedModulesCount);

    switch (voidResult.getStatus())
    {
    case Result<void>::Type::Success:
        {
            return RESULT_SUCCESS(acousea_CommunicationPacket*, &pkt);
        }

    case Result<void>::Type::Incomplete:
        {
            return RESULT_CLASS_FAILUREF(
                acousea_CommunicationPacket*,
                "Incomplete while building status report modules: %s",
                voidResult.getError()
            );
        }

    case Result<void>::Type::Failure:
        {
            return RESULT_CLASS_FAILUREF(
                acousea_CommunicationPacket*,
                "Error while building status report modules: %s",
                voidResult.getError()
            );
        }
    default:
        {
            return RESULT_CLASS_FAILUREF(
                acousea_CommunicationPacket*,
                "Unknown error while building status report modules."
            );
        }
    }
}


const acousea_ReportType* StatusReportingRoutine::getCurrentReportingConfiguration(
    const acousea_NodeConfiguration& nodeConfig)
{
    // Get the current operation mode
    if (!nodeConfig.has_operationModesModule)
    {
        LOG_CLASS_ERROR("No operation modes module in configuration.");
        return nullptr;
    }

    if (!nodeConfig.has_reportTypesModule)
    {
        LOG_CLASS_ERROR("No report types module in configuration.");
        return nullptr;
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
        LOG_CLASS_ERROR("Current operation mode ID %ld not found in configuration.", currentOperationModeId);
        return nullptr;
    }

    const auto currentReportTypeId = currentOperationModeIt->reportTypeId;

    // Find the report type
    const acousea_ReportType* reportType = std::find_if(
        nodeConfig.reportTypesModule.reportTypes, // Inicio del arreglo
        nodeConfig.reportTypesModule.reportTypes + nodeConfig.reportTypesModule.reportTypes_count, // Fin del arreglo
        [currentReportTypeId](const acousea_ReportType& rt)
        {
            return rt.id == currentReportTypeId;
        }
    );


    if (reportType == nodeConfig.reportTypesModule.reportTypes + nodeConfig.reportTypesModule.reportTypes_count)
    {
        LOG_CLASS_ERROR("Report type ID %ld not found in configuration.", currentReportTypeId);
        return nullptr;
    }

    return reportType;
}
