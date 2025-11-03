#include "NodeOperationRunner.h"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <climits> // for  ULONG_MAX
#include <cinttypes> // for PRId32


// -----------------------------------------------------------------------------
// Optimized tag-to-string helpers (const char* version)
// -----------------------------------------------------------------------------

inline const char* bodyTagToCString(uint8_t tag)
{
    switch (tag)
    {
    case acousea_CommunicationPacket_command_tag: return "Command";
    case acousea_CommunicationPacket_response_tag: return "Response";
    case acousea_CommunicationPacket_report_tag: return "Report";
    case acousea_CommunicationPacket_error_tag: return "Error";
    default: return "UnknownBody";
    }
}

inline const char* commandPayloadTagToCString(uint8_t tag)
{
    switch (tag)
    {
    case acousea_CommandBody_setConfiguration_tag: return "Command->SetConfiguration";
    case acousea_CommandBody_requestedConfiguration_tag: return "Command->RequestedConfiguration";
    default: return "Command->Unknown";
    }
}

inline const char* responsePayloadTagToCString(uint8_t tag)
{
    switch (tag)
    {
    case acousea_ResponseBody_setConfiguration_tag: return "Response->SetConfiguration";
    case acousea_ResponseBody_updatedConfiguration_tag: return "Response->UpdatedConfiguration";
    default: return "Response->Unknown";
    }
}

inline const char* reportPayloadTagToCString(uint8_t tag)
{
    switch (tag)
    {
    case acousea_ReportBody_statusPayload_tag: return "Report->StatusPayload";
    default: return "Report->Unknown";
    }
}


NodeOperationRunner::NodeOperationRunner(Router& router,
                                         const NodeConfigurationRepository& nodeConfigurationRepository,
                                         const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>&
                                         commandRoutines,
                                         const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>&
                                         responseRoutines,
                                         const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>&
                                         reportRoutines
) : IRunnable(),
    router(router),
    commandRoutines(commandRoutines),
    responseRoutines(responseRoutines),
    reportRoutines(reportRoutines),
    nodeConfigurationRepository(nodeConfigurationRepository)
{
    cache = {
        acousea_OperationMode_init_default,
        0,
        {ULONG_MAX, ULONG_MAX}
    };
}

