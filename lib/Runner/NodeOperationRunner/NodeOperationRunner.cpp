#include "NodeOperationRunner.h"

inline std::string bodyTagToString(const uint8_t tag)
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

inline std::string commandPayloadTagToString(const uint8_t tag)

{
    switch (tag)
    {
    case acousea_CommandBody_setConfiguration_tag: return "Command->SetConfiguration";
    case acousea_CommandBody_requestedConfiguration_tag: return "Command->RequestedConfiguration";
    default: return "Command->Unknown";
    }
}

inline std::string responsePayloadTagToString(const uint8_t tag)
{
    switch (tag)
    {
    case acousea_ResponseBody_setConfiguration_tag: return "Response->SetConfiguration";
    case acousea_ResponseBody_updatedConfiguration_tag: return "Response->UpdatedConfiguration";
    default: return "Response->Unknown";
    }
}

inline std::string reportPayloadTagToString(const uint8_t tag)
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
): IRunnable(),
   router(router),
   commandRoutines(commandRoutines),
   responseRoutines(responseRoutines),
   reportRoutines(reportRoutines),
   nodeConfigurationRepository(nodeConfigurationRepository)
{
    cache = {
        acousea_OperationModesGraphModule_GraphEntry_init_default,
        0,
        {ULONG_MAX, ULONG_MAX}
    };
}

void NodeOperationRunner::init()
{
    Logger::logInfo(
        "[Init] Operation Cycle for Operation Mode=" + std::to_string(cache.currentOperationMode.key) +
        " with configuration: "
    );
    currentNodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfigurationRepository.printNodeConfiguration(*currentNodeConfiguration);
    if (!currentNodeConfiguration->has_operationGraphModule ||
        currentNodeConfiguration->operationGraphModule.graph_count == 0)
    {
        ErrorHandler::handleError(getClassNameString() + "No operation graph defined in configuration.");
        return;
    }
    const auto activeOpModeIdx = currentNodeConfiguration->operationModesModule.activeOperationModeIdx;
    if (activeOpModeIdx >= currentNodeConfiguration->operationGraphModule.graph_count)
    {
        Logger::logError(
            getClassNameString() + "Active operation mode index out of bounds. Defaulting to first mode.");
        cache.currentOperationMode = currentNodeConfiguration->operationGraphModule.graph[0];
        return;
    }
    cache.currentOperationMode = currentNodeConfiguration->operationGraphModule.graph[activeOpModeIdx];
    Logger::logInfo("[Init] Starting in Operation Mode=" + std::to_string(cache.currentOperationMode.key));
}

void NodeOperationRunner::run()
{
    Logger::logInfo("[Run] Operation Cycle for Operation mode=" + std::to_string(cache.currentOperationMode.key));
    checkIfMustTransition();
    processIncomingPackets(currentNodeConfiguration->localAddress);
    runPendingRoutines();
    processReportingRoutines();
    cache.cycleCount++;
    Logger::logInfo("[Finish] Operation Cycle for Operation mode=" + std::to_string(cache.currentOperationMode.key));
}


void NodeOperationRunner::tryReport(const std::string& moduleType,
                                    const acousea_ReportingPeriodEntry* entries,
                                    const size_t entryCount,
                                    IPort::PortType port,
                                    unsigned long& lastMinute,
                                    unsigned long currentMinute)
{
    auto cfg = searchForReportingEntry(cache.currentOperationMode.key, entries, entryCount);
    if (cfg.isError())
    {
        Logger::logError(moduleType + " reporting entry not found: " + cfg.getError());
        return;
    }
    auto [modeId, period] = cfg.getValue();

    Logger::logInfo(moduleType + " Config: { Period=" + std::to_string(period) +
        ", Current minute=" + std::to_string(currentMinute) +
        ", Last report minute=" + std::to_string(lastMinute) + " }");

    if (mustReport(currentMinute, period, lastMinute))
    {
        const auto it = reportRoutines.find(acousea_ReportBody_statusPayload_tag);
        if (it == reportRoutines.end() || it->second == nullptr)
        {
            Logger::logError("Report routine with tag " + std::to_string(modeId) +
                " not found. Skipping report...");
            return;
        }
        auto result = executeRoutine(it->second, std::nullopt, port, false);
        if (result.has_value())
        {
            sendResponsePacket(port, currentNodeConfiguration->localAddress, *result);
        }
        lastMinute = currentMinute;
    }
}


