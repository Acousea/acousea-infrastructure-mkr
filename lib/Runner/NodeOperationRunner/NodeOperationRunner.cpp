#include "NodeOperationRunner.h"

inline std::string payloadTagToString(const uint8_t tag)
{
    switch (tag)
    {
    case acousea_PayloadWrapper_statusPayload_tag: return "StatusReportPayload";
    case acousea_PayloadWrapper_setConfiguration_tag: return "SetNodeConfigurationPayload";
    case acousea_PayloadWrapper_requestedConfiguration_tag: return "GetUpdatedNodeConfigurationPayload";
    case acousea_PayloadWrapper_errorPayload_tag: return "ErrorPayload";
    default: return "UnknownPayload";
    }
}

NodeOperationRunner::NodeOperationRunner(Router& router,
                                         const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>&
                                         routines,
                                         const NodeConfigurationRepository& nodeConfigurationRepository)
    : IRunnable(),
      router(router),
      routines(routines),
      nodeConfigurationRepository(nodeConfigurationRepository)
{
    cache = {0, 0, {0, 0}};
}

void NodeOperationRunner::init()
{
    Logger::logInfo(
        "[Init] Operation Cycle for Operation Mode=" + std::to_string(cache.currentOperationMode.key) +
        " with configuration: "
    );
    currentNodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfigurationRepository.printNodeConfiguration(*currentNodeConfiguration);
    if (!currentNodeConfiguration->has_operationGraphModule || currentNodeConfiguration->operationGraphModule.
        graph_count == 0)
    {
        ErrorHandler::handleError(getClassNameString() + ": No operation graph defined in configuration.");
        return;
    }
    cache.currentOperationMode = currentNodeConfiguration->operationGraphModule.graph[0];
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



void NodeOperationRunner::tryReport(
    const std::string& moduleType,
    const acousea_ReportingPeriodEntry* entries, const size_t entryCount,
    IPort::PortType port, unsigned long& lastMinute, unsigned long currentMinute)
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
        runReportRoutine(port, currentMinute, lastMinute);
    }
}

void NodeOperationRunner::executeRoutine(const uint8_t routineTag,
                                         const std::optional<acousea_CommunicationPacket>& inputPacket,
                                         IPort::PortType port, const uint8_t destination,
                                         const bool requeueAllowed)
{
    const auto it = routines.find(routineTag);
    if (it == routines.end() || it->second == nullptr)
    {
        ErrorHandler::handleError(getClassNameString() + ": StatusReport routine not found.");
        return;
    }
    IRoutine<acousea_CommunicationPacket>* routine = it->second;
    const auto res = routine->execute(inputPacket);

    if (res.isPending())
    {
        if (!requeueAllowed)
        {
            Logger::logInfo(routine->routineName + " returned pending but requeue not allowed.");
            return;
        }
        pendingRoutines.add({routineTag, inputPacket, 1, port});
        Logger::logInfo(routine->routineName + " incomplete, requeued");
        return;
    }

    if (res.isError())
    {
        Logger::logError(routine->routineName + " failed: " + res.getError());
        const auto errorPkt = buildErrorPacket(res.getError(), currentNodeConfiguration->localAddress, destination);
        sendResponsePacket(port, currentNodeConfiguration->localAddress, errorPkt);
        return;
    }

    sendResponsePacket(port, currentNodeConfiguration->localAddress, res.getValue());
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
        cache.currentOperationMode = nextOpModeResult.getValue();
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
    currentNodeConfiguration->has_loraModule
        ? tryReport(
            "Lora",
            currentNodeConfiguration->loraModule.entries,
            currentNodeConfiguration->loraModule.entries_count,
            IPort::PortType::LoraPort,
            cache.lastReportMinute.lora, currentMinute
        )
        : Logger::logInfo(getClassNameString() + "LoRa module not present, skipping LoRa report.");

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
            processPacket(portType, packet, localAddress);
        }
    }
}

void NodeOperationRunner::processPacket(IPort::PortType portType,
                                        const acousea_CommunicationPacket& packet,
                                        const uint8_t& localAddress)
{
    if (!packet.has_routing)
    {
        Logger::logError(getClassNameString() + ": Packet without routing. Dropping packet...");
        return;
    }

    if (!packet.has_payload)
    {
        ErrorHandler::handleError(getClassNameString() + ": Packet without payload, replying with ERROR_PACKET.");
        const auto errorPkt = buildErrorPacket("Packet without payload.", localAddress, packet.routing.sender);
        sendResponsePacket(portType, localAddress, errorPkt);
        return;
    }

    const uint8_t payloadTypeTag = packet.payload.which_payload;
    const uint8_t sender = packet.routing.sender;
    Logger::logInfo(
        "Processing packet " + payloadTagToString(payloadTypeTag) +
        " from " + std::to_string(sender) +
        " received through " + IPort::portTypeToString(portType)
    );
    executeRoutine(payloadTypeTag, packet, portType, sender, true);
}

void NodeOperationRunner::runPendingRoutines()
{
    // First run incomplete routines
    while (auto entryOpt = pendingRoutines.next())
    {
        auto& [routineTag, inputPacket, attempts, portResponseTo] = *entryOpt;
        Logger::logInfo("Re-attempting pending routine with tag " + payloadTagToString(routineTag) +
            ", attempt " + std::to_string(attempts)
        );
        executeRoutine(routineTag, inputPacket, portResponseTo, currentNodeConfiguration->localAddress, false);
    }
}

void NodeOperationRunner::runReportRoutine(IPort::PortType portType, unsigned long currentMinute,
                                           unsigned long& lastReportMinute)
{
    executeRoutine(acousea_PayloadWrapper_statusPayload_tag,
                   std::nullopt, portType,
                   Router::originAddress,
                   false);
    lastReportMinute = currentMinute;
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             const acousea_CommunicationPacket& responsePacket) const
{
    Logger::logInfo(
        "Sending response packet with Payload " + payloadTagToString(responsePacket.payload.which_payload) +
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
                                                                  const uint8_t& localAddress,
                                                                  const uint8_t& destination)
{
    acousea_ErrorPayload errorPayload;
    strncpy(errorPayload.errorMessage, errorMessage.c_str(), sizeof(errorPayload.errorMessage) - 1);
    errorPayload.errorMessage[sizeof(errorPayload.errorMessage) - 1] = '\0'; // Ensure null termination

    acousea_PayloadWrapper payloadWrapper = acousea_PayloadWrapper_init_default;
    payloadWrapper.which_payload = acousea_PayloadWrapper_errorPayload_tag;
    payloadWrapper.payload.errorPayload = errorPayload;

    acousea_RoutingChunk routing = acousea_RoutingChunk_init_default;
    routing.sender = localAddress;
    routing.receiver = destination;

    acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;
    packet.has_payload = true;
    packet.payload = payloadWrapper;
    packet.has_routing = true;
    packet.routing = routing;

    return packet;
}


//---------------------------------------------- Helpers ------------------------------------------------//
bool NodeOperationRunner::mustReport(const unsigned long currentMinute,
                                     const unsigned long reportingPeriod,
                                     const unsigned long lastReportMinute)
{
    return (currentMinute - lastReportMinute >= reportingPeriod && reportingPeriod != 0)
        || lastReportMinute == 0;
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
