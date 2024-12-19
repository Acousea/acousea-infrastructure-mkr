#ifndef MOCKGPS_H
#define MOCKGPS_H

#include "GPS/IGPS.h"
#include <Arduino.h>

class MockGPS : public IGPS {
public:
    MockGPS(float startLat, float startLon, float moveRate);

    bool init() override;
    GPSLocation read() override;
    unsigned long getTimestamp() override;
    void wakeup() override;
    void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) override;

private:
    float latitude;
    float longitude;
    float moveRate;
};

#endif // MOCKGPS_H
