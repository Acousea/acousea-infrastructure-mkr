#include <Arduino.h>

class GPSUtils {
public:
    static void HaverSine(double lat1, double lon1, double lat2, double lon2, 
                          double &distance_m, double &bearing_deg) {
        double ToRad = PI / 180.0;
        double R_km = 6371;
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

    static void splitFloat(double num, int32_t &ipart, uint32_t &fpart) {
        ipart = int32_t(num);
        if (num >= 0) fpart = uint32_t((num - ipart) * 1000000);
        else fpart = uint32_t((ipart - num) * 1000000);
    }
};
