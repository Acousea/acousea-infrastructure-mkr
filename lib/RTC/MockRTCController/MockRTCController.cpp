#include "MockRTCController.h"

MockRTCController::MockRTCController()
    : baseEpoch(0), baseMillis(getMillis()) {}

void MockRTCController::init() {
    baseEpoch = 0;
    baseMillis = getMillis();
}

void MockRTCController::syncTime(unsigned long gpsTime) {
    baseEpoch = gpsTime;
    baseMillis = getMillis();
}

uint32_t MockRTCController::getEpoch() {
    const unsigned long elapsedMs = getMillis() - baseMillis;
    return baseEpoch + (elapsedMs / 1000);
}

void MockRTCController::setEpoch(unsigned long epoch) {
    baseEpoch = epoch;
    baseMillis = getMillis();
}
