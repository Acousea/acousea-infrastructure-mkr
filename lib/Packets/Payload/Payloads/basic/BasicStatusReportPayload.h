#ifndef ACOUSEA_MKR1310_NODES_BASICSTATUSREPORTPAYLOAD_H
#define ACOUSEA_MKR1310_NODES_BASICSTATUSREPORTPAYLOAD_H


#include "Payload/Payload.h"
#include "Payload/Modules/extModules/battery/BatteryModule.h"
#include "Payload/Modules/extModules/location/LocationModule.h"
#include "Payload/Modules/extModules/rtc/RTCModule.h"


class BasicStatusReportPayload : public Payload {
public:
    BasicStatusReportPayload(const BatteryModule &battery,
                             const LocationModule &location, const RTCModule &rtc);

    // Obtiene el tamaño en bytes del payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    // Serializa el payload a un vector de bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const override;


private:
    BatteryModule battery;
    LocationModule location;
    RTCModule rtc;
};

#endif //ACOUSEA_MKR1310_NODES_BASICSTATUSREPORTPAYLOAD_H
