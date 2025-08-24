#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include "IRunnable.h"
#include "Router.h"
#include "routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h"
#include "routines/BasicStatusReportRoutine/BasicStatusReportRoutine.h"
#include "time/getMillis.hpp"


/**
 * @brief Class that runs the operation modes of the node
 */
class NodeOperationRunner : public IRunnable{
private:
    Router& router;

    std::map<uint8_t, IRoutine<VoidType>*> internalRoutines;
    std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> externalRoutines;

    NodeConfigurationRepository nodeConfigurationRepository;
    std::optional<acousea_NodeConfiguration> currentNodeConfiguration = std::nullopt;

private:
    // Struct to store the current currentOperationMode and cycle count
    struct Cache{
        acousea_OperationModesGraphModule_GraphEntry currentOperationMode;
        uint8_t cycleCount;

        struct{
            unsigned long sbd;
            unsigned long lora;
        } lastReportMinute;
    } cache;

public:
    CLASS_NAME(NodeOperationRunner)

    NodeOperationRunner(Router& router,
                        const std::map<uint8_t, IRoutine<VoidType>*>& internalRoutines,
                        const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>& externalRoutines,
                        const NodeConfigurationRepository& nodeConfigurationRepository
    );

    void init() override;

    void run() override;


    void finish() override;

private:
    [[nodiscard]] acousea_OperationModesGraphModule_GraphEntry searchForOperationMode(uint8_t modeId) const;
    static acousea_ReportingPeriodEntry searchForReportingEntry(uint8_t modeId,
                                                             const acousea_ReportingPeriodEntry* entries,
                                                             size_t entryCount);

    void checkIfMustTransition();

    static bool mustReport(unsigned long currentMinute, unsigned long reportingPeriod, unsigned long lastReportMinute);

    void runRoutines();

    void processIncomingPackets(const uint8_t& localAddress);

    void processPacket(IPort::PortType portType, const acousea_CommunicationPacket& packet,
                       const uint8_t& localAddress);

    void sendResponsePacket(IPort::PortType portType, const uint8_t& localAddress,
                            const acousea_CommunicationPacket& responsePacket) const;

    void reportRoutine(IPort::PortType portType,
                        unsigned long currentMinute,
                        unsigned long& lastReportMinute);
};

#endif // OPERATION_MODE_RUNNER_H
