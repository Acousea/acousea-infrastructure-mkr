#ifndef RTC_CONTROLLER_H
#define RTC_CONTROLLER_H

#ifdef ARDUINO

#include "RTCZero.h"
#include "RTCController.hpp"

class ZeroRTCController : public RTCController {
public:
    ZeroRTCController();

    void init();

    void syncTime(unsigned long gpsTime);

    uint32_t getEpoch();

    void setEpoch(unsigned long epoch);

private:
    RTCZero rtc;
};


#endif

#endif // RTC_CONTROLLER_H
