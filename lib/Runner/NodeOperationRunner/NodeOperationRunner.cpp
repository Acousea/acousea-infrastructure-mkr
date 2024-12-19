#include "NodeOperationRunner.h"

NodeOperationRunner::NodeOperationRunner(IDisplay *display, Router *router,
                                         const std::map<OperationCode::Code, IRoutine<VoidType> *> &routines,
                                         const NodeConfigurationRepository &nodeConfigurationRepository) : IRunnable(display),
                                                                                                           router(router), routines(routines), nodeConfigurationRepository(nodeConfigurationRepository) {
    cache = {0, 0, {0, 0}};
}

void NodeOperationRunner::init() {
    display->print("Initializing with configuration...");
    nodeConfiguration.emplace(nodeConfigurationRepository.getNodeConfiguration());
    nodeConfiguration->print();
    cache.currentOperationMode = nodeConfiguration->getOperationGraphModule()->getGraph().begin()->first;
}

void NodeOperationRunner::run() {
    display->print("Running Working Mode...");
    router->readPorts();
    display->print("Ports relayed...");
    checkIfMustTransition();
    runRoutines();
    cache.cycleCount++;
}

void NodeOperationRunner::stop() {
    display->print("Stopping Working Mode...");

    // Specific cleanup or stop code for drifting mode
}

void NodeOperationRunner::checkIfMustTransition() {
    auto maxDuration = nodeConfiguration->getOperationGraphModule()->getGraph().at(
            cache.currentOperationMode).duration;
    if (cache.cycleCount >= maxDuration) {
        cache.cycleCount = 0;
        cache.currentOperationMode = nodeConfiguration->getOperationGraphModule()->getGraph().at(
                cache.currentOperationMode).nextMode;
    }
}

void NodeOperationRunner::runRoutines() {
    auto reportingConfiguration = nodeConfiguration->getIridiumModule()->getConfigurations().at(
            cache.currentOperationMode);

    auto currentMinute = millis() / 60000;
    if (currentMinute - cache.lastReportMinute.sbd >= reportingConfiguration.getPeriod()) {
        IRoutine<VoidType> *routine = nullptr;

        switch (reportingConfiguration.getReportType()) {
            case ReportingConfiguration::ReportType::COMPLETE:
                routine = routines.find(OperationCode::Code::COMPLETE_STATUS_REPORT)->second;
                break;
            case ReportingConfiguration::ReportType::BASIC:
                routine = routines.find(OperationCode::Code::BASIC_STATUS_REPORT)->second;
                break;
            case ReportingConfiguration::ReportType::SUMMARY:
//                    routine = configurationRoutines.find(SummaryReportRoutine::getClassNameString())->second;
                break;
            default:
                ErrorHandler::handleError("Invalid report type");
                return;
        }

        if (!routine) {
            ErrorHandler::handleError(
                    NodeOperationRunner::getClassNameString() + "Routine" + BasicSummaryReportRoutine::getClassNameString() +
                    " not found");
            return;
        }

        auto result = routine->execute();
        if (result.isSuccess()) {
            router->sender().sendSBD(result.getValue());
        }

        cache.lastReportMinute.sbd = currentMinute;
    }

//        reportingConfiguration = nodeConfiguration->getLoraModule()->getReportingPeriods().at(cache.currentOperationMode);
//        if (currentMinute - cache.lastReportMinute.sbd >= reportingConfiguration) {
//            Packet reportPacket = PacketBuilder::buildSummaryReportPacket();
//            router->sendLoRa(reportPacket);
//            cache.lastReportMinute.sbd = currentMinute;
//        }
}
