#ifndef PINGROUTINE_H
#define PINGROUTINE_H

#include "IRoutine.h"
#include "Packet.h"


class PingRoutine : public IRoutine {

public:
    PingRoutine() {}

    Packet execute(const Packet& packet) override {
        Packet modifiedPacket = packet; // Crear una copia del paquete
        // Invert the sender and recipient addresses
        modifiedPacket.setRecipientAddress(modifiedPacket.getSenderAddress()); // Modificar la dirección
        modifiedPacket.setSenderAddress(Packet::Address::DRIFTER); // Modificar la dirección                
        return modifiedPacket; // Devolver el paquete modificado
    }
};

#endif // PINGROUTINE_H
