#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#ifdef ARDUINO

#include <Arduino.h>
#include "Ports/IPort.h"
#include "Logger/Logger.h"

class SerialPort : public IPort {
private:
    Uart *serialPort;
    int baudRate;
    std::vector<uint8_t> rxBuffer;
    static constexpr uint8_t kSOF = 0x2A;        // Start Of Frame
    static constexpr size_t  kMaxBuf = 4096;     // cota de seguridad del RX buffer

public:
    SerialPort(Uart *serialPort, int baudRate);

    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    bool send(const std::vector<uint8_t>& data) override;

    // Comprueba si hay suficientes datos disponibles para un paquete
    bool available() override;

    // Lee datos y construye un paquete
    std::vector<std::vector<uint8_t>> read() override;

};

#endif // ARDUINO

#endif // SERIAL_PORT_H
