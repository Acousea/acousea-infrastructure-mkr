#include "NodeOperationRunner.h"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <climits> // for  ULONG_MAX
#include <cinttypes> // for PRId32

#include "Ports/IPort.h"
#include "SharedMemory/SharedMemory.hpp"


namespace
{
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
        case acousea_CommunicationPacket_error_tag:
            return acousea_ErrorBody_errorMessage_tag; // Has only one payload (no internal one-of -> no which_error)
        default:
            return 0;
        }
    }

    acousea_CommunicationPacket& buildErrorPacketF(const char* fmt, ...)
    {
        // Formatear el mensaje de error en el buffer temporal
        auto* tmpErrMsgBuffer = reinterpret_cast<char*>(SharedMemory::tmpBuffer());
        SharedMemory::clearTmpBuffer();

        va_list args;
        va_start(args, fmt);
        vsnprintf(tmpErrMsgBuffer, SharedMemory::tmpBufferSize(), fmt, args); // Prints to tmp buffer
        va_end(args);

        // Ahora construir el paquete usando SOLO este buffer
        SharedMemory::resetCommunicationPacket();
        auto& packet = SharedMemory::communicationPacketRef();

        // Ruta estándar
        packet.has_routing = true;
        packet.routing = acousea_RoutingChunk_init_default;

        // Elegimos el tipo del union ANTES de tocarlo
        packet.which_body = acousea_CommunicationPacket_error_tag;

        // Inicializamos correctamente el body
        packet.body.error = acousea_ErrorBody_init_default;

        // Copiar mensaje al campo nanopb sin desbordar
        strncpy(packet.body.error.errorMessage, tmpErrMsgBuffer, sizeof(packet.body.error.errorMessage) - 1);

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
        case acousea_CommunicationPacket_error_tag:
            {
                return "Error->ErrorMessage";
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
      nodeConfigurationRepository(nodeConfigurationRepository),
      routines(routines)
{
    cache = {
        acousea_OperationMode_init_default,
        0,
        {0, 0, 0}
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
    processNextIncomingPacket();
    processReportingRoutines();
    cache.cycleCount++;
    LOG_CLASS_INFO("<Finish> Operation Cycle for Operation mode %" PRId32 "=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
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

void NodeOperationRunner::processReportingRoutines()
{
    const auto currentMinute = getMillis() / 60000;
    const auto& currentNodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();

    currentNodeConfiguration.has_iridiumModule
        ? tryReport(IPort::PortType::SBDPort, cache.nextReportMinute.sbd, currentMinute)
        : LOG_CLASS_INFO("Iridium module not present, skipping Iridium report.");

#ifdef PLATFORM_HAS_LORA
    currentNodeConfiguration.has_loraModule
        ? tryReport(IPort::PortType::LoraPort, cache.nextReportMinute.lora, currentMinute)
        : LOG_CLASS_INFO("LoRa module not present, skipping LoRa report.");
#endif
#ifdef PLATFORM_HAS_GSM
    currentNodeConfiguration.has_gsmMqttModule
        ? tryReport(IPort::PortType::GsmMqttPort, cache.nextReportMinute.gsmMqtt, currentMinute)
        : LOG_CLASS_INFO("GSM-MQTT module not present, skipping GSM-MQTT report.");
#endif
}

void NodeOperationRunner::tryReport(const IPort::PortType port,
                                    unsigned long& nextReportMinute,
                                    const unsigned long currentMinute)
{
    const auto& nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    const auto cfgResult = getReportingEntryForCurrentOperationMode(cache.currentOperationMode.id, port);

    if (cfgResult.isError())
    {
        LOG_CLASS_ERROR("%s", cfgResult.getError());
        return;
    }

    auto [modeId, period] = cfgResult.getValueConst();

    LOG_CLASS_INFO("Trying to report on %s. Config: { Period=%lu, Current minute=%lu, Next report minute=%lu }",
                   IPort::portTypeToCString(port),
                   period,
                   currentMinute,
                   nextReportMinute
    );

    if (currentMinute != nextReportMinute)
    {
        LOG_CLASS_INFO("Not time to report yet on %s (currentMinute=%lu, nextReportMinute=%lu). Skipping...",
                       IPort::portTypeToCString(port),
                       currentMinute,
                       nextReportMinute
        );
        return;
    }

    IRoutine<acousea_CommunicationPacket>* routinePtr = findRoutine(
        acousea_CommunicationPacket_report_tag,acousea_ReportBody_statusPayload_tag
    );

    LOG_CLASS_INFO("%s Found report routine for reportTag = %d and payloadTag = %d.",
                   IPort::portTypeToCString(port),
                   acousea_CommunicationPacket_report_tag,
                   acousea_ReportBody_statusPayload_tag
    );

    if (!routinePtr) // Check for null pointer
    {
        LOG_CLASS_ERROR("Report routine for reportTag = %d and payloadTag = %d not found. Cannot send report.",
                        acousea_CommunicationPacket_report_tag, acousea_ReportBody_statusPayload_tag
        );
        nextReportMinute += period; // Schedule next report
        return;
    }
    LOG_CLASS_INFO("%s Executing routine...", IPort::portTypeToCString(port));
    LOG_CLASS_INFO("Routine pointer address: %p", routinePtr);

    auto [resultType, outPacketPtr] = executeRoutine(routinePtr, nullptr); // No input packet for report routines

    if (resultType == Result<void>::Type::Incomplete)
    {
        LOG_CLASS_WARNING("Report %s execution incomplete on port %s. Will retry in one minute",
                          routinePtr->routineName, IPort::portTypeToCString(port)
        );
        nextReportMinute += 1; // Force retry in one minute
        return;
    }

    // For success and failure we check the output packet
    if (!outPacketPtr && (resultType == Result<void>::Type::Success || resultType == Result<void>::Type::Failure))
    {
        LOG_CLASS_ERROR("%s Report routine with RESULT = %s for port %s failed to produce an output packet.",
                        routinePtr->routineName,
                        resultType == Result<void>::Type::Success ? "Success" : "Failure",
                        IPort::portTypeToCString(port)
        );
        routinePtr->reset(); // Reset routine state
        nextReportMinute += period; // Schedule next report
        return;
    }

    LOG_CLASS_INFO("%s Report routine successfully executed.", IPort::portTypeToCString(port));


    if (const auto sendOk = sendResponsePacket(nodeConfig.localAddress, port, Router::originAddress, outPacketPtr);
        !sendOk)
    {
        LOG_CLASS_ERROR("Failed to send report through %s. Will retry again", IPort::portTypeToCString(port));
        nextReportMinute += 1; // Force retry in one minute
    }
    else
    {
        LOG_CLASS_INFO("Report packet sent successfully through %s.", IPort::portTypeToCString(port));
        routinePtr->reset(); // Reset routine state
        nextReportMinute += period; // Schedule next report
    }
}


void NodeOperationRunner::skipPacketForPort(const uint32_t packetId, IPort::PortType portType)
{
    // Discard the packet
    if (const bool discardOk = router.skipToNextPacket(portType); !discardOk)
    {
        LOG_CLASS_ERROR("Failed to skip packet id % " PRId32 " from port %s.",
                        packetId,
                        IPort::portTypeToCString(portType)
        );
    }
}

void NodeOperationRunner::processNextIncomingPacket()
{
    const auto& localAddress = nodeConfigurationRepository.getNodeConfiguration().localAddress;
    LOG_CLASS_INFO("processNextIncomingPacket for local address %lu ...", localAddress);

    if (const bool routerSyncOk = router.syncAllPorts(); !routerSyncOk)
    {
        LOG_CLASS_ERROR("Failed to sync all router ports. Aborting packet processing.");
    }
    LOG_CLASS_INFO("Router ports synced successfully.");

    auto optNextPacketPair = router.peekNextPacket(localAddress);
    if (!optNextPacketPair.has_value())
    {
        LOG_CLASS_INFO("No incoming packets to process.");
        return;
    }

    auto& [portType, nextInPacketPtr] = *optNextPacketPair;

    if (!nextInPacketPtr) // Check for null pointer
    {
        LOG_CLASS_ERROR("No incoming packet retrieved. Aborting packet processing.");
        return;
    }

    if (!nextInPacketPtr->has_routing)
    {
        LOG_CLASS_ERROR("Packet with id % " PRId32 " has no routing information. Discarding packet.",
                        nextInPacketPtr->packetId
        );
        return;
    }

    const auto packetId = nextInPacketPtr->packetId;
    const auto nextInPacketSender = static_cast<uint8_t>(nextInPacketPtr->routing.sender);
    const auto bodyTag = nextInPacketPtr->which_body;
    const auto payloadTag = getPayloadTag(*nextInPacketPtr);

    LOG_CLASS_INFO("Processing incoming packet with id % " PRId32 " from sender %d  with type %s through port %s ...",
                   packetId,
                   nextInPacketSender,
                   bodyAndPayloadTagToCString(bodyTag, payloadTag),
                   IPort::portTypeToCString(portType));


    const auto routinePtr = findRoutine(bodyTag, payloadTag);
    if (!routinePtr)
    {
        LOG_CLASS_ERROR("Routine not found for Body %d and Payload %d. Building error packet...",
                        bodyTag, payloadTag);
        const auto errorPkt = &buildErrorPacketF(
            "This node cannot process the received packet. No routine for Body=%d and Payload=%d.", bodyTag, payloadTag
        );
        // return sendResponsePacket(portType, localAddress, nextInPacketSender, errorPkt);
        [[maybe_unused]] const bool sendOk = sendResponsePacket(localAddress, portType, nextInPacketSender, errorPkt);
        skipPacketForPort(packetId, portType);
        return;
    }

    // Check if requeue is allowed for this packet type
    auto& retryEntry = retryAttemptsLeft_[static_cast<uint8_t>(portType)];
    if (retryEntry.packetId != packetId)
    {
        LOG_CLASS_WARNING("Another packet is being retried on port %s. "
                          "New packet id % " PRId32 " will not be processed until retries are done for packet id % "
                          PRId32 ".",
                          IPort::portTypeToCString(portType),
                          packetId,
                          retryEntry.packetId
        );
        retryEntry.packetId = packetId;
        retryEntry.processingAttemptsLeft = MAX_RETRIES;
        retryEntry.sendingAttemptsLeft = MAX_RETRIES;
        return;
    }

    if (retryEntry.processingAttemptsLeft == 0 || retryEntry.sendingAttemptsLeft == 0)
    {
        LOG_CLASS_ERROR(
            "Ran out of attempts for packet id % " PRId32
            " on port %s. (processingAttempts= %d, sendingAttempts=%d). Skipping packet...",
            packetId, IPort::portTypeToCString(portType),
            retryEntry.processingAttemptsLeft, retryEntry.sendingAttemptsLeft
        );
        // Reset retry entry
        retryEntry = RetryEntry{0, 0, 0};
        skipPacketForPort(packetId, portType);
        return;
    }

    // Execute the routine and get the output packet
    auto [resultType, outPacketPtr] = executeRoutine(routinePtr, nextInPacketPtr);

    if (resultType == Result<void>::Type::Incomplete)
    {
        LOG_CLASS_WARNING("Routine %s execution incomplete for packet id % " PRId32 " on port %s. (Attempts left: %d)",
                          routinePtr->routineName, packetId,
                          IPort::portTypeToCString(portType), retryEntry.processingAttemptsLeft
        );
        // Skip to next packet if result was successfull
        retryEntry.processingAttemptsLeft -= 1; // Incomplete routine execution
        return;
    }

    if (!outPacketPtr) // Check for null pointer
    {
        LOG_CLASS_ERROR(
            "Routine %s with RESULT = %s did not produce an output packet for packet id % " PRId32
            " on port %s. Skipping packet.",
            routinePtr->routineName, resultType == Result<void>::Type::Failure ? "FAILURE" : "SUCCESS",
            packetId, IPort::portTypeToCString(portType)
        );
        skipPacketForPort(packetId, portType);
        routinePtr->reset(); // Reset routine state
        return;
    }
    // Ensure that the packet ID is preserved
    outPacketPtr->packetId = packetId;

    // Send the packet. If successful, clear the retry entry and skip the packet from the port queue.
    // If sending fails, decrement sending attempts left.
    if (const bool sendOk = sendResponsePacket(localAddress, portType, nextInPacketSender, outPacketPtr);
        !sendOk)
    {
        retryEntry.sendingAttemptsLeft -= 1; // Sending failed
        LOG_CLASS_WARNING("Packet with id % " PRId32 " on port %s has %d attempts left",
                          packetId, IPort::portTypeToCString(portType), retryEntry.sendingAttemptsLeft
        );
    }
    else
    {
        retryEntry = RetryEntry{0, 0};
        skipPacketForPort(packetId, portType);
        routinePtr->reset(); // Reset routine state
    }
}


bool NodeOperationRunner::sendResponsePacket(const uint32_t& sender,
                                             const IPort::PortType portType,
                                             const uint8_t destination,
                                             acousea_CommunicationPacket* outPacketPtr)
{
    const auto sendOk = router.from(sender).to(destination).through(portType).send(*outPacketPtr);
    if (!sendOk)
    {
        LOG_CLASS_ERROR(
            "Failed to send packet with id % " PRId32 " through %s",
            outPacketPtr->packetId, IPort::portTypeToCString(portType)
        );
    }
    else
    {
        LOG_CLASS_INFO(
            "Packet for packet id % " PRId32 " sent successfully through %s",
            outPacketPtr->packetId, IPort::portTypeToCString(portType)
        );
    }
    return sendOk;
}

// -------------------------------------- Packet Sending --------------------------------------

std::pair<Result<void>::Type, acousea_CommunicationPacket*> NodeOperationRunner::executeRoutine(
    IRoutine<acousea_CommunicationPacket>* routine,
    acousea_CommunicationPacket* const optInputPacket
)
{
    // Execute the routine (NO NEED TO CHECK FOR NULLPTR, SOME ROUTINES USE NULLPTR)
    LOG_FREE_MEMORY("PRE_ROUTINE_EXECUTION");
    const Result<acousea_CommunicationPacket*> result = routine->execute(optInputPacket);
    LOG_FREE_MEMORY("POST_ROUTINE_EXECUTION");

    if (result.isIncomplete())
    {
        LOG_CLASS_WARNING("%s => incomplete with message: %s", routine->routineName, result.getError());
        return {Result<void>::Type::Incomplete, nullptr};
    }

    if (result.isError())
    {
        LOG_CLASS_ERROR("%s => failed with message: %s", routine->routineName, result.getError());
        if (!optInputPacket) // Check for null pointer
        {
            LOG_CLASS_INFO("%s: Cannot send error packet, there was no original packet.", routine->routineName);
            return {Result<void>::Type::Failure, nullptr};
        }

        return {Result<void>::Type::Failure, &buildErrorPacketF(result.getError())};
    }

    LOG_CLASS_INFO("%s => succeeded.", routine->routineName);
    return {Result<void>::Type::Success, result.getValueConst()};
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


IRoutine<acousea_CommunicationPacket>* NodeOperationRunner::findRoutine(
    const uint8_t bodyTag, const uint8_t payloadTag) const
{
    // dumpRoutinesMap();
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
    if (routineIt == packetBodyRoutinesMap.end())
    {
        return nullptr;
    }
    LOG_CLASS_INFO("Routine found for Body %d and Payload %d on address %p.",
                   bodyTag, payloadTag, routineIt->second
    );
    return routineIt->second;
}

void NodeOperationRunner::dumpRoutinesMap() const
{
    LOG_CLASS_INFO("=== BEGIN ROUTINES MAP DUMP ===");
    LOG_CLASS_INFO("Total body tags in routines map: %d",
                   static_cast<int>(routines.size()));

    for (const auto& bodyEntry : routines)
    {
        const auto bodyTag = bodyEntry.first;
        const auto& packetBodyRoutinesMap = bodyEntry.second;

        LOG_CLASS_INFO(" BodyTag %d -> has %d payload entries",
                       bodyTag, static_cast<int>(packetBodyRoutinesMap.size()));

        for (const auto& payloadEntry : packetBodyRoutinesMap)
        {
            const auto payloadTag = payloadEntry.first;
            auto* routinePtr = payloadEntry.second;
            LOG_CLASS_INFO("    PayloadTag %d -> Routine ptr %p",
                           payloadTag, routinePtr);
        }
    }
    LOG_CLASS_INFO("=== END ROUTINES MAP DUMP ===");
}
