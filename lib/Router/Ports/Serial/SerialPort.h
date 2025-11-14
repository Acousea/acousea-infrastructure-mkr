#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#ifdef PLATFORM_ARDUINO

#include <Arduino.h>

#include "ClassName.h"
#include "Ports/IPort.h"
#include "PacketQueue/PacketQueue.hpp"

class SerialPort final : public IPort
{
    CLASS_NAME(SerialPort)

private:
    Uart& serialPort;
    int baudRate;
    PacketQueue& packetQueue_;
    static constexpr uint8_t kSOF = 0x2A; // Start Of Frame
    static constexpr size_t kMaxBuf = 4096; // cota de seguridad del RX buffer

public:
    virtual ~SerialPort() = default;

    // Constructor
    SerialPort(Uart& serialPort, int baudRate, PacketQueue& packetQueue);

    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    bool send(const uint8_t* data, size_t length) override;

    // Comprueba si hay suficientes datos disponibles para un paquete
    bool available() override;

    // Lee datos y construye un paquete
    uint16_t readInto(uint8_t* buffer, uint16_t maxSize) override;
    bool sync() override;
};

#endif // ARDUINO

#endif // SERIAL_PORT_H
