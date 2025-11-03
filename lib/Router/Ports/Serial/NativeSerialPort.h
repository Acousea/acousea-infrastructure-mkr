#ifndef NATIVE_SERIAL_PORT_H
#define NATIVE_SERIAL_PORT_H

#if defined(PLATFORM_NATIVE) && !defined(UNIT_TESTING)

#include <string>
#include <vector>
#include <optional>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "Ports/IPort.h"
#include "ClassName.h"

class NativeSerialPort final : public IPort
{
    CLASS_NAME(NativeSerialPort)

private:
    std::string devicePath;
    int baudRate;
    int fd{-1};
    std::vector<uint8_t> rxBuffer;

    static constexpr uint8_t kSOF = 0x2A; // Start Of Frame
    static constexpr size_t kMaxBuf = 4096; // cota de seguridad del RX buffer

public:
    NativeSerialPort(std::string devPath, int baud)
        : IPort(PortType::SerialPort),
          devicePath(std::move(devPath)),
          baudRate(baud)
    {
    }

    // Inicialización del puerto serial
    void init() override;

    // Envía un paquete serializado
    bool send(const std::vector<uint8_t>& data) override;

    // ¿Hay datos disponibles en el puerto?
    bool available() override;

    // Lee datos y construye paquetes [SOF][LEN][PAYLOAD]
    std::vector<std::vector<uint8_t>> read() override;

private:
    void closeIfOpen();
    bool configurePort();
    static std::optional<speed_t> toSpeed(int baud);
};

#endif // PLATFORM_NATIVE

#endif // NATIVE_SERIAL_PORT_H
