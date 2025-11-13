//
// Created by admin on 12/11/2025.
//

#include "FlashPacketQueue.hpp"

#include <Adafruit_SPIFlash.h>
#include "Logger/Logger.h"

namespace
{
    Adafruit_SPIFlash& getFlash()
    {
        static Adafruit_FlashTransport_SPI flashTransport(SS, &SPI);
        static Adafruit_SPIFlash flash(&flashTransport);
        return flash;
    }
}

FlashPacketQueue::FlashPacketQueue(uint32_t baseAddr, uint32_t size) : base(baseAddr), capacity(size)
{
}

bool FlashPacketQueue::begin()
{
    const bool beginOK = getFlash().begin();
    if (!beginOK)
    {
        LOG_CLASS_ERROR("FlashPacketQueue::begin() -> Failed to initialize flash storage");
        return false;
    }

    LOG_CLASS_INFO("FlashPacketQueue::begin() -> Flash storage initialized successfully");
    head = 0;
    tail = 0;
    return true;
}

bool FlashPacketQueue::isEmpty() const
{
    return head == tail;
}

bool FlashPacketQueue::isEmptyForPort(uint8_t targetPortType)
{
    if (isEmpty()) return true;

    uint32_t scanPtr = tail;

    while (scanPtr != head)
    {
        uint8_t header[3];
        readWrappedAt(scanPtr, header, sizeof(header));

        const uint8_t portType = header[0];
        const uint16_t length = static_cast<uint16_t>(header[1]) |
            (static_cast<uint16_t>(header[2]) << 8);
        const uint32_t total = sizeof(header) + length;

        if (portType == targetPortType)
        {
            return false; // encontrado paquete para ese puerto
        }

        scanPtr = advance(scanPtr, total);
    }

    // ningún paquete para ese puerto
    return true;
}

void FlashPacketQueue::clear()
{
    head = tail = 0;
}

bool FlashPacketQueue::push(const uint8_t portType, const uint8_t* data, uint16_t length)
{
    if (!data || length == 0) return false;

    const uint8_t header[3] = {
        portType,
        static_cast<uint8_t>(length & 0xFF),
        static_cast<uint8_t>((length >> 8) & 0xFF)
    };

    const uint32_t total = sizeof(header) + length;
    const uint32_t nextHead = advance(head, total);
    if (nextHead == tail) return false; // buffer lleno

    writeWrapped(header, sizeof(header));
    head = advance(head, sizeof(header));

    writeWrapped(data, length);
    head = advance(head, length);

    getFlash().syncBlocks();
    return true;
}

uint16_t FlashPacketQueue::popAny(uint8_t* outBuffer, uint16_t maxSize)
{
    if (isEmpty()) return 0;

    uint8_t header[3];
    readWrapped(header, sizeof(header));
    tail = advance(tail, sizeof(header));

    uint8_t portType = header[0];
    const uint16_t length = static_cast<uint16_t>(header[1]) |
        (static_cast<uint16_t>(header[2]) << 8);

    if (length == 0 || length > maxSize) return 0;

    readWrapped(outBuffer, length);
    tail = advance(tail, length);

    return length;
}

uint16_t FlashPacketQueue::popForPort(uint8_t targetPortType, uint8_t* outBuffer, uint16_t maxSize)
{
    if (isEmpty()) return 0;

    uint32_t scanPtr = tail;
    while (scanPtr != head)
    {
        // Leer header
        uint8_t header[3];
        readWrappedAt(scanPtr, header, sizeof(header));
        const uint8_t portType = header[0];
        const uint16_t length = static_cast<uint16_t>(header[1]) |
            (static_cast<uint16_t>(header[2]) << 8);
        const uint32_t total = sizeof(header) + length;

        // Avanza puntero a siguiente paquete (sin tocar tail aún)
        const uint32_t nextPtr = advance(scanPtr, total);

        if (portType == targetPortType)
        {
            // Copiar datos
            scanPtr = advance(scanPtr, sizeof(header));
            if (length > maxSize) return 0;

            readWrappedAt(scanPtr, outBuffer, length);

            // Ahora debemos "eliminar" este paquete de la cola
            // Para simplificar, movemos tail al siguiente paquete tras este
            tail = nextPtr;
            return length;
        }

        // Siguiente paquete
        scanPtr = nextPtr;
    }

    // Ningún paquete del puerto encontrado
    return 0;
}

uint32_t FlashPacketQueue::advance(uint32_t pos, uint32_t amount) const
{
    pos += amount;
    if (pos >= capacity) pos -= capacity;
    return pos;
}

void FlashPacketQueue::writeWrapped(const uint8_t* data, uint32_t len) const
{
    if (head + len <= capacity)
    {
        getFlash().writeBuffer(base + head, data, len);
    }
    else
    {
        const uint32_t first = capacity - head;
        getFlash().writeBuffer(base + head, data, first);
        getFlash().writeBuffer(base, data + first, len - first);
    }
}

void FlashPacketQueue::readWrapped(uint8_t* out, uint32_t len)
{
    if (tail + len <= capacity)
    {
        getFlash().readBuffer(base + tail, out, len);
    }
    else
    {
        const uint32_t first = capacity - tail;
        getFlash().readBuffer(base + tail, out, first);
        getFlash().readBuffer(base, out + first, len - first);
    }
}

void FlashPacketQueue::readWrappedAt(uint32_t pos, uint8_t* out, uint32_t len)
{
    if (pos + len <= capacity)
    {
        getFlash().readBuffer(base + pos, out, len);
    }
    else
    {
        const uint32_t first = capacity - pos;
        getFlash().readBuffer(base + pos, out, first);
        getFlash().readBuffer(base, out + first, len - first);
    }
}
