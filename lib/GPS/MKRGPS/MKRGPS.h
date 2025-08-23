#ifndef MKRGPS_H
#define MKRGPS_H

#ifdef ARDUINO

#include <Wire.h>
#include "../IGPS.h"
#include "Arduino_MKRGPS.h"
#include "Logger/Logger.h"

class MKRGPS : public IGPS
{
public:
    bool init() override;
    GPSLocation read() override;
    unsigned long getTimestamp() override;
    void wakeup() override;
    void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) override;

protected:
    float latitude;
    float longitude;
    const int GNSS_WAKEUP_PIN = 4;
    const unsigned long GNSS_MAX_FIX_TIME_MS = 900000; // 15 minutos
    const unsigned long GNSS_WAIT_TIME_MS = 120000;    // 2 minutos
};

#endif // ARDUINO
#endif // MKRGPS_H
