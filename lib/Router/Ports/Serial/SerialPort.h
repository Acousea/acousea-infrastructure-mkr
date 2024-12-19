#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H


#include "Ports/IPort.h"

class SerialPort : public IPort {
private:
    Uart *serialPort;
    int baudRate;
    static constexpr size_t MAX_PACKET_BUFFER = 512;

public:
    SerialPort(Uart *serialPort, int baudRate);

    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    void send(const Packet &packet) override;

    // Comprueba si hay suficientes datos disponibles para un paquete
    bool available() override;

    // Lee datos y construye un paquete
    Result<Packet> read() override;
};

#endif // SERIAL_PORT_H
