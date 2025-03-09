#ifndef MOCK_RTC_CONTROLLER_H
#define MOCK_RTC_CONTROLLER_H


#include "RTCController.hpp"

class MockRTCController : public RTCController {
public:
    MockRTCController();

    void init() override;
    void syncTime(unsigned long gpsTime) override;
    uint32_t getEpoch() override;
    void setEpoch(unsigned long epoch) override;

private:
    uint32_t simulatedEpoch;
};

#endif // MOCK_RTC_CONTROLLER_H
