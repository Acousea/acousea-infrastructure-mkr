#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include "IRunnable.h"
#include "Router.h"
#include "routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h"
#include "routines/BasicStatusReportRoutine/BasicStatusReportRoutine.h"
#include "Result/Result.h"
#include "time/getMillis.hpp"

#include <deque>
#include <optional>

template <size_t MAX_ROUTINES, size_t MAX_ATTEMPTS>
struct PendingRoutines
{
    using Packet = acousea_CommunicationPacket;
    using Routine = IRoutine<Packet>;

    struct Entry
    {
        uint8_t routineTag;
        std::optional<Packet> input{};
        uint8_t attempts = 0;
        IPort::PortType portResponseTo;
    };

    // FIFO
    std::deque<Entry> buffer;

    void add(Entry e)
    {
        if (buffer.size() >= MAX_ROUTINES)
        {
            buffer.pop_front();
        }
        buffer.push_back(e);
    }

    // sacar la más antigua
    std::optional<Entry> next()
    {
        if (buffer.empty()) return std::nullopt;
        Entry e = buffer.front();
        buffer.pop_front();
        return e;
    }

    [[nodiscard]] bool empty() const { return buffer.empty(); }
    [[nodiscard]] size_t size() const { return buffer.size(); }
};


/**
 * @brief Class that runs the operation modes of the node
 */
class NodeOperationRunner : public IRunnable
{
private:
    Router& router;
    std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*> routines;

    // -------------------------
    // Lista de rutinas incompletas (tamaño fijo)
    // -------------------------
    PendingRoutines<5, 3> pendingRoutines;

    NodeConfigurationRepository nodeConfigurationRepository;
    std::optional<acousea_NodeConfiguration> currentNodeConfiguration = std::nullopt;

private:
    // Struct to store the current currentOperationMode and cycle count
    struct Cache
    {
        acousea_OperationModesGraphModule_GraphEntry currentOperationMode;
        uint8_t cycleCount;

        struct
        {
            unsigned long sbd;
            unsigned long lora;
        } lastReportMinute;
    } cache;

public:
    CLASS_NAME(NodeOperationRunner)

    NodeOperationRunner(Router& router,
                        const std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>& routines,
                        const NodeConfigurationRepository& nodeConfigurationRepository
    );


    void init() override;

    void run() override;

private:
    void tryReport(const std::string& moduleType, const acousea_ReportingPeriodEntry* entries, size_t entryCount,
                   IPort::PortType port, unsigned long& lastMinute, unsigned long currentMinute);
    void executeRoutine(
        uint8_t routineTag,
        const std::optional<acousea_CommunicationPacket>& inputPacket, IPort::PortType port, uint8_t destination, bool
        requeueAllowed);

    void processReportingRoutines();
    void processIncomingPackets(const uint8_t& localAddress);
    void runPendingRoutines();

    void processPacket(IPort::PortType portType, const acousea_CommunicationPacket& packet,
                       const uint8_t& localAddress);

    void sendResponsePacket(IPort::PortType portType, const uint8_t& localAddress,
                            const acousea_CommunicationPacket& responsePacket) const;

    [[nodiscard]] static acousea_CommunicationPacket buildErrorPacket(const std::string& errorMessage,
                                                                      const uint8_t& localAddress,
                                                                      const uint8_t& destination);

    void checkIfMustTransition();
    static bool mustReport(unsigned long currentMinute, unsigned long reportingPeriod, unsigned long lastReportMinute);

    [[nodiscard]] Result<acousea_OperationModesGraphModule_GraphEntry> searchForOperationMode(uint8_t modeId) const;
    static Result<acousea_ReportingPeriodEntry> searchForReportingEntry(uint8_t modeId,
                                                                        const acousea_ReportingPeriodEntry* entries,
                                                                        size_t entryCount);

    void runReportRoutine(IPort::PortType portType, unsigned long currentMinute, unsigned long& lastReportMinute);
};

#endif // OPERATION_MODE_RUNNER_H
