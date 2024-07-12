#ifndef RTC_CONTROLLER_H
#define RTC_CONTROLLER_H

#include <RTCZero.h>
#include "IGPS.h"

class RTCController {
public:
    RTCController(IGPS* gps) : gps(gps) {}

    void init() {
        rtc.begin();
    }

    void syncTime() {
        if (gps->init()) {
            unsigned long gpsTime = gps->getTimestamp();
            rtc.setEpoch(gpsTime);
        }
    }

    uint32_t getEpoch() {
        return rtc.getEpoch();
    }

    void setEpoch(unsigned long epoch) {
        rtc.setEpoch(epoch);
    }

private:
    RTCZero rtc;
    IGPS* gps;
};

#endif // RTC_CONTROLLER_H
