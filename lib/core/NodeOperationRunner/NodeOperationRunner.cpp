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

    acousea_CommunicationPacket buildErrorPacket(const char* errorMessage)
    {
        acousea_ErrorBody errorBody;
        strncpy(errorBody.errorMessage, errorMessage, sizeof(errorBody.errorMessage) - 1);
        errorBody.errorMessage[sizeof(errorBody.errorMessage) - 1] = '\0'; // Ensure null termination


        acousea_CommunicationPacket packet = acousea_CommunicationPacket_init_default;
        packet.has_routing = true;
        packet.routing = acousea_RoutingChunk_init_default;
        packet.which_body = acousea_CommunicationPacket_error_tag;
        packet.body.error = errorBody;

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
    const NodeConfigurationRepository& nodeConfigurationRepository,
    const std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routines)
    : IRunnable(),
      router(router),
      routines(routines),
      nodeConfigurationRepository(nodeConfigurationRepository)
{
    cache = {acousea_OperationMode_init_default, 0, {ULONG_MAX, ULONG_MAX, ULONG_MAX}};
}

void NodeOperationRunner::init()
{
    currentNodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    LOG_CLASS_INFO("<Init> Operation Cycle for Operation Mode %" PRId32 "=(%s) with configuration:",
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
    LOG_CLASS_INFO("<Run> Operation Cycle for Operation mode %" PRId32 "=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
    tryTransitionOpMode();
    processIncomingPackets(currentNodeConfiguration->localAddress);
    runPendingRoutines();
    processReportingRoutines();
    cache.cycleCount++;
    LOG_CLASS_INFO("<Finish> Operation Cycle for Operation mode %" PRId32 "=(%s)",
                   cache.currentOperationMode.id,
                   cache.currentOperationMode.name
    );
}


std::optional<IRoutine<acousea_CommunicationPacket>*> NodeOperationRunner::findRoutine(
    const uint8_t bodyTag, const uint8_t payloadTag) const
{
    const auto bodyIt = routines.find(bodyTag);
    if (bodyIt == routines.end())
    {
        return std::nullopt;
    }
    const auto& packetBodyRoutinesMap = bodyIt->second;

    const auto routineIt = packetBodyRoutinesMap.find(payloadTag);
    if (routineIt == packetBodyRoutinesMap.end() || routineIt->second == nullptr)
    {
        return std::nullopt;
    }
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

    auto routineOpt = findRoutine(
        acousea_CommunicationPacket_report_tag,acousea_ReportBody_statusPayload_tag
    );

    if (!routineOpt.has_value())
    {
        LOG_CLASS_ERROR("Report routine for reportTag = %d and payloadTag = %d not found. Cannot send report.",
                        acousea_CommunicationPacket_report_tag, acousea_ReportBody_statusPayload_tag
        );

        return;
    }

    auto resultCommunicationPacket = executeRoutine(routineOpt.value(), std::nullopt, portType, 0, false);
    if (resultCommunicationPacket.has_value())
    {
        sendResponsePacket(portType, currentNodeConfiguration->localAddress, std::nullopt,
                           resultCommunicationPacket.value());
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
        for (auto& inPacket : packets)
        {
            std::optional<acousea_CommunicationPacket> optOutputPacket = processPacket(portType, inPacket);
            if (!optOutputPacket.has_value())
            {
                LOG_CLASS_INFO("No response packet generated for received packet with ID %" PRId32 ". Continuing...",
                               inPacket.packetId);
                continue;
            }
            sendResponsePacket(portType, localAddress, inPacket, optOutputPacket.value());
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

    LOG_CLASS_INFO("Received Packet from %" PRId32 " of type %s",
                   packet.routing.sender,
                   bodyAndPayloadTagToCString(packet.which_body, getPayloadTag(packet))

    );

    const auto bodyTag = packet.which_body;
    const auto payloadTag = getPayloadTag(packet);
    auto routineOpt = findRoutine(bodyTag, payloadTag);
    if (!routineOpt.has_value())
    {
        LOG_CLASS_ERROR("Routine not found for Body %d and Payload %d. Building error packet...",
                        bodyTag, payloadTag);
        return buildErrorPacket("Routine not found.");
    }
    IRoutine<acousea_CommunicationPacket>*& routine = *routineOpt;
    return executeRoutine(routine, packet, portType,
                          PendingRoutines<0>::MAX_ATTEMPTS, isRequeueAllowed(bodyTag, payloadTag)
    );
}


std::optional<acousea_CommunicationPacket> NodeOperationRunner::executeRoutine(
    IRoutine<acousea_CommunicationPacket>*& routine,
    const std::optional<acousea_CommunicationPacket>& optInputPacket,
    const IPort::PortType portType,
    const uint8_t remainingAttempts,
    const bool requeueAllowed
)
{
    const Result<acousea_CommunicationPacket> result = routine->execute(optInputPacket);
    acousea_CommunicationPacket outputPacket = acousea_CommunicationPacket_init_default;

    if (result.isPending() && (remainingAttempts <= 0 || !requeueAllowed))
    {
        LOG_CLASS_WARNING("%s[NO REQUEUE ALLOWED] => incomplete with message: %s", routine->routineName.c_str(),
                          result.getError());

        return std::nullopt;
    }

    if (result.isPending() && remainingAttempts > 0 && requeueAllowed)
    {
        pendingRoutines.add({routine, optInputPacket, static_cast<uint8_t>(remainingAttempts - 1), portType});
        LOG_CLASS_WARNING("%s [REQUEUED] => incomplete with message: %s", routine->routineName.c_str(),
                          result.getError());
        return std::nullopt;
    }

    if (result.isError())
    {
        LOG_CLASS_ERROR("%s => failed with message: %s", routine->routineName.c_str(), result.getError());
        if (!optInputPacket.has_value())
        {
            LOG_CLASS_INFO("%s: Cannot send error packet, there was no original packet.", routine->routineName.c_str());
            return std::nullopt;
        }

        return buildErrorPacket(result.getError());
    }
    LOG_CLASS_INFO("%s => succeeded.", routine->routineName.c_str());
    return result.getValueConst();
}


void NodeOperationRunner::runPendingRoutines()
{
    // First run incomplete routines
    while (auto entryOpt = pendingRoutines.next())
    {
        auto& [routinePtr, inputPacket, attempts, portResponseTo] = *entryOpt;
        LOG_CLASS_INFO("Re-attempting pending routine %s with %d attempts left.", routinePtr->routineName.c_str(),
                       attempts);

        executeRoutine(routinePtr, inputPacket, portResponseTo, attempts,
                       inputPacket.has_value()
                           ? isRequeueAllowed(inputPacket->which_body, getPayloadTag(*inputPacket))
                           : false
        );
    }
}


void NodeOperationRunner::sendResponsePacket(const IPort::PortType portType,
                                             const uint8_t& localAddress,
                                             const std::optional<acousea_CommunicationPacket>& optInputPacket,
                                             acousea_CommunicationPacket& outputPacket) const
{
    LOG_CLASS_INFO("Sending response packet with type %s through %s from local address %d ...",
                   bodyAndPayloadTagToCString(outputPacket.which_body, getPayloadTag(outputPacket)),
                   IPort::portTypeToCString(portType),
                   localAddress
    );

    if (optInputPacket.has_value())
    {
        LOG_CLASS_INFO("Setting routing info in response packet...");
        const auto inputPacket = *optInputPacket;
        const uint8_t destination = inputPacket.has_routing ? inputPacket.routing.sender : Router::originAddress;
        outputPacket.packetId = inputPacket.packetId;
        outputPacket.has_routing = true;
        outputPacket.routing = acousea_RoutingChunk_init_default;
        outputPacket.routing.sender = localAddress;
        outputPacket.routing.receiver = destination;
    }

    if (const auto sendOk = router.from(localAddress).through(portType).send(outputPacket); !sendOk)
    {
        LOG_CLASS_ERROR(": Failed to send response packet through %s", IPort::portTypeToCString(portType));
    }
}

//---------------------------------------------- Helpers ------------------------------------------------//

Result<acousea_OperationMode> NodeOperationRunner::searchForOperationMode(const uint8_t modeId) const
{
    if (!currentNodeConfiguration.has_value())
    {
        return RESULT_FAILUREF(acousea_OperationMode, "Node configuration not loaded.");
    }

    if (!currentNodeConfiguration->has_operationModesModule
        || currentNodeConfiguration->operationModesModule.modes_count == 0)
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
    const uint8_t modeId, const IPort::PortType portType) const
{
    if (!currentNodeConfiguration.has_value())
    {
        return RESULT_FAILUREF(acousea_ReportingPeriodEntry, "Node configuration not loaded.");
    }
    const acousea_ReportingPeriodEntry* entries = nullptr;
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
            if (!currentNodeConfiguration->has_gsmMqttModule ||
                currentNodeConfiguration->gsmMqttModule.entries_count == 0)
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
