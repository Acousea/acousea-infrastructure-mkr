#ifndef MKRGPS_H
#define MKRGPS_H

#ifdef ARDUINO

#include <Wire.h>
#include <ctime> // Or <time.h> depending on the environment
#include <Arduino_MKRGPS.h>

#include "ClassName.h"
#include "IGPS.h"
#include "Logger/Logger.h"

class MKRGPS : public IGPS
{
    CLASS_NAME(MKRGPS)
public:
    bool init() override;
    GPSLocation read() override;
    unsigned long getTimestamp() override;
    void wakeup() override;
    void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) override;

protected:
    float latitude = 0.0;
    float longitude = 0.0;
    const int GNSS_WAKEUP_PIN = 4;
    const unsigned long GNSS_MAX_FIX_TIME_MS = 900000; // 15 minutos
    const unsigned long GNSS_WAIT_TIME_MS = 120000;    // 2 minutos
};

#endif // ARDUINO
#endif // MKRGPS_H
