#ifndef UBLOXGNSS_H
#define UBLOXGNSS_H

#ifdef ARDUINO

#include "IGPS.h"
#include "ClassName.h"


class UBloxGNSS final : public IGPS{
    CLASS_NAME(UBloxGNSS)

public:
    bool config();

    bool init() override;

    GPSLocation read() override;

    unsigned long getTimestamp() override;

    void wakeup() override;

    void calculateTrajectory(float targetLat, float targetLon, float& distance, float& bearing) override;

private:

    const int GNSS_WAKEUP_PIN = 4;
    float latitude = 0, longitude = 0;
    const unsigned long GNSS_MAX_FIX_TIME_MS = 900000, GNSS_WAIT_TIME_MS = 120000, GNSS_MAX_TIMESTAMP_WAIT_TIME_MS = 10000; // 15 minutes, 2 minutes
};


#endif // ARDUINO
#endif // UBLOXGNSS_H
