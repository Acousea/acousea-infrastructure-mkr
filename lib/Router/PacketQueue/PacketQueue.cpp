#include "PacketQueue.hpp"

#include <cstdio>

#include "SharedMemory/SharedMemory.hpp"
#include "BinaryFrame/BinaryFrame.hpp"
#include "Logger/Logger.h"

PacketQueue::PacketQueue(StorageManager& storage, RTCController& rtc) : storage_(storage), rtc_(rtc), writeOffset_{0}
{
}

bool PacketQueue::begin()
{
    // Revisar cada puerto
    for (uint8_t port = 1; port <= MAX_PORT; port++)
    {
        char path[32];
        snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

        if (storage_.fileExists(path))
        {
            // archivo existente -> calcular offset al final (nuevos paquetes)
            writeOffset_[port] = storage_.fileSize(path);
            readOffset_[port] = storage_.fileSize(path);
            nextReadOffset_[port] = storage_.fileSize(path);
        }
        else
        {
            const bool createOk = storage_.createEmptyFile(path);
            if (!createOk)
            {
                LOG_CLASS_ERROR("PacketQueue::begin() -> Cannot create file: %s", path);
                return false;
            }
            writeOffset_[port] = 0;
            readOffset_[port] = 0;
            nextReadOffset_[port] = 0;
        }
    }

    LOG_CLASS_INFO("PacketQueue::begin() -> All queues initialized");
    return true;
}

bool PacketQueue::clear(uint8_t port)
{
    if (port == 0 || port > MAX_PORT)
        return false;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    if (storage_.fileExists(path) && !storage_.deleteFile(path))
    {
        return false;
    }

    const bool createOK = storage_.createEmptyFile(path);
    if (!createOK)
    {
        return false;
    }
    // Reset write offset for the port
    writeOffset_[port] = 0;
    readOffset_[port] = 0;
    nextReadOffset_[port] = 0;

    return true;
}

bool PacketQueue::isEmpty() const
{
    for (uint8_t port = 1; port <= MAX_PORT; port++)
    {
        if (!isPortEmpty(port))
        {
            return false;
        }
    }
    return true;
}

bool PacketQueue::arePortsEmpty(const uint8_t* ports, const size_t portCount) const
{
    if (!ports || portCount == 0)
        return true;

    for (size_t i = 0; i < portCount; i++)
    {
        const uint8_t port = ports[i];
        if (port == 0 || port > MAX_PORT)
            continue;

        if (!isPortEmpty(port))
        {
            return false;
        }
    }

    return true;
}

bool PacketQueue::isPortEmpty(uint8_t port) const
{
    if (port == 0 || port > MAX_PORT)
        return true;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    if (!storage_.fileExists(path))
    {
        return true; // Si el archivo no existe, está vacío
    }

    // LOG_CLASS_INFO("PacketQueue::isPortEmpty() -> Port %u: w=%lu:%lu r=%lu:%lu size=%lu",
    //                port,
    //                static_cast<unsigned long>(writeOffset_[port] >> 32),
    //                static_cast<unsigned long>(writeOffset_[port] & 0xFFFFFFFF),
    //                static_cast<unsigned long>(readOffset_[port] >> 32),
    //                static_cast<unsigned long>(readOffset_[port] & 0xFFFFFFFF),
    //                static_cast<unsigned long>(storage_.fileSize(path)));

    // If the read offset is equal to the write offset, the port is empty (could also check if filesize == readOffset_)
    return writeOffset_[port] == readOffset_[port];
}

bool PacketQueue::push(uint8_t port, const uint8_t* data, const uint16_t dataLength)
{
    if (!data || dataLength == 0)
        return false;
    if (port == 0 || port > MAX_PORT)
        return false;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    auto* outWrappedBuffer = SharedMemory::tmpBuffer();
    constexpr size_t outWrapperBufferMaxLength = SharedMemory::tmpBufferSize();
    const uint32_t ts = rtc_.getEpoch();

    if (const bool wrapOk = BinaryFrame::wrapInPlace(outWrappedBuffer, outWrapperBufferMaxLength, data, dataLength, ts);
        !wrapOk)
    {
        LOG_CLASS_ERROR("PacketQueue::push() -> Failed to wrap data for port %u", port);
        return false;
    }

    // Calculate the end position (header + data + footer)
    const size_t outWrapperBufferSize = BinaryFrame::requiredSize(dataLength);

    const bool ok = storage_.appendBytesToFile(path, outWrappedBuffer, outWrapperBufferSize);
    if (ok)
    {
        writeOffset_[port] += outWrapperBufferSize; // Update write offset
    }

    return ok;
}

uint64_t PacketQueue::getReadOffset(const uint8_t port) const
{
    if (port == 0 || port > MAX_PORT) return 0;
    return readOffset_[port];
}

uint64_t PacketQueue::getNextReadOffset(const uint8_t port) const
{
    if (port == 0 || port > MAX_PORT) return 0;
    return nextReadOffset_[port];
}

uint16_t PacketQueue::peekNext(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize)
{
    return _next(port, outBuffer, maxOutSize, false);
}

uint16_t PacketQueue::peekAny(uint8_t* outBuffer, const uint16_t maxOutSize)
{
    uint8_t outPorts[MAX_PORT] = {};
    for (uint8_t i = 0; i < MAX_PORT; i++) // NAX_PORT = 4 => 0 -> 1, 1 -> 2, 2 -> 3, 3 -> 4 (4 out)
    {
        outPorts[i] = i + 1; // Los puertos empiezan de 1 hasta MAX_PORT
    }

    return peekAnyFromPorts(outPorts, MAX_PORT, outBuffer, maxOutSize);
}

