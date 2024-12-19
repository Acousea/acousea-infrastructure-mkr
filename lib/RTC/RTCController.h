#ifndef RTC_CONTROLLER_H
#define RTC_CONTROLLER_H

#include "RTCZero.h"

class RTCController {
public:
    RTCController();

    void init();

    void syncTime(unsigned long gpsTime);

    uint32_t getEpoch();

    void setEpoch(unsigned long epoch);

private:
    RTCZero rtc;
};

#endif // RTC_CONTROLLER_H
