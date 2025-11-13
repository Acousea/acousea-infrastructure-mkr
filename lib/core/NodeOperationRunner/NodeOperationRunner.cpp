#include "NodeOperationRunner.h"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <climits> // for  ULONG_MAX
#include <cinttypes> // for PRId32


namespace
{
    bool isRequeueAllowed(const uint8_t packetBodyTag, const uint8_t packetPayloadTag) noexcept
    {
        // Currently, only Command packets with SetConfiguration payloads are allowed to be requeued
        return ((packetBodyTag == acousea_CommunicationPacket_command_tag)
                && (packetPayloadTag == acousea_CommandBody_setConfiguration_tag ||
                    packetPayloadTag == acousea_CommandBody_requestedConfiguration_tag))
            || ((packetBodyTag == acousea_CommunicationPacket_response_tag)
                && (packetPayloadTag == acousea_ResponseBody_setConfiguration_tag ||
                    packetPayloadTag == acousea_ResponseBody_updatedConfiguration_tag)
            );
    }

    uint8_t getPayloadTag(const acousea_CommunicationPacket& p) noexcept
    {
        switch (p.which_body)
        {
        case acousea_CommunicationPacket_command_tag:
            return p.body.command.which_command;
        case acousea_CommunicationPacket_response_tag:
            return p.body.response.which_response;
        case acousea_CommunicationPacket_report_tag:
            return p.body.report.which_report;
        default:
            return 0;
        }
    }

    bool mustReport(const unsigned long currentMinute, const unsigned long reportingPeriod,
                    const unsigned long lastReportMinute)
    {
        if (reportingPeriod == 0) return false; // No reporting if period is 0 (disabled)
        if (lastReportMinute == ULONG_MAX) return true; // Always report if never reported before
        if (currentMinute == lastReportMinute) return false; // Avoids issues with duplicated reporting for same minute
        return ((currentMinute - lastReportMinute) >= reportingPeriod);
    }

    acousea_CommunicationPacket& buildErrorPacket(const char* errorMessage)
    {
        SharedMemory::resetCommunicationPacket();
        auto& packet = SharedMemory::communicationPacketRef();

        // Reinicializar routing y encabezado
        packet.has_routing = true;
        packet.routing = acousea_RoutingChunk_init_default;

        // Importante: cambiar el tipo de cuerpo antes de acceder al union
        packet.which_body = acousea_CommunicationPacket_error_tag;

        // Inicializar correctamente el cuerpo de error
        packet.body.error = acousea_ErrorBody_init_default;
        strncpy(packet.body.error.errorMessage, errorMessage, sizeof(packet.body.error.errorMessage) - 1);
        packet.body.error.errorMessage[sizeof(packet.body.error.errorMessage) - 1] = '\0';

        return packet;
    }

    // -----------------------------------------------------------------------------
    // Optimized tag-to-string helpers (const char* version)
    // -----------------------------------------------------------------------------
    constexpr const char* bodyAndPayloadTagToCString(const uint8_t bodyTag, const uint8_t payloadTag) noexcept
    {
        switch (bodyTag)
        {
        case acousea_CommunicationPacket_command_tag:
            switch (payloadTag)
            {
            case acousea_CommandBody_setConfiguration_tag: return "Command->SetConfiguration";
            case acousea_CommandBody_requestedConfiguration_tag: return "Command->RequestedConfiguration";
            default: return "Command->Unknown";
            }
        case acousea_CommunicationPacket_response_tag:
            switch (payloadTag)
            {
            case acousea_ResponseBody_setConfiguration_tag: return "Response->SetConfiguration";
            case acousea_ResponseBody_updatedConfiguration_tag: return "Response->UpdatedConfiguration";
            default: return "Response->Unknown";
            }
        case acousea_CommunicationPacket_report_tag:
            switch (payloadTag)
            {
            case acousea_ReportBody_statusPayload_tag: return "Report->StatusPayload";
            default: return "Report->Unknown";
            }
        default:
            return "UnknownBody";
        }
    }
}


