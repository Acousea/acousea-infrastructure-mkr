#ifndef ACOUSEA_MKR1310_NODES_AMBIENTMODULE_H
#define ACOUSEA_MKR1310_NODES_AMBIENTMODULE_H

#include "Payload/Modules/SerializableModule.h"


class AmbientModule : public SerializableModule {
public:
    static AmbientModule from(int temperature, int humidity);

    static AmbientModule from(const std::vector<uint8_t> &value);

    [[nodiscard]] int getTemperature() const;

    [[nodiscard]] int getHumidity() const;

private:
    explicit AmbientModule(const std::vector<uint8_t> &value);

    AmbientModule(int temperature, int humidity);

private:
    int temperature;
    int humidity;

};


#endif //ACOUSEA_MKR1310_NODES_AMBIENTMODULE_H
