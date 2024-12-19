#ifndef IPORT_H
#define IPORT_H


#include "Packet.h"
#include "Result/Result.h"

// Definición de la interfaz de comunicador
class IPort {

public:
    enum class PortType {
        LoraPort,
        SBDPort,
        SerialPort
    };

    explicit IPort(PortType type) : type(type) {}

    [[nodiscard]] PortType getType() const {
        return type;
    }

public:
    virtual void init() = 0;

    virtual void send(const Packet &packet) = 0;

    virtual bool available() = 0;

    virtual Result<Packet> read() = 0;

protected:
    PortType type;

};

#endif // IPORT_H