void NodeOperationRunner::checkIfMustTransition()
{
    if (const auto maxDuration = cache.currentOperationMode.value.duration; cache.cycleCount >= maxDuration)
    {
        cache.cycleCount = 0;
        const auto nextOpModeResult = searchForOperationMode(cache.currentOperationMode.value.targetMode);
        if (nextOpModeResult.isError())
        {
            ErrorHandler::handleError(getClassNameString() +
                ": Next operation mode not found " +
                nextOpModeResult.getError()
            );
            return;
        }
        cache.currentOperationMode = nextOpModeResult.getValueConst();
        Logger::logInfo("Transitioned to next mode..." + std::to_string(cache.currentOperationMode.key));
    }
}

void NodeOperationRunner::processReportingRoutines()
{
    const auto currentMinute = getMillis() / 60000;

    if (!currentNodeConfiguration.has_value())
    {
        ErrorHandler::handleError(getClassNameString() + ": Node configuration not loaded.");
        return;
    }
    if (currentNodeConfiguration->has_loraModule)
    {
        tryReport(
            "Lora",
            currentNodeConfiguration->loraModule.entries,
            currentNodeConfiguration->loraModule.entries_count,
            IPort::PortType::LoraPort,
            cache.lastReportMinute.lora, currentMinute
        );
    }
    else
        Logger::logInfo(getClassNameString() + "LoRa module not present, skipping LoRa report.");

    currentNodeConfiguration->has_iridiumModule
        ? tryReport(
            "Iridium",
            currentNodeConfiguration->iridiumModule.entries,
            currentNodeConfiguration->iridiumModule.entries_count,
            IPort::PortType::SBDPort,
            cache.lastReportMinute.sbd, currentMinute
        )
        : Logger::logInfo(getClassNameString() + "Iridium module not present, skipping Iridium report.");
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
                Logger::logInfo("No response packet generated for received packet with ID " +
                    std::to_string(packet.packetId) + ". Continuing...");
                continue;
            }
            acousea_CommunicationPacket& optResponsePacket = processingResult.value();
            optResponsePacket.packetId = packet.packetId;
            sendResponsePacket(portType, localAddress, optResponsePacket);
        }
    }
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::processPacket(IPort::PortType portType,
                                                                              const acousea_CommunicationPacket& packet)
{
    if (!packet.has_routing)
    {
        Logger::logError(getClassNameString() + ": Packet without routing. Dropping packet...");
        // Puedes devolver un error, o bien construir una respuesta de error.
        return std::nullopt;
    }

    Logger::logInfo("Received Packet from " + std::to_string(packet.routing.sender) +
        " with BODY = " + bodyTagToString(packet.which_body) +
        " and PAYLOAD = : " + commandPayloadTagToString(packet.body.command.which_command));

    switch (packet.which_body)
    {
    case acousea_CommunicationPacket_command_tag:
        {
            const auto it = commandRoutines.find(packet.body.command.which_command);
            if (it == commandRoutines.end() || it->second == nullptr)
            {
                Logger::logError("Routine with tag " + std::to_string(packet.body.command.which_command) +
                    " not found. Building error packet...");
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
                Logger::logError("Routine with tag " + std::to_string(packet.body.response.which_response) +
                    " not found. Building error packet...");
                return buildErrorPacket("Routine not found.", packet.routing.sender);
            }
            std::optional<acousea_CommunicationPacket> optResponsePacket = executeRoutine(
                it->second, packet, portType, /*shouldRespond*/ true);
            return optResponsePacket;
        }

    case acousea_CommunicationPacket_report_tag:
        Logger::logInfo("Report packet received, but reports are not processed by nodes. Dropping packet...");
        return std::nullopt;

    case acousea_CommunicationPacket_error_tag:
        Logger::logError("Received Error Packet from " + std::to_string(packet.routing.sender) +
            " with error: " + std::string(packet.body.error.errorMessage));
        return std::nullopt;

    default:
        {
            Logger::logError(getClassNameString() + ": Unknown packet body type. Building error packet...");
            return buildErrorPacket("Packet without payload.",
                                    packet.routing.sender); // devolver al origen
        }
    }
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::executeRoutine(
    IRoutine<_acousea_CommunicationPacket>*& routine,
    const std::optional<acousea_CommunicationPacket>& optPacket,
    const IPort::PortType portType,
    const bool requeueAllowed
)
{
    Result<acousea_CommunicationPacket> result = routine->execute(optPacket);

    if (result.isPending() && !requeueAllowed)
    {
        Logger::logWarning(routine->routineName + " incomplete, but requeue not allowed.");
        return std::nullopt;
    }

    if (result.isPending() && requeueAllowed)
    {
        pendingRoutines.add({routine, optPacket, 3, portType});
        Logger::logWarning(routine->routineName + " incomplete, requeued");
        return std::nullopt;
    }

    if (result.isError())
    {
        Logger::logError(routine->routineName + " failed: " + result.getError());
        if (!optPacket.has_value())
        {
            Logger::logError(routine->routineName + ": Cannot send error packet, there was no original packet.");
            return std::nullopt;
        }
        const auto& packet = *optPacket;
        const uint8_t destination = packet.has_routing ? packet.routing.sender : Router::originAddress;
        const acousea_CommunicationPacket errorPkt = buildErrorPacket(result.getError(), destination);
        return errorPkt;
    }

    Logger::logInfo(routine->routineName + " executed successfully.");
    return result.getValue();
}


void NodeOperationRunner::runPendingRoutines()
{
    // First run incomplete routines
    while (auto entryOpt = pendingRoutines.next())
    {
        auto& [routinePtr, inputPacket, attempts, portResponseTo] = *entryOpt;
        Logger::logInfo("Re-attempting pending routine " + routinePtr->routineName +
            " with " + std::to_string(attempts) + " attempts left."
        );
        executeRoutine(routinePtr, inputPacket, portResponseTo, false);
    }
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             acousea_CommunicationPacket& responsePacket) const
{
    Logger::logInfo(
        "Sending response packet with Body " + bodyTagToString(responsePacket.which_body) +
        " to " + std::to_string(localAddress) +
        " through " + IPort::portTypeToString(portType)
    );
    switch (portType)
    {
    case IPort::PortType::SBDPort:
        router.sendFrom(localAddress).sendSBD(responsePacket);
        break;
    case IPort::PortType::LoraPort:
        router.sendFrom(localAddress).sendLoRa(responsePacket);
        break;
    case IPort::PortType::SerialPort:
        router.sendFrom(localAddress).sendSerial(responsePacket);
        break;
    default:
        Logger::logError(getClassNameString() + ": Unknown port type for response packet!");
        break;
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

Result<acousea_OperationModesGraphModule_GraphEntry> NodeOperationRunner::searchForOperationMode(
    const uint8_t modeId) const
{
    for (int i = 0; i < currentNodeConfiguration->operationGraphModule.graph_count; ++i)
    {
        if (currentNodeConfiguration->operationGraphModule.graph[i].key == modeId)
            return Result<acousea_OperationModesGraphModule_GraphEntry>::success(
                currentNodeConfiguration->operationGraphModule.graph[i]
            );
    }

    Logger::logError("Operation mode " + std::to_string(modeId) + " not found. Returning default mode.");
    return Result<acousea_OperationModesGraphModule_GraphEntry>::failure(
        "Operation mode " + std::to_string(modeId) + " not found."
    );
}

Result<acousea_ReportingPeriodEntry> NodeOperationRunner::searchForReportingEntry(
    const uint8_t modeId, const acousea_ReportingPeriodEntry* entries, const size_t entryCount)
{
    for (size_t i = 0; i < entryCount; ++i)
    {
        if (entries[i].modeId == modeId) return Result<acousea_ReportingPeriodEntry>::success(entries[i]);
    }
    Logger::logError("Reporting entry for mode " + std::to_string(modeId) + " not found. Returning default entry.");
    return Result<acousea_ReportingPeriodEntry>::failure(
        "Reporting entry for mode " + std::to_string(modeId) + " not found."
    );
}
