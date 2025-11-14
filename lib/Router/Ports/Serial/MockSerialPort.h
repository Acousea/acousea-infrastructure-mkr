#ifndef MOCK_SERIAL_PORT_H
#define MOCK_SERIAL_PORT_H

#include "ClassName.h"
#include "Ports/IPort.h"


class MockSerialPort final : public IPort
{
    CLASS_NAME(MockSerialPort)

public:

    explicit MockSerialPort();
    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    bool send(const uint8_t* data, size_t length) override;

    // Comprueba si hay suficientes datos disponibles para un paquete
    bool available() override;

    // Lee datos y construye un paquete
    bool sync() override;
};

#endif // SERIAL_PORT_H
