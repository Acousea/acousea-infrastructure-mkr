#ifndef ACOUSEA_MKR1310_NODES_LOCATIONMODULE_H
#define ACOUSEA_MKR1310_NODES_LOCATIONMODULE_H

#include "Payload/Modules/SerializableModule.h"

class LocationModule : public SerializableModule {
public:
    static LocationModule from(float latitude, float longitude);

    static LocationModule from(const std::vector<uint8_t>& value);

    
    [[nodiscard]] float getLatitude() const;

    [[nodiscard]] float getLongitude() const;

private:
    
    explicit LocationModule(const std::vector<uint8_t>& value);

    LocationModule(float latitude, float longitude);
    
    static std::vector<uint8_t> serializeValues(float latitude, float longitude);

private:
    float latitude;  
    float longitude; 
};

#endif 
