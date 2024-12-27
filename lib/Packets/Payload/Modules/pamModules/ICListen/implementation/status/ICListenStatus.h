#ifndef ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H
#define ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H


#include <ctime>
#include <vector>
#include <cstdint>
#include "../../../../SerializableModule.h"

class ICListenStatus : public SerializableModule {
public:
    ICListenStatus(int unitStatus, int batteryStatus, double batteryPercentage,
                   double temperature, double humidity, std::time_t timestamp);

    static ICListenStatus createDefault();

    static ICListenStatus fromBytes(const std::vector<uint8_t> &data);

    // Constructor de movimiento
    ICListenStatus(ICListenStatus &&other) noexcept;

    // Operador de asignación por movimiento
    ICListenStatus &operator=(ICListenStatus &&other) noexcept;

    // Constructor y operador de copia eliminados
    ICListenStatus(const ICListenStatus &) noexcept;

    ICListenStatus &operator=(const ICListenStatus &) noexcept;

public:
    const int unitStatus;
    const int batteryStatus;
    const double batteryPercentage;
    const double temperature;
    const double humidity;
    const std::time_t timestamp;

private:
    static std::vector<uint8_t> serializeValues(int unitStatus, int batteryStatus, double batteryPercentage,
                                                double temperature, double humidity, std::time_t timestamp);

};


#endif //ACOUSEA_MKR1310_NODES_ICLISTENSTATUS_H
