#include "MockRTCController.h"

MockRTCController::MockRTCController() : simulatedEpoch(0) {}

void MockRTCController::init() {
    // Mock initialization - No actual hardware involved
}

void MockRTCController::syncTime(unsigned long gpsTime) {
    simulatedEpoch = gpsTime;
}

uint32_t MockRTCController::getEpoch() {
    return simulatedEpoch;
}

void MockRTCController::setEpoch(unsigned long epoch) {
    simulatedEpoch = epoch;
}
