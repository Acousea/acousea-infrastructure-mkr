#ifdef PLATFORM_ARDUINO
#include "SerialPort.h"
#include <Logger/Logger.h>

#include "BinaryFrame/BinaryFrame.hpp"
#include "SharedMemory/SharedMemory.hpp"
#include "wait/WaitFor.hpp"
#include "WatchDog/WatchDogUtils.hpp"


SerialPort::SerialPort(Uart& serialPort, const int baudRate, PacketQueue& packetQueue)
    : IPort(PortType::SerialPort), serialPort(serialPort), baudRate(baudRate), packetQueue_(packetQueue)
{
}

void SerialPort::init()
{
    serialPort.begin(baudRate);
    serialPort.setTimeout(2500); // Set a timeout of 1000ms
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
    // Construir y enviar el frame binario
    auto* outBuffer = SharedMemory::tmpBuffer();
    constexpr size_t OUT_BUFFER_CAPACITY = SharedMemory::tmpBufferSize();
    if (const bool wrapOk = BinaryFrame::wrapInPlace(outBuffer, OUT_BUFFER_CAPACITY, data, len, 0);
        !wrapOk)
    {
        LOG_CLASS_ERROR("SerialPort::send() -> Failed to wrap data into binary frame");
        return false;
    }
    serialPort.write(outBuffer, BinaryFrame::requiredSize(len));
    serialPort.flush();

    LOG_CLASS_INFO("SerialPort::send() -> Packet sent successfully");

    return true;
}

bool SerialPort::sync()
{
    auto* rxBuffer = SharedMemory::tmpBuffer();
    constexpr size_t RX_BUFFER_CAPACITY = SharedMemory::tmpBufferSize();
    static size_t rxBufferReceivedBytes = 0; // bytes actualmente acumulados en el buffer

    LOG_CLASS_FREE_MEMORY("::sync() -> Before reading from serial port");
    // 1) Acumular lo disponible en el puerto (no bloqueante más de lo necesario)
    while (serialPort.available() > 0)
    {
        // Check for start byte
        WatchdogUtils::reset(); // Reset the watchdog to avoid resets during long reads
        if (const auto startByte = serialPort.peek(); startByte != BinaryFrame::START_BYTE)
        {
            LOG_CLASS_ERROR("::sync() -> Invalid start byte 0x%02X, clearing buffer", startByte);
            serialPort.read(); // discard invalid byte
            rxBufferReceivedBytes = 0; // clear buffer
            continue;
        }

        size_t remainingSpace = RX_BUFFER_CAPACITY - rxBufferReceivedBytes;
        if (BinaryFrame::HEADER_SIZE > remainingSpace)
        {
            LOG_CLASS_ERROR("::sync() -> RX buffer overflow, clearing buffer");
            rxBufferReceivedBytes = 0; // buffer overflow -> descartar
            break;
        }
        // First we try to read the header.
        WatchdogUtils::reset(); // Reset the watchdog to avoid resets during long reads
        const size_t numHeaderBytesRead = serialPort.readBytes(rxBuffer + rxBufferReceivedBytes,
                                                               BinaryFrame::HEADER_SIZE);

        rxBufferReceivedBytes += numHeaderBytesRead;
        if (numHeaderBytesRead == 0 || numHeaderBytesRead != BinaryFrame::HEADER_SIZE)
        {
            LOG_CLASS_INFO("::sync() -> Incomplete header read, waiting for more data");
            waitFor(100); // wait a bit before trying to read more
            if (serialPort.available() > 0) continue; // Check if more data is available
            break; // no more data available for now
        }

        BinaryFrame::Header header{};
        if (!BinaryFrame::parseHeader(rxBuffer, rxBufferReceivedBytes, header))
        {
            LOG_CLASS_ERROR("::sync() -> Invalid header detected, clearing buffer");
            rxBufferReceivedBytes = 0;
            return false;
        }

        // Now we check if we have enough space for the full frame. If not, we discard the current buffer content.
        const size_t totalFrameSize = BinaryFrame::requiredSize(header.payloadLength);
        if (totalFrameSize > RX_BUFFER_CAPACITY)
        {
            LOG_CLASS_ERROR(
                "::sync() -> RX buffer overflow due to large frame size, clearing buffer and discarding frame");
            rxBufferReceivedBytes = 0; // buffer overflow -> discard everything
            continue;
        }
        // We try to read the rest of the frame. (Inside a loop in case of incomplete reads)

        size_t bytesLeftToRead = totalFrameSize - BinaryFrame::HEADER_SIZE;
        if (bytesLeftToRead > RX_BUFFER_CAPACITY - rxBufferReceivedBytes)
        {
            LOG_CLASS_ERROR("::sync() -> RX buffer overflow. Bytes left to read = %d, remaining space = %d",
                            bytesLeftToRead, remainingSpace);
            rxBufferReceivedBytes = 0;
            continue;
        }

        while (bytesLeftToRead > 0) // Read the remaining bytes of the frame.
        {
            // Now we fill the rxBuffer with the remaining bytes
            WatchdogUtils::reset(); // Reset the watchdog to avoid resets during long reads
            const size_t numPayloadBytesRead = serialPort.readBytes(rxBuffer + rxBufferReceivedBytes, bytesLeftToRead);

            rxBufferReceivedBytes += numPayloadBytesRead;
            bytesLeftToRead -= numPayloadBytesRead;

            if (numPayloadBytesRead == 0 || numPayloadBytesRead < bytesLeftToRead)
            {
                LOG_CLASS_INFO("::sync() -> Incomplete payload read, waiting for more data");
                waitFor(100); // wait a bit before trying to read more
                if (serialPort.available() > 0) continue; // Check if more data is available
                break; // no more data available for now
            }

            // Now we unwrap the frame and store the payload in the packet queue.
            BinaryFrame::FrameView frameView;
            if (const bool unwrapOk = BinaryFrame::unwrap(rxBuffer, rxBufferReceivedBytes, frameView); !unwrapOk)
            {
                LOG_CLASS_ERROR("::sync() -> Failed to unwrap frame, clearing buffer");
                rxBufferReceivedBytes = 0; // frame inválido -> descartar
                break;
            }
            // Store the payload in the packet queue.
            if (const bool pushOk = packetQueue_.push(getTypeU8(), frameView.payload, frameView.payloadLength);
                !pushOk)
            {
                LOG_CLASS_ERROR("SerialPort::sync() -> Failed to push frame into Flash queue");
            }
            else
            {
                LOG_CLASS_INFO("SerialPort::sync() -> Stored frame of %d bytes into Flash queue",
                               frameView.payloadLength);
            }
        }

        // Clear the rxBuffer for the next frame.
        rxBufferReceivedBytes = 0;
    }
    return true;
}


#endif // ARDUINO
