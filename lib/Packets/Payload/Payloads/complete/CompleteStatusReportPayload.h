#ifndef ACOUSEA_MKR1310_NODES_COMPLETESTATUSREPORTPAYLOAD_H
#define ACOUSEA_MKR1310_NODES_COMPLETESTATUSREPORTPAYLOAD_H

#include "Payload/Payload.h"
#include "Payload/Modules/pamModules/PamModule.h"
#include "Payload/Modules/implementation/ambient/AmbientModule.h"
#include "Payload/Modules/implementation/battery/BatteryModule.h"
#include "Payload/Modules/implementation/location/LocationModule.h"
#include "Payload/Modules/implementation/storage/StorageModule.h"

class CompleteStatusReportPayload : public Payload {
public:
    CompleteStatusReportPayload(const BatteryModule &battery,
                                const AmbientModule &ambient,
                                const LocationModule &location,
                                const StorageModule &storage,
                                const PamModule &pamModule);

    // Devuelve el tamaño total en bytes del payload
    [[nodiscard]] uint16_t getBytesSize() const override;

    // Serializa el payload a un vector de bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

private:
    BatteryModule battery;
    AmbientModule ambient;
    LocationModule location;
    StorageModule storage;
    PamModule pamModule;

    // Método auxiliar para insertar bytes de un módulo en el buffer
    static void appendModuleBytes(std::vector<uint8_t> &buffer, const std::vector<uint8_t> &moduleBytes);
};

#endif //ACOUSEA_MKR1310_NODES_COMPLETESTATUSREPORTPAYLOAD_H