NodeOperationRunner::NodeOperationRunner(
    Router& router,
    StorageManager& storageManager,
    NodeConfigurationRepository& nodeConfigurationRepository,
    const std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routines)
    : IRunnable(),
      router(router),
      routines(routines),
      pendingRoutines(storageManager),
      nodeConfigurationRepository(nodeConfigurationRepository)
{
    cache = {
        acousea_OperationMode_init_default,
        0,
        {ULONG_MAX, ULONG_MAX, ULONG_MAX}
    };
}

void NodeOperationRunner::init()
{
    LOG_CLASS_INFO("<Init> Operation Cycle for Operation Mode id=%" PRId32 " name=\"%s\" with configuration:",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
    const auto& currentNodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();

    NodeConfigurationRepository::printNodeConfiguration(currentNodeConfiguration);
    // Search for the operation mode with the given ID
    const auto opModeResult = searchForOperationMode(currentNodeConfiguration.operationModesModule.activeModeId);
    if (opModeResult.isError())
    {
        ERROR_HANDLE_CLASS(": Initial operation mode not found: %s", opModeResult.getError());
        return;
    }

    cache.currentOperationMode = opModeResult.getValueConst();
}

void NodeOperationRunner::run()
{
    LOG_CLASS_INFO("<Start> Operation Cycle for Operation mode %" PRId32 "=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
    tryTransitionOpMode();
    processNextIncomingPacket(nodeConfigurationRepository.getNodeConfiguration().localAddress);
    runPendingRoutines();
    processReportingRoutines();
    cache.cycleCount++;
    LOG_CLASS_INFO("<Finish> Operation Cycle for Operation mode %" PRId32 "=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
}


IRoutine<acousea_CommunicationPacket>* NodeOperationRunner::findRoutine(
    const uint8_t bodyTag, const uint8_t payloadTag) const
{
    LOG_CLASS_INFO("Searching routine for Body %d and Payload %d...",
                   bodyTag, payloadTag
    );
    LOG_CLASS_INFO("The current amount of routines is %d.", static_cast<int>(routines.size()));

    const auto bodyIt = routines.find(bodyTag);
    if (bodyIt == routines.end())
    {
        return nullptr;
    }
    LOG_CLASS_INFO("Finding routine for Body %d and Payload %d...",
                   bodyTag, payloadTag
    );
    const auto& packetBodyRoutinesMap = bodyIt->second;

    LOG_CLASS_INFO("Searching routine for Payload Tag %d...", payloadTag);
    LOG_CLASS_INFO("The current amount of payload routines is %d.",
                   static_cast<int>(packetBodyRoutinesMap.size())
    );

    const auto routineIt = packetBodyRoutinesMap.find(payloadTag);
    if (routineIt == packetBodyRoutinesMap.end() || routineIt->second == nullptr)
    {
        return nullptr;
    }
    LOG_CLASS_INFO("Routine found for Body %d and Payload %d.",
                   bodyTag, payloadTag
    );
    return routineIt->second;
}

void NodeOperationRunner::tryTransitionOpMode()
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
        LOG_CLASS_INFO("Transitioned to next mode... %" PRId32 "=(%s)",
                       cache.currentOperationMode.id,
                       cache.currentOperationMode.name
        );
    }
}

void NodeOperationRunner::tryReport(const IPort::PortType portType,
                                    unsigned long& lastMinute,
                                    const unsigned long currentMinute)
{
    const auto resultCfg = getReportingEntryForCurrentOperationMode(cache.currentOperationMode.id, portType);

    if (resultCfg.isError())
    {
        LOG_CLASS_ERROR("%s", resultCfg.getError());
        return;
    }

    auto [modeId, period] = resultCfg.getValueConst();

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

    IRoutine<acousea_CommunicationPacket>* routinePtr = findRoutine(
        acousea_CommunicationPacket_report_tag,acousea_ReportBody_statusPayload_tag
    );

    LOG_CLASS_INFO("%s Found report routine for reportTag = %d and payloadTag = %d.",
                   IPort::portTypeToCString(portType),
                   acousea_CommunicationPacket_report_tag,
                   acousea_ReportBody_statusPayload_tag
    );

    if (!routinePtr) // Check for null pointer
    {
        LOG_CLASS_ERROR("Report routine for reportTag = %d and payloadTag = %d not found. Cannot send report.",
                        acousea_CommunicationPacket_report_tag, acousea_ReportBody_statusPayload_tag
        );

        return;
    }
    LOG_CLASS_INFO("%s Executing routine...", IPort::portTypeToCString(portType));

    LOG_CLASS_INFO("Routine pointer address: %p", routinePtr);

    LOG_FREE_MEMORY("PRE_ROUTINE_EXECUTION");
    auto outPacketPtr = executeRoutine(
        routinePtr, // Routine pointer
        nullptr, // No input packet for reporting
        portType, // Port type
        0, // Remaining attempts (not used for reporting)
        false // Requeue not allowed for reporting
    );
    LOG_CLASS_INFO("%s Report routine executed.", IPort::portTypeToCString(portType));
    LOG_FREE_MEMORY("POST_ROUTINE_EXECUTION");

    if (!outPacketPtr) // Check for valid result (ptr)
    {
        LOG_CLASS_ERROR("%s Report routine failed to generate report packet. Skipping report...",
                        IPort::portTypeToCString(portType));
        lastMinute = currentMinute;
        return;
    }

    sendResponsePacket(portType, // Port type
                       nodeConfigurationRepository.getNodeConfiguration().localAddress, // Local address
                       nullptr, // Input packet (No input packet for reporting)
                       outPacketPtr // Output packet
    );
    lastMinute = currentMinute;
}


void NodeOperationRunner::processReportingRoutines()
{
    const auto currentMinute = getMillis() / 60000;
    const auto& currentNodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();

    currentNodeConfiguration.has_iridiumModule
        ? tryReport(IPort::PortType::SBDPort, cache.lastReportMinute.sbd, currentMinute)
        : LOG_CLASS_INFO("Iridium module not present, skipping Iridium report.");

#ifdef PLATFORM_HAS_LORA
    currentNodeConfiguration.has_loraModule
        ? tryReport(IPort::PortType::LoraPort, cache.lastReportMinute.lora, currentMinute)
        : LOG_CLASS_INFO("LoRa module not present, skipping LoRa report.");
#endif
#ifdef PLATFORM_HAS_GSM
    currentNodeConfiguration.has_gsmMqttModule
        ? tryReport(IPort::PortType::GsmMqttPort, cache.lastReportMinute.gsmMqtt, currentMinute)
        : LOG_CLASS_INFO("GSM-MQTT module not present, skipping GSM-MQTT report.");
#endif
}

void NodeOperationRunner::processNextIncomingPacket(const uint8_t& localAddress)
{
    LOG_CLASS_INFO("processNextIncomingPacket for local address %d ...", localAddress);
    const bool routerSyncOk = router.syncAllPorts();

    if (!routerSyncOk)
    {
        LOG_CLASS_ERROR("Failed to sync all router ports. Aborting packet processing.");
    }
    LOG_CLASS_INFO("Router ports synced successfully.");

    auto optNextPacketPair = router.nextPacket(localAddress);
    if (!optNextPacketPair.has_value())
    {
        LOG_CLASS_INFO("No incoming packets to process.");
        return;
    }

    auto& [portType, nextInPacketPtr] = *optNextPacketPair;
    acousea_CommunicationPacket* outPacketPtr = processPacket(portType, nextInPacketPtr);
    sendResponsePacket(portType, localAddress, nextInPacketPtr, outPacketPtr);
}


void NodeOperationRunner::runPendingRoutines()
{
    // First run incomplete routines
    auto entryPtr = pendingRoutines.next();
    while (entryPtr)
    {
        const auto& entry = *entryPtr;
        LOG_CLASS_INFO("Re-attempting pending routine %s with %d attempts left.", entry.routine->routineName,
                       entry.remainingAttempts);

        executeRoutine(entry.routine,
                       entry.packet,
                       entry.portResponseTo,
                       entry.remainingAttempts,
                       entry.packet
                           ? isRequeueAllowed(entry.packet->which_body, getPayloadTag(*entry.packet))
                           : false
        );
        // Move to next pending routine
        entryPtr = pendingRoutines.next();
    }
    LOG_CLASS_INFO("Finished processing pending routines.");
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             const acousea_CommunicationPacket* inputPacketPtr,
                                             acousea_CommunicationPacket* outputPacketPtr) const
{
    if (!outputPacketPtr) // Check for null pointer
    {
        LOG_CLASS_ERROR("No output packet provided to sendResponsePacket. Aborting send.");
        return;
    }

    auto& outputPacketRef = *outputPacketPtr;

    LOG_CLASS_INFO("Sending response packet with type %s through %s from local address %d ...",
                   bodyAndPayloadTagToCString(outputPacketRef.which_body, getPayloadTag(outputPacketRef)),
                   IPort::portTypeToCString(portType),
                   localAddress
    );

    if (inputPacketPtr) // Invert routing info if input packet is provided
    {
        LOG_CLASS_INFO("Setting routing info in response packet...");
        const auto inputPacket = *inputPacketPtr;
        const uint8_t destination = inputPacket.has_routing ? inputPacket.routing.sender : Router::originAddress;
        outputPacketRef.packetId = inputPacket.packetId;
        outputPacketRef.has_routing = true;
        outputPacketRef.routing = acousea_RoutingChunk_init_default;
        outputPacketRef.routing.sender = localAddress;
        outputPacketRef.routing.receiver = destination;
    }

    if (const auto sendOk = router.from(localAddress).through(portType).send(outputPacketRef); !sendOk)
    {
        LOG_CLASS_ERROR(": Failed to send response packet through %s", IPort::portTypeToCString(portType));
    }
}


acousea_CommunicationPacket* NodeOperationRunner::processPacket(
    const IPort::PortType portType, acousea_CommunicationPacket* inPacketPtr)
{
    if (!inPacketPtr) // Check for null pointer
    {
        LOG_CLASS_ERROR("No input packet provided to processPacket. Dropping packet...");
        return nullptr;
    }

    if (!inPacketPtr->has_routing)
    {
        LOG_CLASS_ERROR("Packet without routing. Dropping packet...");
        // Puedes devolver un error, o bien construir una respuesta de error.
        return &buildErrorPacket("Packet has no routing information.");
    }

    LOG_CLASS_INFO("Received Packet from %" PRId32 " of type %s",
                   inPacketPtr->routing.sender,
                   bodyAndPayloadTagToCString(inPacketPtr->which_body, getPayloadTag(*inPacketPtr))

    );

    const auto bodyTag = inPacketPtr->which_body;
    const auto payloadTag = getPayloadTag(*inPacketPtr);

    const auto routinePtr = findRoutine(bodyTag, payloadTag);
    if (!routinePtr)
    {
        LOG_CLASS_ERROR("Routine not found for Body %d and Payload %d. Building error packet...",
                        bodyTag, payloadTag);
        return &buildErrorPacket("Routine not found.");
    }
    return executeRoutine(
        routinePtr, // Routine pointer
        inPacketPtr, // Input packet
        portType, // Port type
        PendingRoutines<0>::MAX_ATTEMPTS, // Remaining attempts
        isRequeueAllowed(bodyTag, payloadTag) // Requeue allowed
    );
}


acousea_CommunicationPacket* NodeOperationRunner::executeRoutine(
    IRoutine<acousea_CommunicationPacket>* routine,
    acousea_CommunicationPacket* const optInputPacket, // Const pointer, not const data!
    const IPort::PortType portType,
    const uint8_t remainingAttempts,
    const bool requeueAllowed
)
{
    // Execute the routine (NO NEED TO CHECK FOR NULLPTR, SOME ROUTINES USE NULLPTR)
    Result<acousea_CommunicationPacket*> result = routine->execute(optInputPacket);

    if (result.isPending() && (remainingAttempts <= 0 || !requeueAllowed))
    {
        LOG_CLASS_WARNING("%s[NO REQUEUE ALLOWED] => incomplete with message: %s", routine->routineName,
                          result.getError());

        return &buildErrorPacket(result.getError());
    }

    if (result.isPending() && remainingAttempts > 0 && requeueAllowed)
    {
        // pendingRoutines.add({routine, optInputPacket, static_cast<uint8_t>(remainingAttempts - 1), portType});
        LOG_CLASS_WARNING("%s [REQUEUED] => incomplete with message: %s", routine->routineName,
                          result.getError());
        return nullptr;
    }

    if (result.isError())
    {
        LOG_CLASS_ERROR("%s => failed with message: %s", routine->routineName, result.getError());
        if (!optInputPacket) // Check for null pointer
        {
            LOG_CLASS_INFO("%s: Cannot send error packet, there was no original packet.", routine->routineName);
            return nullptr;
        }

        return &buildErrorPacket(result.getError());
    }
    LOG_CLASS_INFO("%s => succeeded.", routine->routineName);
    return result.getValueConst();
}

//---------------------------------------------- Helpers ------------------------------------------------//

Result<acousea_OperationMode> NodeOperationRunner::searchForOperationMode(const uint8_t modeId) const
{
    const auto& currentNodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();
    if (!currentNodeConfiguration.has_operationModesModule
        || currentNodeConfiguration.operationModesModule.modes_count == 0)
    {
        return RESULT_CLASS_FAILUREF(acousea_OperationMode, "No operation modes module in configuration.");
    }

    for (int i = 0; i < currentNodeConfiguration.operationModesModule.modes_count; ++i)
    {
        if (currentNodeConfiguration.operationModesModule.modes[i].id == modeId)
            return RESULT_SUCCESS(acousea_OperationMode, currentNodeConfiguration.operationModesModule.modes[i]);
    }

    LOG_CLASS_ERROR("Operation mode %d not found in configuration.", modeId);
    return RESULT_CLASS_FAILUREF(acousea_OperationMode, "Operation mode %d not found in configuration.", modeId);
}

Result<acousea_ReportingPeriodEntry> NodeOperationRunner::getReportingEntryForCurrentOperationMode(
    const uint8_t modeId, const IPort::PortType portType) const
{
    const auto& currentNodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();
    const acousea_ReportingPeriodEntry* entries = nullptr;
    size_t entryCount = 0;
    switch (portType)
    {
    case IPort::PortType::SBDPort:
        {
            if (!currentNodeConfiguration.has_iridiumModule ||
                currentNodeConfiguration.iridiumModule.entries_count == 0)
            {
                return RESULT_CLASS_FAILUREF(acousea_ReportingPeriodEntry,
                                             "No Iridium reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration.iridiumModule.entries;
            entryCount = currentNodeConfiguration.iridiumModule.entries_count;
            break;
        }
#ifdef PLATFORM_HAS_LORA
    case IPort::PortType::LoraPort:
        {
            if (!currentNodeConfiguration.has_loraModule || currentNodeConfiguration.loraModule.entries_count == 0)
            {
                return RESULT_CLASS_FAILUREF(acousea_ReportingPeriodEntry,
                                             "No LoRa reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration.loraModule.entries;
            entryCount = currentNodeConfiguration.loraModule.entries_count;
            break;
        }
#endif // PLATFORM_HAS_LORA
#ifdef PLATFORM_HAS_GSM
    case IPort::PortType::GsmMqttPort:
        {
            if (!currentNodeConfiguration.has_gsmMqttModule ||
                currentNodeConfiguration.gsmMqttModule.entries_count == 0)
            {
                return RESULT_CLASS_FAILUREF(acousea_ReportingPeriodEntry,
                                             "No GSM-MQTT reporting entries defined in configuration.");
            }
            entries = currentNodeConfiguration.gsmMqttModule.entries;
            entryCount = currentNodeConfiguration.gsmMqttModule.entries_count;
            break;
        }
#endif // PLATFORM_HAS_GSM
    default:
        return RESULT_CLASS_FAILUREF(acousea_ReportingPeriodEntry, "Unsupported port type for reporting entry.");
    }

    for (size_t i = 0; i < entryCount; ++i)
    {
        if (entries[i].modeId == modeId) return RESULT_SUCCESS(acousea_ReportingPeriodEntry, entries[i]);
    }
    return RESULT_CLASS_FAILUREF(acousea_ReportingPeriodEntry,
                                 "::getReportingEntryForCurrentOperationMode() Reporting entry for mode %d not found.",
                                 modeId
    );
}
