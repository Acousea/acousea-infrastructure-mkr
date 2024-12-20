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
    Logger::logInfo("Initializing with configuration...");
    nodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfiguration->print();
    cache.currentOperationMode = nodeConfiguration->getOperationGraphModule()->getGraph().begin()->first;
}

void NodeOperationRunner::run() {
    processIncomingPackets(nodeConfiguration->getLocalAddress());
    checkIfMustTransition();
    runRoutines();
    cache.cycleCount++;
}

void NodeOperationRunner::finish() {
    Logger::logInfo("Stopping Working Mode...");
}

void NodeOperationRunner::checkIfMustTransition() {
    auto maxDuration = nodeConfiguration->getOperationGraphModule()->getGraph().at(
            cache.currentOperationMode).duration;
    if (cache.cycleCount >= maxDuration) {
        cache.cycleCount = 0;
        cache.currentOperationMode = nodeConfiguration->getOperationGraphModule()->getGraph().at(
                cache.currentOperationMode).nextMode;
    }
}

void NodeOperationRunner::runRoutines() {
    auto currentMinute = millis() / 60000;

    // Handle Iridium reporting
    if (nodeConfiguration->getIridiumModule().has_value()) {
        const auto &iridiumConfig = nodeConfiguration->getIridiumModule()->getConfigurations().at(
                cache.currentOperationMode);

        if (currentMinute - cache.lastReportMinute.sbd >= iridiumConfig.getPeriod()) {
            processRoutine(iridiumConfig, IPort::PortType::SBDPort, currentMinute, cache.lastReportMinute.sbd);
        }
    }

    // Handle LoRa reporting
    if (nodeConfiguration->getLoraModule().has_value()) {
        const auto &loraConfig = nodeConfiguration->getLoraModule()->getConfigurations().at(
                cache.currentOperationMode);

        if (currentMinute - cache.lastReportMinute.lora >= loraConfig.getPeriod()) {
            processRoutine(loraConfig, IPort::PortType::LoraPort, currentMinute, cache.lastReportMinute.lora);
        }
    }
}

void NodeOperationRunner::processRoutine(const ReportingConfiguration &config, IPort::PortType portType,
                                         unsigned long currentMinute, unsigned long &lastReportMinute) {
    IRoutine<VoidType> *routine = nullptr;

    switch (config.getReportType()) {
        case ReportingConfiguration::ReportType::COMPLETE:
            routine = internalRoutines.find(OperationCode::Code::COMPLETE_STATUS_REPORT)->second;
            break;
        case ReportingConfiguration::ReportType::BASIC:
            routine = internalRoutines.find(OperationCode::Code::BASIC_STATUS_REPORT)->second;
            break;
        case ReportingConfiguration::ReportType::SUMMARY:
            // Implement or fetch routine for SUMMARY if needed
            break;
        default:
            ErrorHandler::handleError("Invalid report type");
            return;
    }

    if (!routine) {
        ErrorHandler::handleError(NodeOperationRunner::getClassNameString() + "Routine not found for report type");
        return;
    }

    auto result = routine->execute();
    if (result.isError()) {
        ErrorHandler::handleError("Routine execution failed: " + result.getError());
        return;
    }
    sendResponsePacket(portType, nodeConfiguration->getLocalAddress(), result.getValue());
    lastReportMinute = currentMinute;
}

void NodeOperationRunner::processIncomingPackets(const Address &localAddress) {
    auto receivedPackets = router.readPorts(localAddress);

    for (const auto &[portType, packets]: receivedPackets) {
        for (auto &packet: packets) {
            SerialUSB.print("Processing packet from port type: ");
            SerialUSB.println(static_cast<int>(portType));

            // Handle packet processing
            processPacket(portType, packet, localAddress);
        }
    }
}

void NodeOperationRunner::processPacket(IPort::PortType portType, const Packet &packet, const Address &localAddress) {
    OperationCode::Code opCode = packet.getOpCodeEnum();

    if (externalRoutines.find(opCode) == externalRoutines.end()) {
        SerialUSB.println(getClassNameCString() + String("Exception: Invalid OpCode"));
        sendResponsePacket(portType, localAddress, ErrorPacket::invalidOpcode(packet.getRoutingChunk()));
        return;
    }

    Result<Packet> result = externalRoutines[opCode]->execute(packet);
    if (result.isError()) {
        SerialUSB.println(getClassNameCString() + String("Exception: ") + result.getError().c_str());
        sendResponsePacket(portType, localAddress, ErrorPacket::invalidPayload(packet.getRoutingChunk()));
        return;
    }

    Packet responsePacket = result.getValue();
    sendResponsePacket(portType, localAddress, responsePacket);
}

void NodeOperationRunner::sendResponsePacket(IPort::PortType portType, const Address &localAddress,
                                             const Packet &responsePacket) {
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
            SerialUSB.println("Unknown port type for response packet!");
            break;
    }
}
