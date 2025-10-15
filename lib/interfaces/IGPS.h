#ifndef IGPS_H
#define IGPS_H

// Struct to store the GPS data
typedef struct {
    float latitude;
    float longitude;
} GPSLocation;


class IGPS
{
public:
    virtual bool init() = 0;
    virtual GPSLocation read() = 0;
    virtual unsigned long getTimestamp() = 0;
    virtual void wakeup() = 0;
    virtual void calculateTrajectory(float targetLat, float targetLon, float &distance, float &bearing) = 0;
    virtual ~IGPS() = default;
    static void HaverSine(float lat1, float lon1, float lat2, float lon2, float &distance_m, float &bearing_deg);

};

#endif // IGPS_H
