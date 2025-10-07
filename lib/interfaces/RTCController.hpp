#ifndef ACOUSEA_RTC_HPP
#define ACOUSEA_RTC_HPP

#include <stdint.h>

class RTCController {
public:
    virtual ~RTCController() = default;

    virtual void init() = 0;
    virtual void syncTime(unsigned long gpsTime) = 0;
    virtual uint32_t getEpoch() = 0;
    virtual void setEpoch(unsigned long epoch) = 0;
};


#endif //ACOUSEA_RTC_HPP
