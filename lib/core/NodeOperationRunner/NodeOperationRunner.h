#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include <map>
#include "IRoutine.h"
#include "IRunnable.h"
#include "Result.h"
#include "Router.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "time/getMillis.hpp"


/**
 * @brief Class that runs the operation modes of the node
 */
class NodeOperationRunner final : public IRunnable
{
public:
    CLASS_NAME(NodeOperationRunner)

    NodeOperationRunner(Router& router,
                        StorageManager& storageManager,
                        NodeConfigurationRepository& nodeConfigurationRepository,
                        const std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>>& routines);

    void init() override;

    void run() override;
    void dumpRoutinesMap() const;

private:
    Router& router;
    NodeConfigurationRepository nodeConfigurationRepository;
    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines{};

    // WARNING: The retry attempts associate packetId -> attempts left, packet IDS must be unique
    // If this cannot be guaranteed, then the packetQueue readOffset_ can be used as unique identifier
    struct RetryEntry
    {
        uint32_t packetId = 0;
        uint8_t attemptsLeft = 0;
        bool inUse = false;
    };

    RetryEntry retryAttemptsLeft_[IPort::MAX_PORT_TYPE_U8 + 1] = {};
    static constexpr uint8_t MAX_RETRIES = 3;


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

private:
    [[nodiscard]] IRoutine<acousea_CommunicationPacket>* findRoutine(
        uint8_t bodyTag, uint8_t payloadTag) const;
    void tryTransitionOpMode();
    void tryReport(IPort::PortType port, unsigned long& lastMinute, unsigned long currentMinute);


    void processReportingRoutines();

    void processNextIncomingPacket();
    void sendResponsePacket(IPort::PortType portType, uint8_t localAddress, uint8_t recipientAddress,
                            acousea_CommunicationPacket* outputPacketPtr);


    acousea_CommunicationPacket* executeRoutine(IRoutine<acousea_CommunicationPacket>* routine,
                                                       acousea_CommunicationPacket* optInputPacket,
                                                       IPort::PortType portType, uint8_t& attemptsLeft);

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
