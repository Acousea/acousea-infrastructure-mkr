#include "RTCController.h"

RTCController::RTCController(IGPS *gps) : gps(gps) {}

void RTCController::init() {
    rtc.begin();
}

void RTCController::syncTime() {
    if (gps->init()) {
        unsigned long gpsTime = gps->getTimestamp();
        rtc.setEpoch(gpsTime);
    }
}

uint32_t RTCController::getEpoch() {
    return rtc.getEpoch();
}

void RTCController::setEpoch(unsigned long epoch) {
    rtc.setEpoch(epoch);
}
