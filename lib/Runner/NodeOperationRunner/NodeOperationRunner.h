#ifndef OPERATION_MODE_RUNNER_H
#define OPERATION_MODE_RUNNER_H


#include <utility>
#include "IRunnable.h"
#include "Router.h"
#include "ClassName.h"
#include "Packets/CompleteReportPacket.h"
#include "routines/CompleteSummaryReportRoutine/CompleteSummaryReportRoutine.h"
#include "routines/BasicSummaryReportRoutine/BasicSummaryReportRoutine.h"


/**
 * @brief Working mode class (DRIFTER)
 * The working mode is the third mode of operation, immediatly after the launching mode.
 * In this mode, the drifter has been successfully launched and is in the working state.
 *  - The router is used to send periodic status reports to the backend, including:
 *     - Current location data from the GPS.
 *     - Current battery status and system health.
 *     - AudioDetectionStats data from the IC-Listen sensor. 
 
 *  - The mode an indefinite duration, until the drifter is recovered.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_DRIFTING_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_DRIFTING_SEC` just for testing (It will not be used in real deployment).
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 * 
 *  - It will send periodic commands to the IC-Listen sensor to start/stop logging data. (commands sent to PI3 API)
 */

class NodeOperationRunner : public IRunnable {

private:
    Router *router;
    // Map of string keys and IRoutine pointers
    std::map<OperationCode::Code, IRoutine<VoidType> *> routines;


private: // Variables to establish periods
    std::optional<NodeConfiguration> nodeConfiguration = std::nullopt;
    NodeConfigurationRepository nodeConfigurationRepository;
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

    NodeOperationRunner(IDisplay *display, Router *router,
                        const std::map<OperationCode::Code, IRoutine<VoidType> *> &routines,
                        const NodeConfigurationRepository &nodeConfigurationRepository
    );

    void init() override;

    void run() override;


    void stop() override;

private:
    void checkIfMustTransition();

    void runRoutines();

};

#endif // OPERATION_MODE_RUNNER_H
