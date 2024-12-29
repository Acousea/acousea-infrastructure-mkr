#ifndef ACOUSEA_MKR1310_NODES_RTCMODULE_H
#define ACOUSEA_MKR1310_NODES_RTCMODULE_H

#include <ctime>
#include "Payload/Modules/SerializableModule.h"

class RTCModule : public SerializableModule {
public:
    
    static RTCModule from(std::time_t currentTime);

    
    static RTCModule from(const std::vector<uint8_t>& value);

    
    [[nodiscard]] std::time_t getCurrentTime() const;

private:
    
    explicit RTCModule(const std::vector<uint8_t>& value);

    
    explicit RTCModule(std::time_t currentTime);

    
    static std::vector<uint8_t> serializeValue(std::time_t currentTime);

    static uint64_t timeTToUint64(std::time_t value);

    static std::time_t uint64ToTimeT(uint64_t value);

private:
    std::time_t currentTime; 
};

#endif 
