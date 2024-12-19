#ifndef ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H
#define ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H


#include "Payload/Modules/SerializableModule.h"

class ICListenStatus : public SerializableModule {
public:
    ICListenStatus(int unitStatus, int batteryStatus, double batteryPercentage,
                   double temperature, double humidity, std::time_t timestamp);

    static ICListenStatus createDefault();

private:
    int unitStatus;
    int batteryStatus;
    double batteryPercentage;
    double temperature;
    double humidity;
    std::time_t timestamp;

    static std::vector<uint8_t> serializeValues(int unitStatus, int batteryStatus, double batteryPercentage,
                                                double temperature, double humidity, std::time_t timestamp);
};


#endif //ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H
