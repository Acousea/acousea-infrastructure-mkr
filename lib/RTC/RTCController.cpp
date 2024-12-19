#include "RTCController.h"

RTCController::RTCController() {}

void RTCController::init() {
    rtc.begin();
}

void RTCController::syncTime(unsigned long gpsTime) {
    rtc.setEpoch(gpsTime);
}

uint32_t RTCController::getEpoch() {
    return rtc.getEpoch();
}

void RTCController::setEpoch(unsigned long epoch) {
    rtc.setEpoch(epoch);
}