uint16_t PacketQueue::peekAnyFromPorts(const uint8_t* ports, const size_t portCount, uint8_t* outBuffer,
                                       const uint16_t maxOutSize)
{
    if (!ports || portCount == 0)
        return 0;
    if (!outBuffer || maxOutSize == 0)
        return 0;

    // Iterate over the provided ports
    for (size_t i = 0; i < portCount; i++)
    {
        // Check if this port is empty
        if (isPortEmpty(ports[i]))
        {
            continue;
        }
        // If not empty, peek from this port
        return _next(ports[i], outBuffer, maxOutSize, false);
    }

    return 0;
}


uint16_t PacketQueue::popNext(uint8_t port, uint8_t* outBuffer, uint16_t maxOutSize)
{
    return _next(port, outBuffer, maxOutSize, true);
}


uint16_t PacketQueue::popAny(uint8_t* outBuffer, uint16_t maxOutSize)
{
    uint8_t outPorts[MAX_PORT] = {};
    for (uint8_t i = 0; i < MAX_PORT; i++) // NAX_PORT = 4 => 0 -> 1, 1 -> 2, 2 -> 3, 3 -> 4 (4 out)
    {
        outPorts[i] = i + 1; // Los puertos empiezan de 1 hasta MAX_PORT
    }

    return popAnyFromPorts(outPorts, MAX_PORT, outBuffer, maxOutSize);
}

uint16_t PacketQueue::popAnyFromPorts(const uint8_t* ports, const size_t portCount, uint8_t* outBuffer,
                                      const uint16_t maxOutSize)
{
    if (!ports || portCount == 0)
        return 0;
    if (!outBuffer || maxOutSize == 0)
        return 0;

    // Iterate over the provided ports
    for (size_t i = 0; i < portCount; i++)
    {
        // Check if this port is empty
        if (isPortEmpty(ports[i]))
        {
            continue;
        }
        // If not empty, pop from this port
        return _next(ports[i], outBuffer, maxOutSize, true);
    }

    return 0;
}

bool PacketQueue::skipToNextPacket(uint8_t port)
{
    if (port == 0 || port > MAX_PORT)
        return false;

    // Nowhere to skip to if the port is already empty
    if (readOffset_[port] == writeOffset_[port])
        return false;

    // Nowhere to skip to if the next read offset is the same as the current read offset
    // (must have been peeked before)
    if (nextReadOffset_[port] == readOffset_[port])
        return false;

    // Update the current read offset to the next read offset
    readOffset_[port] = nextReadOffset_[port];

    // Ensure we do not exceed the write offset
    if (readOffset_[port] > writeOffset_[port])
    {
        readOffset_[port] = writeOffset_[port];
    }

    return true;
}

uint16_t PacketQueue::_next(const uint8_t port, uint8_t* outBuffer, const uint16_t maxOutSize, bool pop)
{
    if (!outBuffer || maxOutSize == 0)
        return 0;
    if (port == 0 || port > MAX_PORT)
        return 0;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    auto* dataBuffer = SharedMemory::tmpBuffer();

    const size_t dataBufferSize = storage_.readFileRegionBytes(
        path, readOffset_[port], dataBuffer, SharedMemory::tmpBufferSize()
    );

    BinaryFrame::FrameView outFrameView{};
    if (const bool unwrapOk = BinaryFrame::unwrap(dataBuffer, dataBufferSize, outFrameView); !unwrapOk)
    {
        LOG_CLASS_ERROR("PacketQueue::popNext() -> Port %u: Failed to unwrap binary frame", port);
        return 0;
    }


    // Copy the payload to the output buffer
    if (outFrameView.payloadLength > maxOutSize)
    {
        LOG_CLASS_ERROR("PacketQueue::popNext() -> Port %u: Output buffer too small (%u < %u)",
                        port,
                        static_cast<unsigned int>(maxOutSize),
                        static_cast<unsigned int>(outFrameView.payloadLength));
        return 0;
    }
    memcpy(outBuffer, outFrameView.payload, outFrameView.payloadLength);

    LOG_CLASS_INFO("PacketQueue::popNext() -> Port %u: ts=%lu len=%u (w=%lu:%lu r=%lu:%lu size=%lu)",
                   port,
                   static_cast<unsigned long>(outFrameView.timestamp),
                   static_cast<unsigned int>(outFrameView.payloadLength),
                   static_cast<unsigned long>(writeOffset_[port] >> 32),
                   static_cast<unsigned long>(writeOffset_[port] & 0xFFFFFFFF),
                   static_cast<unsigned long>(readOffset_[port] >> 32),
                   static_cast<unsigned long>(readOffset_[port] & 0xFFFFFFFF),
                   static_cast<unsigned long>(storage_.fileSize(path)));


    const auto totalEntrySize = BinaryFrame::requiredSize(outFrameView.payloadLength);

    // Always update the next read offset
    nextReadOffset_[port] = readOffset_[port] + totalEntrySize;

    // Actualiza el índice para el siguiente paquete si es necesario
    if (pop)
    {
        readOffset_[port] = nextReadOffset_[port];
    }

    // LOG_CLASS_INFO("PacketQueue::popNext() -> Port %u, Read %u bytes", port, static_cast<unsigned int>(readLen));
    return outFrameView.payloadLength;

}
