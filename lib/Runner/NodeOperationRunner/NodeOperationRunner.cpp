#include "NodeOperationRunner.h"

NodeOperationRunner::NodeOperationRunner(Router& router,
                                         const std::map<uint8_t, IRoutine<VoidType>*>& internalRoutines,
                                         const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>&
                                         externalRoutines,
                                         const NodeConfigurationRepository& nodeConfigurationRepository)
    : IRunnable(),
      router(router),
      internalRoutines(internalRoutines),
      externalRoutines(externalRoutines),
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
    runRoutines();
}

void NodeOperationRunner::finish()
{
    cache.cycleCount++;
    Logger::logInfo("[Finish] Operation Cycle for Operation mode=" + std::to_string(cache.currentOperationMode.key));
}

acousea_OperationModesGraphModule_GraphEntry NodeOperationRunner::searchForOperationMode(const uint8_t modeId) const
{
    for (int i = 0; i < currentNodeConfiguration->operationGraphModule.graph_count; ++i)
    {
        if (currentNodeConfiguration->operationGraphModule.graph[i].key == modeId)
        {
            return currentNodeConfiguration->operationGraphModule.graph[i];
        }
    }
    Logger::logError("Operation mode " + std::to_string(modeId) + " not found. Returning default mode.");
    return currentNodeConfiguration->operationGraphModule.graph[0];
}

acousea_ReportingPeriodEntry NodeOperationRunner::searchForReportingEntry(
    const uint8_t modeId, const acousea_ReportingPeriodEntry* entries, const size_t entryCount)
{
    for (size_t i = 0; i < entryCount; ++i)
    {
        if (entries[i].modeId == modeId)
        {
            return entries[i];
        }
    }
    Logger::logError("Reporting entry for mode " + std::to_string(modeId) + " not found. Returning default entry.");
    return entries[0];
}


void NodeOperationRunner::checkIfMustTransition()
{
    const auto maxDuration = cache.currentOperationMode.value.duration;

    if (cache.cycleCount >= maxDuration)
    {
        cache.cycleCount = 0;
        const auto nextOpMode = searchForOperationMode(cache.currentOperationMode.value.targetMode);
        cache.currentOperationMode = nextOpMode;
        Logger::logInfo("Transitioned to next mode..." + std::to_string(cache.currentOperationMode.key));
    }
}

bool NodeOperationRunner::mustReport(const unsigned long currentMinute, const unsigned long reportingPeriod,
                                     const unsigned long lastReportMinute)
{
    return (currentMinute - lastReportMinute >= reportingPeriod && reportingPeriod != 0)
        || lastReportMinute == 0;
}

void NodeOperationRunner::runRoutines()
{
    const auto currentMinute = getMillis() / 60000;

    if (!currentNodeConfiguration.has_value())
    {
        ErrorHandler::handleError(getClassNameString() + ": Node configuration not loaded.");
        return;
    }

    // Handle Iridium reporting
    if (currentNodeConfiguration->has_iridiumModule)
    {
        const auto& reportingPeriodEntry = NodeOperationRunner::searchForReportingEntry(
            cache.currentOperationMode.key,
            currentNodeConfiguration->iridiumModule.entries,
            currentNodeConfiguration->iridiumModule.entries_count
        );

        Logger::logInfo("Iridium Config: { Period=" + std::to_string(reportingPeriodEntry.period) +
            ", Current minute=" + std::to_string(currentMinute) +
            ", Last report minute=" + std::to_string(cache.lastReportMinute.sbd) + " }");

        if (mustReport(currentMinute, reportingPeriodEntry.period, cache.lastReportMinute.sbd))
        {
            reportRoutine(IPort::PortType::SBDPort, currentMinute, cache.lastReportMinute.sbd);
        }
    }

    // Handle LoRa reporting
    if (currentNodeConfiguration->has_loraModule)
    {
        const auto& loraConfig = NodeOperationRunner::searchForReportingEntry(
            cache.currentOperationMode.key,
            currentNodeConfiguration->loraModule.entries,
            currentNodeConfiguration->loraModule.entries_count
        );

        Logger::logInfo("LoRa Config: { Period=" + std::to_string(loraConfig.period) +
            ", Current minute=" + std::to_string(currentMinute) +
            ", Last report minute=" + std::to_string(cache.lastReportMinute.lora) + " }");

        if (mustReport(currentMinute, loraConfig.period, cache.lastReportMinute.lora))
        {
            reportRoutine(IPort::PortType::LoraPort, currentMinute, cache.lastReportMinute.lora);
        }
    }
}

void NodeOperationRunner::reportRoutine(IPort::PortType portType,
                                        unsigned long currentMinute,
                                        unsigned long& lastReportMinute)
{
    IRoutine<VoidType>* routine = nullptr;

    routine = internalRoutines.find(acousea_PayloadWrapper_statusPayload_tag)->second;
    if (!routine)
    {
        ErrorHandler::handleError(getClassNameString() + ": StatusReport routine not found.");
        return;
    }

    const Result<acousea_CommunicationPacket> result = routine->execute();

    if (result.isError())
    {
        Logger::logError("Routine execution failed: " + result.getError());
        return;
    }

    sendResponsePacket(portType, currentNodeConfiguration->localAddress, result.getValue());
    lastReportMinute = currentMinute;
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
    if (!packet.has_payload)
    {
        ErrorHandler::handleError(getClassNameString() + ": Packet without payload.");
    }

    if (!packet.has_routing)
    {
        ErrorHandler::handleError(getClassNameString() + ": Packet without routing.");
    }


    const uint8_t payloadTypeTag = packet.payload.which_payload;
    const uint8_t sender = packet.routing.sender;

    Logger::logInfo(
        "Processing packet " + std::to_string(payloadTypeTag) +
        " from " + std::to_string(sender) +
        " received through " + IPort::portTypeToString(portType)
    );

    // Routine
    const auto routineResultIterator = externalRoutines.find(payloadTypeTag);
    if (routineResultIterator == externalRoutines.end() || routineResultIterator->second == nullptr)
    {
        Logger::logError(getClassNameString() +
            " Exception: No routine found for payload Tag Type: " + std::to_string(payloadTypeTag)
        );
        return;
    }

    IRoutine<acousea_CommunicationPacket>* routine = routineResultIterator->second;
    const Result<acousea_CommunicationPacket> result = routine->execute(packet);

    if (!result.isSuccess())
    {
        Logger::logError(getClassNameString() + " Exception: " + result.getError());
        return;
    }

    if (result.isIncomplete() && packet.routing.ttl == 0) // FIXME: SHOULD SEND ERROR PACKET BACK TO THE SENDER
    {
        Logger::logError(getClassNameString() + ": Incomplete Packet TTL expired. Dropping packet with payload " +
            std::to_string(payloadTypeTag));
        return;
    }

    if (result.isIncomplete() && packet.routing.ttl > 0)
    {
        // Decrease ttl and keep packet for next cycle
        acousea_CommunicationPacket packetCopy = packet;
        packetCopy.routing.ttl--;
        Logger::logInfo(getClassNameString() +
            ": Execution not yet complete. Waiting for next cycle for payload " +
            std::to_string(payloadTypeTag) + "TTL is now " + std::to_string(packetCopy.routing.ttl)
        );
        router.addIncompletePacket(packetCopy);
        return;
    }

    const acousea_CommunicationPacket responsePacket = result.getValue();
    sendResponsePacket(portType, localAddress, responsePacket);
}

void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             const acousea_CommunicationPacket& responsePacket) const
{
    Logger::logInfo(
        "Sending response packet with Payload Tag" + std::to_string(responsePacket.payload.which_payload) +
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
