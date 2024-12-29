#include "NodeOperationRunner.h"

NodeOperationRunner::NodeOperationRunner(Router &router,
                                         const std::map<OperationCode::Code, IRoutine<VoidType> *> &internalRoutines,
                                         const std::map<OperationCode::Code, IRoutine<Packet> *> &externalRoutines,
                                         const NodeConfigurationRepository &nodeConfigurationRepository)
    : IRunnable(),
      router(router),
      internalRoutines(internalRoutines),
      externalRoutines(externalRoutines),
      nodeConfigurationRepository(nodeConfigurationRepository) {
    cache = {0, 0, {0, 0}};
}

void NodeOperationRunner::init() {
    Logger::logInfo(
        "-> Init Operation Cycle for Operation Mode=" + std::to_string(cache.currentOperationMode) +
        " with configuration: "
    );
    nodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfiguration->print();
    cache.currentOperationMode = nodeConfiguration->getOperationGraphModule()->getGraph().begin()->first;
}

void NodeOperationRunner::run() {
    Logger::logInfo("-> Run Operation Cycle for Operation mode=" + std::to_string(cache.currentOperationMode));
    checkIfMustTransition();
    processIncomingPackets(nodeConfiguration->getLocalAddress());
    runRoutines();
}

void NodeOperationRunner::finish() {
    cache.cycleCount++;
    Logger::logInfo("-> Finish Operation Cycle for Operation mode=" + std::to_string(cache.currentOperationMode));
}

void NodeOperationRunner::checkIfMustTransition() {
    const auto maxDuration = nodeConfiguration->getOperationGraphModule()
            ->getGraph()
            .at(cache.currentOperationMode).duration;

    if (cache.cycleCount >= maxDuration) {
        cache.cycleCount = 0;
        const auto nextOpMode = nodeConfiguration->getOperationGraphModule()
                ->getGraph()
                .at(cache.currentOperationMode).nextMode;
        cache.currentOperationMode = nextOpMode;
        Logger::logInfo("Transitioned to next mode..." + std::to_string(cache.currentOperationMode));
    }
}

bool NodeOperationRunner::mustReport(const unsigned long currentMinute, const unsigned long reportingPeriod,
                                     const unsigned long lastReportMinute) {
    return (currentMinute - lastReportMinute >= reportingPeriod && reportingPeriod != 0)
           || lastReportMinute == 0;
}

void NodeOperationRunner::runRoutines() {
    const auto currentMinute = millis() / 60000;
    // Handle Iridium reporting
    if (nodeConfiguration->getIridiumModule().has_value()) {
        const auto &iridiumConfig = nodeConfiguration->getIridiumModule()
                ->getConfigurations()
                .at(cache.currentOperationMode);
        Logger::logInfo("Iridium Config: { Period=" + std::to_string(iridiumConfig.getPeriod()) +
                        ", Current minute=" + std::to_string(currentMinute) +
                        ", Last report minute=" + std::to_string(cache.lastReportMinute.sbd) + " }");

        if (mustReport(currentMinute, iridiumConfig.getPeriod(), cache.lastReportMinute.sbd)) {
            processRoutine(iridiumConfig, IPort::PortType::SBDPort, currentMinute, cache.lastReportMinute.sbd);
        }
    }

    // Handle LoRa reporting
    if (nodeConfiguration->getLoraModule().has_value()) {
        const auto &loraConfig = nodeConfiguration->getLoraModule()
                ->getConfigurations()
                .at(cache.currentOperationMode);

        Logger::logInfo("Lora Config: { Period=" + std::to_string(loraConfig.getPeriod()) +
                        ", Current minute=" + std::to_string(currentMinute) +
                        ", Last report minute=" + std::to_string(cache.lastReportMinute.lora) + " }");

        if (mustReport(currentMinute, loraConfig.getPeriod(), cache.lastReportMinute.lora)) {
            processRoutine(loraConfig, IPort::PortType::LoraPort, currentMinute, cache.lastReportMinute.lora);
        }
    }
}

void NodeOperationRunner::processRoutine(const ReportingConfiguration &config, IPort::PortType portType,
                                         unsigned long currentMinute, unsigned long &lastReportMinute) {
    IRoutine<VoidType> *routine = nullptr;

    Logger::logInfo("Executing routine for report type: " + config.getReportTypeString());
    switch (config.getReportType()) {
        case ReportingConfiguration::ReportType::COMPLETE:
            routine = internalRoutines.find(OperationCode::Code::COMPLETE_STATUS_REPORT)->second;
            break;
        case ReportingConfiguration::ReportType::BASIC:
            routine = internalRoutines.find(OperationCode::Code::BASIC_STATUS_REPORT)->second;
            break;
        default:
            ErrorHandler::handleError(getClassNameString() + ": Invalid report type");
            return;
    }

    if (!routine) {
        ErrorHandler::handleError(getClassNameString() + ": Routine not found for report type " + config.getReportTypeString());
        return;
    }

    const Result<Packet> result = routine->execute();

    if (result.isError()) {
        Logger::logError("Routine execution failed: " + result.getError());
        return;
    }

    sendResponsePacket(portType, nodeConfiguration->getLocalAddress(), result.getValue());
    lastReportMinute = currentMinute;
}

void NodeOperationRunner::processIncomingPackets(const Address &localAddress) {
    auto receivedPackets = router.readPorts(localAddress);
    for (const auto &[portType, packets]: receivedPackets) {
        for (auto &packet: packets) {
            processPacket(portType, packet, localAddress);
        }
    }
}

void NodeOperationRunner::processPacket(IPort::PortType portType, const Packet &packet, const Address &localAddress) {
    Logger::logInfo(
        "Processing packet " + packet.encode() +
        "from " + std::to_string(packet.getRoutingChunk().getSender().getValue()) +
        " received through " + IPort::portTypeToString(portType)
    );

    const OperationCode::Code opCode = packet.getOpCodeEnum();

    if (externalRoutines.find(opCode) == externalRoutines.end()) {
        Logger::logError(getClassNameString() + " Exception: No routine found for OpCode: " + std::to_string(opCode));
        sendResponsePacket(portType, localAddress, ErrorPacket::invalidOpcode(packet.getRoutingChunk()));
        return;
    }

    const Result<Packet> result = externalRoutines[opCode]->execute(packet);
    if (!result.isSuccess()) {
        Logger::logError(getClassNameString() + " Exception: " + result.getError());
        sendResponsePacket(portType, localAddress, ErrorPacket::invalidPayload(packet.getRoutingChunk()));
        return;
    }

    if (result.isEmpty()) {
        Logger::logInfo(
            getClassNameString() + ": Execution successful but no response packet for OpCode: " + std::to_string(opCode));
        return;
    }

    const Packet responsePacket = result.getValue();
    sendResponsePacket(portType, localAddress, responsePacket);
}

void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const Address &localAddress,
                                             const Packet &responsePacket) const {
    Logger::logInfo(
        "Sending response packet..." + responsePacket.encode() +
        " to " + std::to_string(localAddress.getValue()) +
        " through " + IPort::portTypeToString(portType)
    );
    switch (portType) {
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
