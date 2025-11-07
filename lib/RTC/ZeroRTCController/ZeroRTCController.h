#ifndef ACOUSEA_ZERO_RTC_CONTROLLER_H
#define ACOUSEA_ZERO_RTC_CONTROLLER_H

#ifdef PLATFORM_ARDUINO
#include "RTCController.hpp"

class ZeroRTCController final : public RTCController {
public:
    ZeroRTCController();

    void init();

    void syncTime(unsigned long gpsTime);

    uint32_t getEpoch();

    void setEpoch(unsigned long epoch);
};


#endif

#endif // RTC_CONTROLLER_H
