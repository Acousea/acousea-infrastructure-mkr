#include "PacketQueue.hpp"

#include <cstdio>

#include "SharedMemory/SharedMemory.hpp"
#include "Logger/Logger.h"

PacketQueue::PacketQueue(StorageManager &storage, RTCController &rtc) : storage_(storage), rtc_(rtc), writeOffset_{0}
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

bool PacketQueue::arePortsEmpty(const uint8_t *ports, const size_t portCount) const
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

bool PacketQueue::push(uint8_t port, const uint8_t *data, uint16_t length)
{
    if (!data || length == 0)
        return false;
    if (port == 0 || port > MAX_PORT)
        return false;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    auto *tmp = reinterpret_cast<uint8_t *>(SharedMemory::tmpBuffer());
    const size_t max = SharedMemory::tmpBufferSize();

    const size_t needed = HEADER_SIZE + length + FOOTER_SIZE;
    if (needed > max)
    {
        return false;
    }

    // Safe handling if data == tmp (in case the user uses the temporary buffer directly)
    if (data == tmp)
    {
        memmove(tmp + HEADER_SIZE, tmp, length);
    }
    else
    {
        memcpy(tmp + HEADER_SIZE, data, length);
    }

    const uint32_t ts = rtc_.getEpoch();

    // Copy the header afterwards
    tmp[0] = START_BYTE;                                 // Start byte
    tmp[1] = static_cast<uint8_t>(ts & 0xFF);            // Timestamp (4 bytes)
    tmp[2] = static_cast<uint8_t>((ts >> 8) & 0xFF);     // Timestamp (4 bytes)
    tmp[3] = static_cast<uint8_t>((ts >> 16) & 0xFF);    // Timestamp (4 bytes)
    tmp[4] = static_cast<uint8_t>((ts >> 24) & 0xFF);    // Timestamp (4 bytes)
    tmp[5] = static_cast<uint8_t>(length & 0xFF);        // Length (2 bytes)
    tmp[6] = static_cast<uint8_t>((length >> 8) & 0xFF); // Length (2 bytes) [HEADER_SIZE - 1]

    // Add footer
    tmp[HEADER_SIZE + length] = END_BYTE;

    // Calculate the end position (header + data + footer)
    const size_t pos = HEADER_SIZE + length + FOOTER_SIZE;

    const bool ok = storage_.appendBytesToFile(path, tmp, pos);
    if (ok)
    {
        writeOffset_[port] += pos; // Update write offset
    }

    return ok;
}

uint16_t PacketQueue::popNext(const uint8_t port, uint8_t *outBuffer, const uint16_t maxOutSize)
{
    if (!outBuffer || maxOutSize == 0)
        return 0;
    if (port == 0 || port > MAX_PORT)
        return 0;

    char path[32];
    snprintf(path, sizeof(path), "%s%u", queueBaseName_, port);

    auto *readBuffer = reinterpret_cast<uint8_t *>(SharedMemory::tmpBuffer());

    const size_t readBytes = storage_.readFileRegionBytes(path, readOffset_[port], readBuffer, SharedMemory::tmpBufferSize());

    if (readBytes < HEADER_SIZE + FOOTER_SIZE)
    {
        return 0;
    }

    size_t pos = 0;

    if (readBuffer[pos] != START_BYTE)
    {
        return 0;
    }

    pos++; // Skip start byte
    uint32_t timestamp =
        (static_cast<uint32_t>(readBuffer[pos++]) |
         (static_cast<uint32_t>(readBuffer[pos++]) << 8) |
         (static_cast<uint32_t>(readBuffer[pos++]) << 16) |
         (static_cast<uint32_t>(readBuffer[pos++]) << 24));

    const uint16_t readLen = static_cast<uint16_t>(readBuffer[pos]) | (static_cast<uint16_t>(readBuffer[pos + 1]) << 8);
    pos += 2;

    // LOG_CLASS_INFO("PacketQueue::popNext() -> Port %u: ts=%lu len=%u (w=%lu:%lu r=%lu:%lu size=%lu)",
    //            port,
    //            static_cast<unsigned long>(timestamp),
    //            static_cast<unsigned int>(readLen),
    //            static_cast<unsigned long>(writeOffset_[port] >> 32),
    //            static_cast<unsigned long>(writeOffset_[port] & 0xFFFFFFFF),
    //            static_cast<unsigned long>(readOffset_[port] >> 32),
    //            static_cast<unsigned long>(readOffset_[port] & 0xFFFFFFFF),
    //            static_cast<unsigned long>(storage_.fileSize(path)));


    if (readLen > maxOutSize)
    {
        LOG_CLASS_ERROR("PacketQueue::popNext() -> Port %u: Output buffer too small (need %u, have %u)", port,
                        static_cast<unsigned int>(readLen), static_cast<unsigned int>(maxOutSize));
        return 0;
    }

    memcpy(outBuffer, readBuffer + pos, readLen);

    pos += readLen; // Skip to the end byte

    if (readBuffer[pos] != END_BYTE)
    {
        LOG_CLASS_ERROR("PacketQueue::popNext() -> Port %u: Invalid end byte", port);
        return 0;
    }

    // Actualiza el índice para el siguiente paquete
    readOffset_[port] += HEADER_SIZE + readLen + FOOTER_SIZE;
    // LOG_CLASS_INFO("PacketQueue::popNext() -> Port %u, Read %u bytes", port, static_cast<unsigned int>(readLen));
    return readLen;
}

uint16_t PacketQueue::popAny(uint8_t *outBuffer, uint16_t maxOutSize)
{
    uint8_t outPorts[MAX_PORT] = {};
    for (uint8_t i = 0; i < MAX_PORT; i++) // NAX_PORT = 4 => 0 -> 1, 1 -> 2, 2 -> 3, 3 -> 4 (4 out)
    {
        outPorts[i] = i + 1; // Los puertos empiezan de 1 hasta MAX_PORT
    }

    return popAnyFromPorts(outPorts, MAX_PORT, outBuffer, maxOutSize);
}

uint16_t PacketQueue::popAnyFromPorts(const uint8_t *ports, const size_t portCount, uint8_t *outBuffer,
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
        return popNext(ports[i], outBuffer, maxOutSize);
    }

    return 0;
}
