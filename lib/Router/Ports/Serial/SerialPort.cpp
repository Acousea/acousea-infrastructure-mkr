#ifdef ARDUINO
#include "SerialPort.h"

#include <algorithm>


SerialPort::SerialPort(Uart* serialPort, int baudRate)
    : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate)
{
}

void SerialPort::init()
{
    serialPort->begin(baudRate);
    serialPort->setTimeout(10000); // Set a timeout of 5000ms
    serialPort->flush();
    LOG_CLASS_INFO("SerialPort::init() -> Serial port initialized");
}


bool SerialPort::available()
{
    return serialPort->available() > 0;
}

bool SerialPort::send(const std::vector<uint8_t>& data)
{
    if (data.size() > 255)
    {
        LOG_CLASS_ERROR("SerialPort::send() -> packet > 255 bytes");
        return false;
    }
    const auto len = static_cast<uint8_t>(data.size());
    LOG_CLASS_INFO("SerialPort::send() -> %s", Logger::vectorToHexString(data.data(), data.size()).c_str());
    serialPort->write(&kSOF, 1);
    serialPort->write(&len, 1);
    serialPort->write(data.data(), data.size());
    return true;
}

std::vector<std::vector<uint8_t>> SerialPort::read()
{
    std::vector<std::vector<uint8_t>> packets;

    // 1) Acumular lo disponible en el puerto (no bloqueante más de lo necesario)
    uint8_t tmp[128];
    while (serialPort->available() > 0)
    {
        const size_t n = serialPort->readBytes(tmp, sizeof(tmp));
        if (n == 0) break;
        rxBuffer.insert(rxBuffer.end(), tmp, tmp + n);

        // Cota de seguridad: conservar solo los últimos kMaxBuf bytes
        if (rxBuffer.size() > kMaxBuf)
        {
            LOG_CLASS_ERROR("SerialPort::read() -> RX overflow, trimming");
            using diff_t = std::vector<uint8_t>::difference_type;
            rxBuffer.erase(rxBuffer.begin(), rxBuffer.end() - static_cast<diff_t>(kMaxBuf));
        }
    }

    if (rxBuffer.empty()) return packets;

    // 2) Parsear frames: [SOF][LEN][PAYLOAD...]
    size_t consume = 0; // cuántos bytes eliminamos al final
    for (;;)
    {
        // Buscar el próximo SOF desde 'consume'
        auto sofIt = std::find(
            rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(consume),
            rxBuffer.end(),
            kSOF);

        if (sofIt == rxBuffer.end())
        {
            LOG_CLASS_INFO("SerialPort::read() -> No SOF found, clearing buffer");
            break; // no hay SOF -> esperar más datos
        }


        const size_t sofPos = static_cast<size_t>(sofIt - rxBuffer.begin());

        // ¿Hay al menos SOF + LEN?
        if (rxBuffer.size() < sofPos + 2) break;

        const uint8_t len = rxBuffer[sofPos + 1];
        if (len == 0 || len > 255 || len > MAX_RECEIVED_PACKET_SIZE)
        {
            // Longitud inválida: avanza 1 byte y reintenta
            LOG_CLASS_ERROR("SerialPort::read() -> invalid length: %d", len);
            consume = sofPos + 1;
            continue;
        }

        // ¿Tenemos el payload completo?
        if (rxBuffer.size() < sofPos + 2 + len)
        {
            LOG_CLASS_INFO("SerialPort::read() -> Incomplete frame, waiting for more data");
            break; // frame incompleto
        }

        // Extraer payload (solo bytes protobuf)
        std::vector<uint8_t> frame(rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(sofPos + 2),
                                   rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(sofPos + 2 +
                                       len));
        packets.push_back(std::move(frame));

        // Consumir este frame y seguir buscando el siguiente
        consume = sofPos + 2 + len;
    }

    // 3) Eliminar lo consumido (todo hasta el final del último frame completo o último SOF procesado)
    if (consume > 0)
    {
        rxBuffer.erase(rxBuffer.begin(),
                       rxBuffer.begin() + static_cast<std::vector<uint8_t>::difference_type>(consume));
    }

    return packets;
}

#endif // ARDUINO
