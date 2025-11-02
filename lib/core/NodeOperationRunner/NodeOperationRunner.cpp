#include "NodeOperationRunner.h"

#include <climits> // for  ULONG_MAX

inline std::string bodyTagToString(const uint8_t tag){
    switch (tag){
    case acousea_CommunicationPacket_command_tag: return "Command";
    case acousea_CommunicationPacket_response_tag: return "Response";
    case acousea_CommunicationPacket_report_tag: return "Report";
    case acousea_CommunicationPacket_error_tag: return "Error";
    default: return "UnknownBody";
    }
}

inline std::string commandPayloadTagToString(const uint8_t tag){
    switch (tag){
    case acousea_CommandBody_setConfiguration_tag: return "Command->SetConfiguration";
    case acousea_CommandBody_requestedConfiguration_tag: return "Command->RequestedConfiguration";
    default: return "Command->Unknown";
    }
}

inline std::string responsePayloadTagToString(const uint8_t tag){
    switch (tag){
    case acousea_ResponseBody_setConfiguration_tag: return "Response->SetConfiguration";
    case acousea_ResponseBody_updatedConfiguration_tag: return "Response->UpdatedConfiguration";
    default: return "Response->Unknown";
    }
}

inline std::string reportPayloadTagToString(const uint8_t tag){
    switch (tag){
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
   nodeConfigurationRepository(nodeConfigurationRepository){
    cache = {
        acousea_OperationMode_init_default,
        0,
        {ULONG_MAX, ULONG_MAX}
    };
}

void NodeOperationRunner::init(){
    Logger::logInfo(getClassNameString() +
        std::string("<Init> Operation Cycle for Operation Mode ") +
        std::to_string(cache.currentOperationMode.id) + "=" + std::string(cache.currentOperationMode.name) +
        " with configuration: "
    );

    currentNodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfigurationRepository.printNodeConfiguration(*currentNodeConfiguration);
    // Search for the operation mode with the given ID
    const auto opModeResult = searchForOperationMode(currentNodeConfiguration->operationModesModule.activeModeId);
    if (opModeResult.isError()){
        ErrorHandler::handleError(
            getClassNameString() + ": Initial operation mode not found: " + opModeResult.getError()
        );
        return;
    }

    cache.currentOperationMode = opModeResult.getValueConst();
}

void NodeOperationRunner::run(){
    Logger::logInfo(getClassNameString() + "<Run> Operation Cycle for Operation mode " +
        std::to_string(cache.currentOperationMode.id) + "=" + " (" + std::string(cache.currentOperationMode.name) + ")"
    );
    checkIfMustTransition();
    processIncomingPackets(currentNodeConfiguration->localAddress);
    runPendingRoutines();
    processReportingRoutines();
    cache.cycleCount++;
    Logger::logInfo(getClassNameString() + "<Finish> Operation Cycle for Operation mode " +
        std::to_string(cache.currentOperationMode.id) + "=" + " (" + std::string(cache.currentOperationMode.name) + ")"
    );
}


void NodeOperationRunner::checkIfMustTransition(){
    if (!cache.currentOperationMode.has_transition){
        ErrorHandler::handleError(
            getClassNameString() + ": Current operation mode has no transition defined. Resetting ..."
        );
        return;
    }
    if (const auto maxDuration = cache.currentOperationMode.transition.duration; cache.cycleCount >= maxDuration){
        cache.cycleCount = 0;
        const auto nextOpModeResult = searchForOperationMode(cache.currentOperationMode.transition.targetModeId);
        if (nextOpModeResult.isError()){
            ErrorHandler::handleError(getClassNameString() +
                ": Next operation mode not found " +
                nextOpModeResult.getError()
            );
            return;
        }
        cache.currentOperationMode = nextOpModeResult.getValueConst();
        Logger::logInfo(getClassNameString() + "Transitioned to next mode..." +
            std::to_string(cache.currentOperationMode.id) + "=" + " (" + std::string(cache.currentOperationMode.name) +
            ")"
        );
    }
}

void NodeOperationRunner::tryReport(const IPort::PortType portType,
                                    unsigned long& lastMinute,
                                    const unsigned long currentMinute){
    const auto cfg = getReportingEntryForCurrentOperationMode(
        cache.currentOperationMode.id, portType
    );

    if (cfg.isError()){
        Logger::logError(cfg.getError());
        return;
    }

    auto [modeId, period] = cfg.getValueConst();

    Logger::logInfo(
        getClassNameString() + IPort::portTypeToString(portType) + " Config: { Period=" + std::to_string(period) +
        ", Current minute=" + std::to_string(currentMinute) +
        ", Last report minute=" + std::to_string(lastMinute) + " }");

    if (!mustReport(currentMinute, period, lastMinute)){
        Logger::logInfo(
            getClassNameString() + IPort::portTypeToString(portType) + " Not time to report yet. Skipping report...");
        return;
    }

    const auto it = reportRoutines.find(acousea_ReportBody_statusPayload_tag);
    if (it == reportRoutines.end() || it->second == nullptr){
        Logger::logError(getClassNameString() + "Report routine with tag " + std::to_string(modeId) +
            " not found. Skipping report...");
        return;
    }

    auto result = executeRoutine(it->second, std::nullopt, portType, false);
    if (result.has_value()){
        sendResponsePacket(portType, currentNodeConfiguration->localAddress, *result);
    }

    lastMinute = currentMinute;
}


void NodeOperationRunner::processReportingRoutines(){
    const auto currentMinute = getMillis() / 60000;

    if (!currentNodeConfiguration.has_value()){
        ErrorHandler::handleError(getClassNameString() + ": Node configuration not loaded.");
        return;
    }
    currentNodeConfiguration->has_loraModule
        ? tryReport(IPort::PortType::LoraPort, cache.lastReportMinute.lora, currentMinute)
        : Logger::logInfo(getClassNameString() + "LoRa module not present, skipping LoRa report.");

    currentNodeConfiguration->has_iridiumModule
        ? tryReport(IPort::PortType::SBDPort, cache.lastReportMinute.sbd, currentMinute)
        : Logger::logInfo(getClassNameString() + "Iridium module not present, skipping Iridium report.");

    currentNodeConfiguration->has_gsmMqttModule
        ? tryReport(IPort::PortType::GsmMqttPort, cache.lastReportMinute.gsmMqtt, currentMinute)
        : Logger::logInfo(getClassNameString() + "GSM-MQTT module not present, skipping GSM-MQTT report.");
}

void NodeOperationRunner::processIncomingPackets(const uint8_t& localAddress){
    auto receivedPackets = router.readPorts(localAddress);
    for (const auto& [portType, packets] : receivedPackets){
        for (auto& packet : packets){
            std::optional<acousea_CommunicationPacket> processingResult = processPacket(portType, packet);
            if (!processingResult.has_value()){
                Logger::logInfo(getClassNameString() + "No response packet generated for received packet with ID " +
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
                                                                              const acousea_CommunicationPacket&
                                                                              packet){
    if (!packet.has_routing){
        Logger::logError(getClassNameString() + ": Packet without routing. Dropping packet...");
        // Puedes devolver un error, o bien construir una respuesta de error.
        return std::nullopt;
    }

    Logger::logInfo(getClassNameString() + "Received Packet from " + std::to_string(packet.routing.sender) +
        " with BODY = " + bodyTagToString(packet.which_body) +
        " and PAYLOAD = " + commandPayloadTagToString(packet.body.command.which_command));

    switch (packet.which_body){
    case acousea_CommunicationPacket_command_tag: {
        const auto it = commandRoutines.find(packet.body.command.which_command);
        if (it == commandRoutines.end() || it->second == nullptr){
            Logger::logError(
                getClassNameString() + "Routine with tag " + std::to_string(packet.body.command.which_command) +
                " not found. Building error packet...");
            return buildErrorPacket("Routine not found.", packet.routing.sender);
        }

        std::optional<acousea_CommunicationPacket> optResponsePacket = executeRoutine(
            it->second, packet, portType, /*shouldRespond*/ true);

        return optResponsePacket;
    }

    case acousea_CommunicationPacket_response_tag: {
        const auto it = responseRoutines.find(packet.body.response.which_response);
        if (it == responseRoutines.end() || it->second == nullptr){
            Logger::logError(getClassNameString() +
                "Routine with tag " + std::to_string(packet.body.response.which_response) + " not found."
                " Building error packet..."
            );
            return buildErrorPacket("Routine not found.", packet.routing.sender);
        }
        std::optional<acousea_CommunicationPacket> optResponsePacket = executeRoutine(
            it->second, packet, portType, /*shouldRespond*/ true
        );
        return optResponsePacket;
    }

    case acousea_CommunicationPacket_report_tag:
        Logger::logInfo(
            getClassNameString() +
            "Report packet received, but reports are not processed by nodes. Dropping packet...");
        return std::nullopt;

    case acousea_CommunicationPacket_error_tag:
        Logger::logError(getClassNameString() + "Received Error Packet from " + std::to_string(packet.routing.sender) +
            " with error: " + std::string(packet.body.error.errorMessage));
        return std::nullopt;

    default: {
        Logger::logError(getClassNameString() + ": Unknown packet body type. Building error packet...");
        return buildErrorPacket("Packet without payload.",
                                packet.routing.sender); // devolver al origen
    }
    }
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::executeRoutine(
    IRoutine<acousea_CommunicationPacket>*& routine,
    const std::optional<acousea_CommunicationPacket>& optPacket,
    const IPort::PortType portType,
    const bool requeueAllowed
){
    Result<acousea_CommunicationPacket> result = routine->execute(optPacket);

    if (result.isPending() && !requeueAllowed){
        Logger::logWarning(routine->routineName + " incomplete, but requeue not allowed.");
        return std::nullopt;
    }

    if (result.isPending() && requeueAllowed){
        pendingRoutines.add({routine, optPacket, 3, portType});
        Logger::logWarning(routine->routineName + " incomplete, requeued");
        return std::nullopt;
    }

    if (result.isError()){
        Logger::logError(routine->routineName + " failed: " + result.getError());
        if (!optPacket.has_value()){
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


void NodeOperationRunner::runPendingRoutines(){
    // First run incomplete routines
    while (auto entryOpt = pendingRoutines.next()){
        auto& [routinePtr, inputPacket, attempts, portResponseTo] = *entryOpt;
        Logger::logInfo("Re-attempting pending routine " + routinePtr->routineName +
            " with " + std::to_string(attempts) + " attempts left."
        );
        executeRoutine(routinePtr, inputPacket, portResponseTo, false);
    }
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             acousea_CommunicationPacket& responsePacket) const{
    Logger::logInfo(
        "Sending response packet with Body " + bodyTagToString(responsePacket.which_body) +
        " to " + std::to_string(localAddress) +
        " through " + IPort::portTypeToString(portType)
    );
    switch (portType){
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
                                                                  const uint8_t& destination){
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
                                     const unsigned long lastReportMinute){
    if (reportingPeriod == 0) return false; // No reporting if period is 0 (disabled)
    if (lastReportMinute == ULONG_MAX) return true; // Always report if never reported before
    if (currentMinute == lastReportMinute) return false; // Avoids issues with duplicated reporting for same minute
    return ((currentMinute - lastReportMinute) >= reportingPeriod);
}

Result<acousea_OperationMode> NodeOperationRunner::searchForOperationMode(const uint8_t modeId) const{
    if (!currentNodeConfiguration.has_value()){
        return Result<acousea_OperationMode>::failure("Node configuration not loaded.");
    }

    if (!currentNodeConfiguration->has_operationModesModule || currentNodeConfiguration->operationModesModule.
        modes_count == 0){
        return Result<acousea_OperationMode>::failure("No operation graph defined in configuration.");
    }

    for (int i = 0; i < currentNodeConfiguration->operationModesModule.modes_count; ++i){
        if (currentNodeConfiguration->operationModesModule.modes[i].id == modeId)
            return Result<acousea_OperationMode>::success(
                currentNodeConfiguration->operationModesModule.modes[i]
            );
    }

    Logger::logError("Operation mode " + std::to_string(modeId) + " not found. Returning default mode.");
    return Result<acousea_OperationMode>::failure(
        "Operation mode " + std::to_string(modeId) + " not found."
    );
}

Result<acousea_ReportingPeriodEntry> NodeOperationRunner::getReportingEntryForCurrentOperationMode(
    const uint8_t modeId, const IPort::PortType portType){
    if (!currentNodeConfiguration.has_value()){
        return Result<acousea_ReportingPeriodEntry>::failure("Node configuration not loaded.");
    }
    acousea_ReportingPeriodEntry* entries = nullptr;
    size_t entryCount = 0;
    switch (portType){
    case IPort::PortType::LoraPort: {
        if (!currentNodeConfiguration->has_loraModule || currentNodeConfiguration->loraModule.entries_count == 0){
            return Result<acousea_ReportingPeriodEntry>::failure("No LoRa reporting entries defined in configuration.");
        }
        entries = currentNodeConfiguration->loraModule.entries;
        entryCount = currentNodeConfiguration->loraModule.entries_count;
        break;
    }
    case IPort::PortType::SBDPort: {
        if (!currentNodeConfiguration->has_iridiumModule || currentNodeConfiguration->iridiumModule.entries_count == 0){
            return Result<acousea_ReportingPeriodEntry>::failure(
                "No Iridium reporting entries defined in configuration.");
        }
        entries = currentNodeConfiguration->iridiumModule.entries;
        entryCount = currentNodeConfiguration->iridiumModule.entries_count;
        break;
    }
    case IPort::PortType::GsmMqttPort: {
        if (!currentNodeConfiguration->has_gsmMqttModule || currentNodeConfiguration->gsmMqttModule.entries_count == 0){
            return Result<acousea_ReportingPeriodEntry>::failure(
                "No GSM-MQTT reporting entries defined in configuration.");
        }
        entries = currentNodeConfiguration->gsmMqttModule.entries;
        entryCount = currentNodeConfiguration->gsmMqttModule.entries_count;
        break;
    }
    default:
        return Result<acousea_ReportingPeriodEntry>::failure("Unsupported port type for reporting entry.");
    }

    for (size_t i = 0; i < entryCount; ++i){
        if (entries[i].modeId == modeId) return Result<acousea_ReportingPeriodEntry>::success(entries[i]);
    }
    return Result<acousea_ReportingPeriodEntry>::failure(getClassNameString() +
        "::getReportingEntryForCurrentOperationMode() Reporting entry for mode " + std::to_string(modeId) +
        " not found.");
}
