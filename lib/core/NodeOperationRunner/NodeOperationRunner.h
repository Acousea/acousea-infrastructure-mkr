#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include "IRunnable.h"
#include "Router.h"
#include "IRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Result.h"
#include "time/getMillis.hpp"

#include "PendingRoutines/PendingRoutines.hpp"

#include <deque>
#include <optional>

/**
 * @brief Class that runs the operation modes of the node
 */
class NodeOperationRunner final : public IRunnable
{
public:
    CLASS_NAME(NodeOperationRunner)

    NodeOperationRunner(Router& router,
                        const NodeConfigurationRepository& nodeConfigurationRepository,
                        const std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routines);

    void init() override;

    void run() override;
    [[nodiscard]] std::optional<IRoutine<acousea_CommunicationPacket>*> findRoutine(
        uint8_t bodyTag, uint8_t payloadTag) const;

private:

    Router& router;
    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines;

    // -------------------------
    // Lista de rutinas incompletas (tamaño fijo)
    // -------------------------
    PendingRoutines<3> pendingRoutines;

    NodeConfigurationRepository nodeConfigurationRepository;
    std::optional<acousea_NodeConfiguration> currentNodeConfiguration = std::nullopt;

    // Struct to store the current currentOperationMode and cycle count
    struct Cache
    {
        acousea_OperationMode currentOperationMode;
        uint8_t cycleCount;

        struct
        {
            unsigned long sbd;
            unsigned long lora;
            unsigned long gsmMqtt;
        } lastReportMinute;
    } cache{};


    void tryTransitionOpMode();
    void tryReport(IPort::PortType port, unsigned long& lastMinute, unsigned long currentMinute);


    void processReportingRoutines();
    void processIncomingPackets(const uint8_t& localAddress);
    void runPendingRoutines();
    void sendResponsePacket(IPort::PortType portType, const uint8_t& localAddress,
                            const std::optional<acousea_CommunicationPacket>& optInputPacket,
                            acousea_CommunicationPacket& outputPacket) const;


    std::optional<acousea_CommunicationPacket> processPacket(IPort::PortType portType,
                                                             const acousea_CommunicationPacket& packet);
    std::optional<acousea_CommunicationPacket> executeRoutine(
        IRoutine<acousea_CommunicationPacket>*& routine,
        const std::optional<acousea_CommunicationPacket>& optInputPacket,
        IPort::PortType portType,
        uint8_t remainingAttempts,
        bool requeueAllowed
    );

    [[nodiscard]] Result<acousea_OperationMode> searchForOperationMode(uint8_t modeId) const;
    [[nodiscard]] Result<acousea_ReportingPeriodEntry> getReportingEntryForCurrentOperationMode(
        uint8_t modeId, IPort::PortType portType) const;


#ifdef UNIT_TESTING
#warning "UNIT_TESTING is defined"
    friend class NodeOperationRunnerTest_InitLoadsConfigurationCorrectly_Test;
    friend class NodeOperationRunnerTest_SearchForOperationModeReturnsCorrectMode_Test;
    friend class NodeOperationRunnerTest_SearchForOperationModeFailsWhenNotFound_Test;
    friend class NodeOperationRunnerTest_FindRoutineReturnsCorrectPointer_Test;
    friend class NodeOperationRunnerTest_ExecuteRoutineReturnsSuccessPacket_Test;
    friend class NodeOperationRunnerTest_ExecuteRoutineRequeuesWhenPending_Test;
    friend class NodeOperationRunnerTest_ProcessPacketReturnsErrorPacketWhenNoRoutine_Test;
    friend class NodeOperationRunnerTest_TryTransitionChangesModeWhenDurationReached_Test;
    friend class NodeOperationRunnerTest_GetReportingEntryReturnsCorrectEntry_Test;
    friend class NodeOperationRunnerTest_RunPendingRoutinesProcessesAll_Test;
    friend class NodeOperationRunnerTest_ProcessIncomingPacketsSendsResponsesThroughRouter_Test;
#endif

};

#endif // OPERATION_MODE_RUNNER_H
