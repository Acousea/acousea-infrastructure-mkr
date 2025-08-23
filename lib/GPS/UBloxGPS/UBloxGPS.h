#ifndef UBLOXGNSS_H
#define UBLOXGNSS_H

#ifdef ARDUINO

#include "../IGPS.h"
#include "SparkFun_u-blox_GNSS_Arduino_Library.h"
#include <Logger/Logger.h>


class UBloxGNSS : public IGPS {
public:
    bool init() override;

    GPSLocation read() override;

    unsigned long getTimestamp() override;

    void wakeup() override;

    void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) override;

private:
    SFE_UBLOX_GNSS myGNSS;
    const int GNSS_WAKEUP_PIN = 4;
    float latitude = 0, longitude = 0;
    const unsigned long GNSS_MAX_FIX_TIME_MS = 900000, GNSS_WAIT_TIME_MS = 120000; // 15 minutes, 2 minutes
};


#endif // ARDUINO
#endif // UBLOXGNSS_H
