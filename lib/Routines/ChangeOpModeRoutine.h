#ifndef CHANGE_MODE_ROUTINE_H
#define CHANGE_MODE_ROUTINE_H

#include "IRoutine.h"
#include "../OperationManager/OperationManager.h"

class ChangeOperationModeRoutine : public IRoutine {
private:
    OperationManager &operationManager;    

public:
    ChangeOperationModeRoutine(OperationManager &manager) : operationManager(manager) {}

    Packet execute(const Packet &packet) override {
        // Leer el nuevo modo de operación del paquete
        uint8_t newModeByte = packet.getPayload()[0];
        if (newModeByte >= 4) {
            return Packet(packet.getOpCode(), packet.getSwappedAddresses(), {Packet::ResponseCode::NACKNOWLEDGE});
        }
        OPERATION_MODE newMode = static_cast<OPERATION_MODE>(newModeByte);
        operationManager.setMode(newMode);
        // Devolver un paquete de respuesta o un paquete de confirmación
        return Packet(packet.getOpCode(), packet.getSwappedAddresses(), {Packet::ResponseCode::ACKNOWLEDGE});
    }
};

#endif // CHANGE_MODE_ROUTINE_H
