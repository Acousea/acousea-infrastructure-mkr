#if defined(PLATFORM_NATIVE) && !defined(UNIT_TESTING)

#include "NativeSerialPort.h"
#include <cstring>
#include <algorithm>
#include "Logger/Logger.h"


void NativeSerialPort::closeIfOpen()
{
    if (fd >= 0)
    {
        ::close(fd);
        fd = -1;
    }
}

std::optional<speed_t> NativeSerialPort::toSpeed(int baud)
{
    switch (baud)
    {
    case 9600: return B9600;
    case 19200: return B19200;
    case 38400: return B38400;
    case 57600: return B57600;
    case 115200: return B115200;
#ifdef B230400
    case 230400: return B230400;
#endif
#ifdef B460800
    case 460800: return B460800;
#endif
#ifdef B921600
    case 921600: return B921600;
#endif
    default: return std::nullopt;
    }
}

bool NativeSerialPort::configurePort()
{
    termios tio{};
    if (tcgetattr(fd, &tio) != 0)
    {
        LOG_CLASS_ERROR("::configurePort() -> tcgetattr failed: %s", std::strerror(errno));
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
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 1;

    auto speed = toSpeed(baudRate);
    if (!speed)
    {
        LOG_CLASS_ERROR("configurePort() -> Unsupported baud rate: %d", baudRate);
        return false;
    }
    if (cfsetispeed(&tio, *speed) != 0 || cfsetospeed(&tio, *speed) != 0)
    {
        LOG_CLASS_ERROR("::configurePort() -> cfset*speed failed: %s", std::strerror(errno));
        return false;
    }

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
    {
        LOG_CLASS_ERROR("::configurePort() -> tcsetattr failed: %s", std::strerror(errno));
        return false;
    }

    // Vaciar colas
    tcflush(fd, TCIOFLUSH);
    return true;
}

void NativeSerialPort::init()
{
    closeIfOpen();

    fd = ::open(devicePath.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0)
    {
        LOG_CLASS_ERROR("::init() -> open(%s) failed: %s", devicePath.c_str(), std::strerror(errno));
        return;
    }
    if (!configurePort())
    {
        closeIfOpen();
        return;
    }

    LOG_CLASS_ERROR("::init() -> Opened %s @ %d baud", devicePath.c_str(), baudRate);
}

bool NativeSerialPort::available()
{
    if (fd < 0) return false;
    int bytes = 0;
    if (ioctl(fd, FIONREAD, &bytes) == -1)
    {
        // FIONREAD no siempre está soportado; fallback: intentar una lectura no bloqueante
        uint8_t tmp;
        ssize_t n = ::read(fd, &tmp, 1);
        if (n > 0)
        {
            rxBuffer.push_back(tmp);
            return true;
        }
        return !rxBuffer.empty();
    }
    return (bytes > 0) || !rxBuffer.empty();
}

bool NativeSerialPort::send(const std::vector<uint8_t>& data)
{
    if (fd < 0)
    {
        LOG_CLASS_ERROR("NativeSerialPort::send() -> port not open");
        return false;
    }
    if (data.size() > 255)
    {
        LOG_CLASS_ERROR("NativeSerialPort::send() -> packet > 255 bytes");
        return false;
    }

    const uint8_t sof = kSOF;
    const uint8_t len = static_cast<uint8_t>(data.size());

    LOG_CLASS_INFO("::send() -> Sending packet: SOF=0x%02X, LEN=%d, DATA=%s", sof, len,
                   Logger::vectorToHexString(data.data(), data.size()).c_str());


    // Escribir paquete completo (pequeños buffers; una sola write normalmente basta)
    ssize_t w1 = ::write(fd, &sof, 1);
    ssize_t w2 = ::write(fd, &len, 1);
    ssize_t w3 = ::write(fd, data.data(), data.size());

    if (w1 != 1 || w2 != 1 || w3 != static_cast<ssize_t>(data.size()))
    {
        LOG_CLASS_ERROR("::send() -> write failed: %s", std::strerror(errno));
        return false;
    }
    return true;
}

std::vector<std::vector<uint8_t>> NativeSerialPort::read()
{
    std::vector<std::vector<uint8_t>> packets;
    if (fd < 0) return packets;

    // 1) Acumular lo disponible en el puerto (no bloqueante)
    uint8_t tmp[256];
    for (;;)
    {
        ssize_t n = ::read(fd, tmp, sizeof(tmp));
        if (n > 0)
        {
            rxBuffer.insert(rxBuffer.end(), tmp, tmp + n);

            // Cota de seguridad
            if (rxBuffer.size() > kMaxBuf)
            {
                LOG_CLASS_ERROR("NativeSerialPort::read() -> RX overflow, trimming");
                using diff_t = std::vector<uint8_t>::difference_type;
                rxBuffer.erase(rxBuffer.begin(),
                               rxBuffer.end() - static_cast<diff_t>(kMaxBuf));
            }
            // sigue leyendo hasta agotar
            continue;
        }
        if (n == 0)
        {
            // EOF (raro en TTY), salir
            break;
        }
        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // no hay más por ahora
                break;
            }
            LOG_CLASS_ERROR("::read() -> read error: " "%s", std::strerror(errno));
            break;
        }
    }

    if (rxBuffer.empty()) return packets;

    // 2) Parsear frames: [SOF][LEN][PAYLOAD]
    size_t consume = 0;
    for (;;)
    {
        auto searchStart = rxBuffer.begin() +
            static_cast<std::vector<uint8_t>::difference_type>(consume);

        auto sofIt = std::find(searchStart, rxBuffer.end(), kSOF);
        if (sofIt == rxBuffer.end())
        {
            // no hay SOF; descartamos lo previo a la búsqueda
            break;
        }
        const size_t sofPos = static_cast<size_t>(sofIt - rxBuffer.begin());

        // ¿Hay [SOF][LEN]?
        if (rxBuffer.size() < sofPos + 2) break;

        const uint8_t len = rxBuffer[sofPos + 1];
        if (len == 0 || len > 255 || len > MAX_RECEIVED_PACKET_SIZE)
        {
            LOG_CLASS_ERROR("NativeSerialPort::read() -> invalid length: %d", len);
            consume = sofPos + 1; // avanzar 1 y seguir buscando
            continue;
        }

        // ¿Tenemos el payload completo?
        if (rxBuffer.size() < sofPos + 2 + len)
        {
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
    if (consume > 0)
    {
        rxBuffer.erase(rxBuffer.begin(),
                       rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(consume));
    }

    return packets;
}

#endif // !ARDUINO
