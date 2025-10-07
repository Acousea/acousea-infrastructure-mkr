#ifndef MOCK_SERIAL_PORT_H
#define MOCK_SERIAL_PORT_H

#include "Ports/IPort.h"
#include "Logger/Logger.h"

class MockSerialPort : public IPort {

public:
    MockSerialPort();

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
