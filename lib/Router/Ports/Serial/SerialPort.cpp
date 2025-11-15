#ifdef PLATFORM_ARDUINO
#include "SerialPort.h"
#include <Logger/Logger.h>

#include "SharedMemory/SharedMemory.hpp"


SerialPort::SerialPort(Uart& serialPort, const int baudRate, PacketQueue& packetQueue)
    : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate), packetQueue_(packetQueue)
{
}

void SerialPort::init()
{
    serialPort.begin(baudRate);
    serialPort.setTimeout(10000); // Set a timeout of 5000ms
    serialPort.flush();
    LOG_CLASS_INFO("SerialPort::init() -> Serial port initialized");
}


bool SerialPort::available()
{
    return !packetQueue_.isPortEmpty(getTypeU8());
}

bool SerialPort::send(const uint8_t* data, const size_t length)
{
    if (length > 255)
    {
        LOG_CLASS_ERROR("SerialPort::send() -> packet > 255 bytes");
        return false;
    }
    const auto len = static_cast<uint8_t>(length);
    LOG_CLASS_INFO("::send() -> %s", Logger::vectorToHexString(data, length).c_str());
    LOG_CLASS_FREE_MEMORY("::send() -> Sending packet of %d bytes", length);
    serialPort.write(&kSOF, 1);
    serialPort.write(&len, 1);
    serialPort.write(data, length);
    return true;
}

bool SerialPort::sync()
{
    auto* rxBuffer = SharedMemory::tmpBuffer();
    constexpr size_t RX_BUFFER_CAPACITY = SharedMemory::tmpBufferSize();
    static size_t rxBufferLength = 0; // bytes actualmente acumulados en el buffer

    // 1) Acumular lo disponible en el puerto (no bloqueante más de lo necesario)
    while (serialPort.available() > 0 && rxBufferLength < RX_BUFFER_CAPACITY)
    {
        const size_t space = RX_BUFFER_CAPACITY - rxBufferLength;
        const size_t n = serialPort.readBytes(rxBuffer + rxBufferLength, space);
        if (n == 0) break;
        rxBufferLength += n;
    }


    if (rxBufferLength == 0) return true;

    // 2) Parsear frames: [SOF][LEN][PAYLOAD...]
    size_t consume = 0; // cuántos bytes eliminamos al final
    for (;;)
    {
        // Buscar el próximo SOF desde 'consume'
        auto sofPtr = static_cast<uint8_t*>(
            memchr(rxBuffer + consume, kSOF, rxBufferLength - consume)
        );

        if (sofPtr == nullptr)
        {
            LOG_CLASS_INFO("SerialPort::read() -> No SOF found, clearing buffer");
            break; // no hay SOF -> esperar más datos
        }


        const auto sofPos = static_cast<size_t>(sofPtr - rxBuffer);


        // ¿Hay al menos SOF + LEN?
        if (rxBufferLength < sofPos + 2) break;

        const uint8_t len = rxBuffer[sofPos + 1];
        if (len == 0 || len > RX_BUFFER_CAPACITY)
        {
            // Longitud inválida: avanza 1 byte y reintenta
            LOG_CLASS_ERROR("SerialPort::read() -> invalid length: %d", len);
            consume = sofPos + 1;
            continue;
        }

        // ¿Tenemos el payload completo?
        if (rxBufferLength < sofPos + 2 + len)
        {
            LOG_CLASS_INFO("SerialPort::read() -> Incomplete frame, waiting for more data");
            break; // frame incompleto
        }

        // Payload completo → guardar en Flash
        const uint8_t* payload = rxBuffer + sofPos + 2;
        if (!packetQueue_.push(getTypeU8(), payload, len))
        {
            LOG_CLASS_ERROR("SerialPort::sync() -> Failed to push frame into Flash queue");
        }
        else
        {
            LOG_CLASS_INFO("SerialPort::sync() -> Stored frame of %d bytes into Flash queue", len);
        }

        // Consumir este frame y seguir buscando el siguiente
        consume = sofPos + 2 + len;
    }

    // 3) Compactar buffer (mover datos pendientes al inicio)
    if (consume > 0)
    {
        const size_t remaining = rxBufferLength - consume;
        memmove(rxBuffer, rxBuffer + consume, remaining);
        rxBufferLength = remaining;
    }

    return true;
}


#endif // ARDUINO
