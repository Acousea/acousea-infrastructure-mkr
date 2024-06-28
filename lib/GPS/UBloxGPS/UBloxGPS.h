#ifndef UBLOXGNSS_H
#define UBLOXGNSS_H

#include "../IGPS.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>

/*
 * !!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!
 * FIXME: Fails even with battery connected -> must buy button cell battery (probably will work with it) 
 * !!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!
**/
class UBloxGNSS : public IGPS {
public:
    bool init() override;
    GPSLocation read() override;
    unsigned long getTimestamp() override;
    void wakeup() override;
    void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) override;

private:
    float latitude;
    float longitude;    
    SFE_UBLOX_GNSS myGNSS;
    const int GNSS_WAKEUP_PIN = 4;
    const unsigned long GNSS_MAX_FIX_TIME_MS = 900000; // 15 minutos
    const unsigned long GNSS_WAIT_TIME_MS = 120000;    // 2 minutos
};

#endif // UBLOXGNSS_H
