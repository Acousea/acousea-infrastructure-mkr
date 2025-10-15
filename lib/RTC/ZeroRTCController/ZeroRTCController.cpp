#ifdef ARDUINO
#include "ZeroRTCController.h"
#include "RTCZero.h"

static RTCZero rtc;

ZeroRTCController::ZeroRTCController()
{
}

void ZeroRTCController::init()
{
    rtc.begin();
}

void ZeroRTCController::syncTime(unsigned long gpsTime)
{
    rtc.setEpoch(gpsTime);
}

uint32_t ZeroRTCController::getEpoch()
{
    return rtc.getEpoch();
}

void ZeroRTCController::setEpoch(unsigned long epoch)
{
    rtc.setEpoch(epoch);
}

#endif // ARDUINO
