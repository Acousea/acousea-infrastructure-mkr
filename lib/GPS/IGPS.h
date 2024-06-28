#ifndef IGPS_H
#define IGPS_H

#include <cmath>
#include <math.h>   

// Struct to store the GPS data
typedef struct 
{
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
    virtual ~IGPS() {}
    static void HaverSine(float lat1, float lon1, float lat2, float lon2, float &distance_m, float &bearing_deg)
    {
        double ToRad = M_PI / 180.0;
        double R_km = 6371; // radius earth in Km

        double lat1_rad = lat1 * ToRad;
        double lat2_rad = lat2 * ToRad;
        double dLat_rad = (lat2 - lat1) * ToRad;
        double dLon_rad = (lon2 - lon1) * ToRad;

        double a = sin(dLat_rad / 2);
        double b = sin(dLon_rad / 2);
        a = a * a + cos(lat1_rad) * cos(lat2_rad) * b * b;

        double c = 2 * atan2(sqrt(a), sqrt(1 - a));
        distance_m = R_km * c * 1000;

        double bearing_rad = atan2f(sin(dLon_rad) * cos(lat2_rad),
                                    (cos(lat1_rad) * sin(lat2_rad) -
                                     sin(lat1_rad) * cos(lat2_rad) * cos(dLon_rad)));
        bearing_deg = fmod(bearing_rad / ToRad + 360, 360);
    }

protected:

};

#endif // IGPS_H
