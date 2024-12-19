#ifndef RTC_CONTROLLER_H
#define RTC_CONTROLLER_H

#include "GPS/IGPS.h"
#include <RTCZero.h>

class RTCController {
public:
    RTCController(IGPS* gps);

    void init();

    void syncTime();

    uint32_t getEpoch();

    void setEpoch(unsigned long epoch);

private:
    RTCZero rtc;
    IGPS* gps;
};

#endif // RTC_CONTROLLER_H
