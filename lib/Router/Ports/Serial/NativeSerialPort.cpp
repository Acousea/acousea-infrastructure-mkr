#ifndef ARDUINO
#include "NativeSerialPort.h"
#include <cstring>
#include <algorithm>

void NativeSerialPort::closeIfOpen() {
    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

std::optional<speed_t> NativeSerialPort::toSpeed(int baud) {
    switch (baud) {
        case 9600:    return B9600;
        case 19200:   return B19200;
        case 38400:   return B38400;
        case 57600:   return B57600;
        case 115200:  return B115200;
#ifdef B230400
        case 230400:  return B230400;
#endif
#ifdef B460800
        case 460800:  return B460800;
#endif
#ifdef B921600
        case 921600:  return B921600;
#endif
        default: return std::nullopt;
    }
}

bool NativeSerialPort::configurePort() {
    termios tio{};
    if (tcgetattr(fd, &tio) != 0) {
        Logger::logError(std::string("NativeSerialPort::configurePort() -> tcgetattr failed: ")
                         + std::strerror(errno));
        return false;
    }

    // Modo "raw"
    cfmakeraw(&tio);

    // 8N1, sin control de flujo
    tio.c_cflag &= ~PARENB;
    tio.c_cflag &= ~CSTOPB;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CS8;
    tio.c_cflag |= CLOCAL | CREAD;
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif

    // No bloqueante por caracteres: VMIN=0, VTIME=1 (100 ms)
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 1;

    auto speed = toSpeed(baudRate);
    if (!speed) {
        Logger::logError("NativeSerialPort::configurePort() -> Unsupported baud rate: "
                         + std::to_string(baudRate));
        return false;
    }
    if (cfsetispeed(&tio, *speed) != 0 || cfsetospeed(&tio, *speed) != 0) {
        Logger::logError(std::string("NativeSerialPort::configurePort() -> cfset*speed failed: ")
                         + std::strerror(errno));
        return false;
    }

    if (tcsetattr(fd, TCSANOW, &tio) != 0) {
        Logger::logError(std::string("NativeSerialPort::configurePort() -> tcsetattr failed: ")
                         + std::strerror(errno));
        return false;
    }

    // Vaciar colas
    tcflush(fd, TCIOFLUSH);
    return true;
}

void NativeSerialPort::init() {
    closeIfOpen();

    fd = ::open(devicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        Logger::logError(std::string("NativeSerialPort::init() -> open(") + devicePath + ") failed: "
                         + std::strerror(errno));
        return;
    }
    if (!configurePort()) {
        closeIfOpen();
        return;
    }

    Logger::logInfo(std::string("NativeSerialPort::init() -> Opened ") + devicePath
                    + " @ " + std::to_string(baudRate) + " baud");
}

bool NativeSerialPort::available() {
    if (fd < 0) return false;
    int bytes = 0;
    if (ioctl(fd, FIONREAD, &bytes) == -1) {
        // FIONREAD no siempre está soportado; fallback: intentar una lectura no bloqueante
        uint8_t tmp;
        ssize_t n = ::read(fd, &tmp, 1);
        if (n > 0) {
            rxBuffer.push_back(tmp);
            return true;
        }
        return !rxBuffer.empty();
    }
    return (bytes > 0) || !rxBuffer.empty();
}

void NativeSerialPort::send(const std::vector<uint8_t>& data) {
    if (fd < 0) {
        Logger::logError("NativeSerialPort::send() -> port not open");
        return;
    }
    if (data.size() > 255) {
        Logger::logError("NativeSerialPort::send() -> packet > 255 bytes");
        return;
    }

    const uint8_t sof = kSOF;
    const uint8_t len = static_cast<uint8_t>(data.size());

    Logger::logInfo("NativeSerialPort::send() -> " + Logger::vectorToHexString(data));

    // Escribir paquete completo (pequeños buffers; una sola write normalmente basta)
    ssize_t w1 = ::write(fd, &sof, 1);
    ssize_t w2 = ::write(fd, &len, 1);
    ssize_t w3 = ::write(fd, data.data(), data.size());

    if (w1 != 1 || w2 != 1 || w3 != static_cast<ssize_t>(data.size())) {
        Logger::logError(std::string("NativeSerialPort::send() -> write failed: ")
                         + std::strerror(errno));
    }
}

std::vector<std::vector<uint8_t>> NativeSerialPort::read() {
    std::vector<std::vector<uint8_t>> packets;
    if (fd < 0) return packets;

    // 1) Acumular lo disponible en el puerto (no bloqueante)
    uint8_t tmp[256];
    for (;;) {
        ssize_t n = ::read(fd, tmp, sizeof(tmp));
        if (n > 0) {
            rxBuffer.insert(rxBuffer.end(), tmp, tmp + n);

            // Cota de seguridad
            if (rxBuffer.size() > kMaxBuf) {
                Logger::logError("NativeSerialPort::read() -> RX overflow, trimming");
                using diff_t = std::vector<uint8_t>::difference_type;
                rxBuffer.erase(rxBuffer.begin(),
                               rxBuffer.end() - static_cast<diff_t>(kMaxBuf));
            }
            // sigue leyendo hasta agotar
            continue;
        }
        if (n == 0) {
            // EOF (raro en TTY), salir
            break;
        }
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // no hay más por ahora
                break;
            }
            Logger::logError(std::string("NativeSerialPort::read() -> read error: ")
                             + std::strerror(errno));
            break;
        }
    }

    if (rxBuffer.empty()) return packets;

    // 2) Parsear frames: [SOF][LEN][PAYLOAD]
    size_t consume = 0;
    for (;;) {
        auto searchStart = rxBuffer.begin() +
            static_cast<std::vector<uint8_t>::difference_type>(consume);

        auto sofIt = std::find(searchStart, rxBuffer.end(), kSOF);
        if (sofIt == rxBuffer.end()) {
            // no hay SOF; descartamos lo previo a la búsqueda
            break;
        }
        const size_t sofPos = static_cast<size_t>(sofIt - rxBuffer.begin());

        // ¿Hay [SOF][LEN]?
        if (rxBuffer.size() < sofPos + 2) break;

        const uint8_t len = rxBuffer[sofPos + 1];
        if (len == 0 || len > 255 || len > MAX_RECEIVED_PACKET_SIZE) {
            Logger::logError("NativeSerialPort::read() -> invalid length: " + std::to_string(len));
            consume = sofPos + 1; // avanzar 1 y seguir buscando
            continue;
        }

        // ¿Tenemos el payload completo?
        if (rxBuffer.size() < sofPos + 2 + len) {
            // incompleto
            break;
        }

        // Extraer payload
        std::vector<uint8_t> frame(
            rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(sofPos + 2),
            rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(sofPos + 2 + len)
        );
        packets.push_back(std::move(frame));

        // Consumir este frame y seguir
        consume = sofPos + 2 + len;
    }

    // 3) Eliminar lo consumido
    if (consume > 0) {
        rxBuffer.erase(rxBuffer.begin(),
                       rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(consume));
    }

    return packets;
}

#endif // !ARDUINO
