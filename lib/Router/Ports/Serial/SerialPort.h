#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H


#include <Arduino.h>
#include "Ports/IPort.h"

class SerialPort : public IPort {
private:
    Uart *serialPort;
    int baudRate;

public:
    SerialPort(Uart *serialPort, int baudRate);

    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    void send(const std::vector<uint8_t> &data) override;

    // Comprueba si hay suficientes datos disponibles para un paquete
    bool available() override;

    // Lee datos y construye un paquete
    std::vector<std::vector<uint8_t>> read() override;

};

#endif // SERIAL_PORT_H