void NodeOperationRunner::init()
{
    currentNodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    LOG_CLASS_INFO("<Init> Operation Cycle for Operation Mode %" PRId32 "d=(%s) with configuration:",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
    nodeConfigurationRepository.printNodeConfiguration(*currentNodeConfiguration);
    // Search for the operation mode with the given ID
    const auto opModeResult = searchForOperationMode(currentNodeConfiguration->operationModesModule.activeModeId);
    if (opModeResult.isError())
    {
        ERROR_HANDLE_CLASS(": Initial operation mode not found: %s", opModeResult.getError());
        return;
    }

    cache.currentOperationMode = opModeResult.getValueConst();
}

void NodeOperationRunner::run()
{
    LOG_CLASS_INFO("<Run> Operation Cycle for Operation mode %" PRId32 "d=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
    checkIfMustTransition();
    processIncomingPackets(currentNodeConfiguration->localAddress);
    runPendingRoutines();
    processReportingRoutines();
    cache.cycleCount++;
    LOG_CLASS_INFO("<Finish> Operation Cycle for Operation mode %" PRId32 "d=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
}


void NodeOperationRunner::checkIfMustTransition()
{
    if (!cache.currentOperationMode.has_transition)
    {
        ERROR_HANDLE_CLASS("No transition defined for current operation mode.");
        return;
    }
    if (const auto maxDuration = cache.currentOperationMode.transition.duration; cache.cycleCount >= maxDuration)
    {
        cache.cycleCount = 0;
        const auto nextOpModeResult = searchForOperationMode(cache.currentOperationMode.transition.targetModeId);
        if (nextOpModeResult.isError())
        {
            ERROR_HANDLE_CLASS(": Operation mode not found: %s", nextOpModeResult.getError());
            return;
        }
        cache.currentOperationMode = nextOpModeResult.getValueConst();
        LOG_CLASS_INFO("Transitioned to next mode... %" PRId32 "d=(%s)",
                       cache.currentOperationMode.id,
                       cache.currentOperationMode.name
        );
    }
}

void NodeOperationRunner::tryReport(const IPort::PortType portType,
                                    unsigned long& lastMinute,
                                    const unsigned long currentMinute)
{
    const auto cfg = getReportingEntryForCurrentOperationMode(
        cache.currentOperationMode.id, portType
    );

    if (cfg.isError())
    {
        LOG_CLASS_ERROR("%s", cfg.getError());
        return;
    }

    auto [modeId, period] = cfg.getValueConst();

    LOG_CLASS_INFO("%s Config: { Period=%lu, Current minute=%lu, Last report minute=%lu }",
                   IPort::portTypeToCString(portType),
                   period,
                   currentMinute,
                   lastMinute
    );

    if (!mustReport(currentMinute, period, lastMinute))
    {
        LOG_CLASS_INFO("%s Not time to report yet. Skipping report...", IPort::portTypeToCString(portType));
        return;
    }

    const auto it = reportRoutines.find(acousea_ReportBody_statusPayload_tag);
    if (it == reportRoutines.end() || it->second == nullptr)
    {
        LOG_CLASS_ERROR("Report routine with tag %d not found. Skipping report...",
                        acousea_ReportBody_statusPayload_tag);

        return;
    }

    auto result = executeRoutine(it->second, std::nullopt, portType, false);
    if (result.has_value())
    {
        sendResponsePacket(portType, currentNodeConfiguration->localAddress, *result);
    }

    lastMinute = currentMinute;
}


void NodeOperationRunner::processReportingRoutines()
{
    const auto currentMinute = getMillis() / 60000;

    if (!currentNodeConfiguration.has_value())
    {
        ERROR_HANDLE_CLASS(" No current node configuration loaded.");
        return;
    }
    currentNodeConfiguration->has_loraModule
        ? tryReport(IPort::PortType::LoraPort, cache.lastReportMinute.lora, currentMinute)
        : LOG_CLASS_INFO("LoRa module not present, skipping LoRa report.");

    currentNodeConfiguration->has_iridiumModule
        ? tryReport(IPort::PortType::SBDPort, cache.lastReportMinute.sbd, currentMinute)
        : LOG_CLASS_INFO("Iridium module not present, skipping Iridium report.");

    currentNodeConfiguration->has_gsmMqttModule
        ? tryReport(IPort::PortType::GsmMqttPort, cache.lastReportMinute.gsmMqtt, currentMinute)
        : LOG_CLASS_INFO("GSM-MQTT module not present, skipping GSM-MQTT report.");
}

void NodeOperationRunner::processIncomingPackets(const uint8_t& localAddress)
{
    auto receivedPackets = router.readPorts(localAddress);
    for (const auto& [portType, packets] : receivedPackets)
    {
        for (auto& packet : packets)
        {
            std::optional<acousea_CommunicationPacket> processingResult = processPacket(portType, packet);
            if (!processingResult.has_value())
            {
                LOG_CLASS_INFO("No response packet generated for received packet with ID %" PRId32 ". Continuing...",
                               packet.packetId);
                continue;
            }
            acousea_CommunicationPacket& optResponsePacket = processingResult.value();
            optResponsePacket.packetId = packet.packetId;
            sendResponsePacket(portType, localAddress, optResponsePacket);
        }
    }
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::processPacket(IPort::PortType portType,
                                                                              const acousea_CommunicationPacket&
                                                                              packet)
{
    if (!packet.has_routing)
    {
        LOG_CLASS_ERROR("Packet without routing. Dropping packet...");
        // Puedes devolver un error, o bien construir una respuesta de error.
        return std::nullopt;
    }

    LOG_CLASS_INFO("Received Packet from %" PRId32 " with BODY = %s and PAYLOAD = %s",
                   packet.routing.sender,
                   bodyTagToCString(packet.which_body),
                   commandPayloadTagToCString(packet.body.command.which_command)
    );

    switch (packet.which_body)
    {
    case acousea_CommunicationPacket_command_tag:
        {
            const auto it = commandRoutines.find(packet.body.command.which_command);
            if (it == commandRoutines.end() || it->second == nullptr)
            {
                LOG_CLASS_ERROR("Routine with tag %d not found. Building error packet...",
                                packet.body.command.which_command);
                return buildErrorPacket("Routine not found.", packet.routing.sender);
            }

            std::optional<acousea_CommunicationPacket> optResponsePacket = executeRoutine(
                it->second, packet, portType, /*shouldRespond*/ true);

            return optResponsePacket;
        }

    case acousea_CommunicationPacket_response_tag:
        {
            const auto it = responseRoutines.find(packet.body.response.which_response);
            if (it == responseRoutines.end() || it->second == nullptr)
            {
                LOG_CLASS_ERROR("Routine with tag %d not found. Building error packet...",
                                packet.body.response.which_response);
                return buildErrorPacket("Routine not found.", packet.routing.sender);
            }
            std::optional<acousea_CommunicationPacket> optResponsePacket = executeRoutine(
                it->second, packet, portType, /*shouldRespond*/ true
            );
            return optResponsePacket;
        }

    case acousea_CommunicationPacket_report_tag:
        LOG_CLASS_INFO("Report packet received, but reports are not processed by nodes. Dropping packet...");
        return std::nullopt;

    case acousea_CommunicationPacket_error_tag:
        LOG_CLASS_ERROR("Received Error Packet from %" PRId32 " with error: %s",
                        packet.routing.sender,
                        packet.body.error.errorMessage);
        return std::nullopt;

    default:
        {
            LOG_CLASS_ERROR("Unknown packet body type. Building error packet...");
            return buildErrorPacket("Packet without payload.", packet.routing.sender); // devolver al origen
        }
    }
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::executeRoutine(
    IRoutine<acousea_CommunicationPacket>*& routine,
    const std::optional<acousea_CommunicationPacket>& optPacket,
    const IPort::PortType portType,
    const bool requeueAllowed
)
{
    Result<acousea_CommunicationPacket> result = routine->execute(optPacket);

    if (result.isPending() && !requeueAllowed)
    {
        LOG_CLASS_WARNING("%s[NO REQUEUE ALLOWED] => incomplete with message: %s", routine->routineName.c_str(),
                          result.getError());

        return std::nullopt;
    }

    if (result.isPending() && requeueAllowed)
    {
        pendingRoutines.add({routine, optPacket, 3, portType});
        LOG_CLASS_WARNING("%s [REQUEUED] => incomplete with message: %s", routine->routineName.c_str(),
                          result.getError());
        return std::nullopt;
    }

    if (result.isError())
    {
        LOG_CLASS_ERROR("%s => failed with message: %s", routine->routineName.c_str(), result.getError());
        if (!optPacket.has_value())
        {
            LOG_CLASS_INFO("%s: Cannot send error packet, there was no original packet.", routine->routineName.c_str());
            return std::nullopt;
        }
        const auto& packet = *optPacket;
        const uint8_t destination = packet.has_routing ? packet.routing.sender : Router::originAddress;
        const acousea_CommunicationPacket errorPkt = buildErrorPacket(result.getError(), destination);
        return errorPkt;
    }

    LOG_CLASS_INFO("%s => succeeded.", routine->routineName.c_str());
    return result.getValue();
}


void NodeOperationRunner::runPendingRoutines()
{
    // First run incomplete routines
    while (auto entryOpt = pendingRoutines.next())
    {
        auto& [routinePtr, inputPacket, attempts, portResponseTo] = *entryOpt;
        LOG_CLASS_INFO("Re-attempting pending routine %s with %d attempts left.", routinePtr->routineName.c_str(),
                       attempts);
        executeRoutine(routinePtr, inputPacket, portResponseTo, false);
    }
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             acousea_CommunicationPacket& responsePacket) const
{
    LOG_CLASS_INFO("Sending response packet with Body %s to %d through %s",
                   bodyTagToCString(responsePacket.which_body),
                   localAddress,
                   IPort::portTypeToCString(portType)
    );
    if (const auto sendOk = router.from(localAddress).through(portType).send(responsePacket); !sendOk)
    {
        LOG_CLASS_ERROR(": Failed to send response packet through %s", IPort::portTypeToCString(portType));
    }
}


acousea_CommunicationPacket NodeOperationRunner::buildErrorPacket(const std::string& errorMessage,
                                                                  const uint8_t& destination)
{
    acousea_ErrorBody errorBody;
    strncpy(errorBody.errorMessage, errorMessage.c_str(), sizeof(errorBody.errorMessage) - 1);
    errorBody.errorMessage[sizeof(errorBody.errorMessage) - 1] = '\0'; // Ensure null termination


    acousea_RoutingChunk routing = acousea_RoutingChunk_init_default;
    routing.receiver = destination;

    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;
    packet.has_routing = true;
    packet.routing = routing;
    packet.which_body = acousea_CommunicationPacket_error_tag;
    packet.body.error = errorBody;

    return packet;
}


//---------------------------------------------- Helpers ------------------------------------------------//
bool NodeOperationRunner::mustReport(const unsigned long currentMinute,
                                     const unsigned long reportingPeriod,
                                     const unsigned long lastReportMinute)
{
    if (reportingPeriod == 0) return false; // No reporting if period is 0 (disabled)
    if (lastReportMinute == ULONG_MAX) return true; // Always report if never reported before
    if (currentMinute == lastReportMinute) return false; // Avoids issues with duplicated reporting for same minute
    return ((currentMinute - lastReportMinute) >= reportingPeriod);
}

Result<acousea_OperationMode> NodeOperationRunner::searchForOperationMode(const uint8_t modeId) const
{
    if (!currentNodeConfiguration.has_value())
    {
        return RESULT_FAILUREF(acousea_OperationMode, "Node configuration not loaded.");
    }

    if (!currentNodeConfiguration->has_operationModesModule || currentNodeConfiguration->operationModesModule.
        modes_count == 0)
    {
        return RESULT_FAILUREF(acousea_OperationMode, "No operation modes module in configuration.");
    }

    for (int i = 0; i < currentNodeConfiguration->operationModesModule.modes_count; ++i)
    {
        if (currentNodeConfiguration->operationModesModule.modes[i].id == modeId)
            return RESULT_SUCCESS(acousea_OperationMode, currentNodeConfiguration->operationModesModule.modes[i]);
    }

    LOG_CLASS_ERROR("Operation mode %d not found in configuration.", modeId);
    return RESULT_FAILUREF(acousea_OperationMode, "Operation mode %d not found in configuration.", modeId);
}

Result<acousea_ReportingPeriodEntry> NodeOperationRunner::getReportingEntryForCurrentOperationMode(
    const uint8_t modeId, const IPort::PortType portType)
{
    if (!currentNodeConfiguration.has_value())
    {
        return RESULT_FAILUREF(acousea_ReportingPeriodEntry, "Node configuration not loaded.");
    }
    acousea_ReportingPeriodEntry* entries = nullptr;
    size_t entryCount = 0;
    switch (portType)
    {
    case IPort::PortType::LoraPort:
        {
            if (!currentNodeConfiguration->has_loraModule || currentNodeConfiguration->loraModule.entries_count == 0)
            {
                return RESULT_FAILUREF(acousea_ReportingPeriodEntry,
                                       "No LoRa reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration->loraModule.entries;
            entryCount = currentNodeConfiguration->loraModule.entries_count;
            break;
        }
    case IPort::PortType::SBDPort:
        {
            if (!currentNodeConfiguration->has_iridiumModule ||
                currentNodeConfiguration->iridiumModule.entries_count == 0)
            {
                return RESULT_FAILUREF(acousea_ReportingPeriodEntry,
                                       "No Iridium reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration->iridiumModule.entries;
            entryCount = currentNodeConfiguration->iridiumModule.entries_count;
            break;
        }
    case IPort::PortType::GsmMqttPort:
        {
            if (!currentNodeConfiguration->has_gsmMqttModule || currentNodeConfiguration->gsmMqttModule.entries_count ==
                0)
            {
                return RESULT_FAILUREF(acousea_ReportingPeriodEntry,
                                       "No GSM-MQTT reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration->gsmMqttModule.entries;
            entryCount = currentNodeConfiguration->gsmMqttModule.entries_count;
            break;
        }
    default:
        return RESULT_FAILUREF(acousea_ReportingPeriodEntry, "Unsupported port type for reporting entry.");
    }

    for (size_t i = 0; i < entryCount; ++i)
    {
        if (entries[i].modeId == modeId) return RESULT_SUCCESS(acousea_ReportingPeriodEntry, entries[i]);
    }
    return RESULT_FAILUREF(acousea_ReportingPeriodEntry,
                           "::getReportingEntryForCurrentOperationMode() Reporting entry for mode %d not found.",
                           modeId
    );
}
