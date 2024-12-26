#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include "IRunnable.h"
#include "Router.h"
#include "routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h"
#include "routines/BasicStatusReportRoutine/BasicStatusReportRoutine.h"


/**
 * @brief Class that runs the operation modes of the node
 */
class NodeOperationRunner : public IRunnable {

private:
    Router &router;

    std::map<OperationCode::Code, IRoutine<VoidType> *> internalRoutines;
    std::map<OperationCode::Code, IRoutine<Packet> *> externalRoutines;

    NodeConfigurationRepository nodeConfigurationRepository;
    std::optional<NodeConfiguration> nodeConfiguration = std::nullopt;

private:
    // Struct to store the current currentOperationMode and cycle count
    struct Cache {
        uint8_t currentOperationMode;
        uint8_t cycleCount;
        struct {
            unsigned long sbd;
            unsigned long lora;
        } lastReportMinute;
    } cache;


public:
    CLASS_NAME(NodeOperationRunner)

    NodeOperationRunner(Router &router,
                        const std::map<OperationCode::Code, IRoutine<VoidType> *> &internalRoutines,
                        const std::map<OperationCode::Code, IRoutine<Packet> *> &externalRoutines,
                        const NodeConfigurationRepository &nodeConfigurationRepository
    );

    void init() override;

    void run() override;


    void finish() override;

private:
    void checkIfMustTransition();

    static bool mustReport(unsigned long currentMinute, unsigned long reportingPeriod, unsigned long lastReportMinute);

    void runRoutines();

    void processIncomingPackets(const Address &localAddress);

    void processPacket(IPort::PortType portType, const Packet &packet, const Address &localAddress);

    void sendResponsePacket(IPort::PortType portType, const Address &localAddress, const Packet &responsePacket) const;

    void processRoutine(const ReportingConfiguration &config, IPort::PortType portType, unsigned long currentMinute,
                        unsigned long &lastReportMinute);
};

#endif // OPERATION_MODE_RUNNER_H
