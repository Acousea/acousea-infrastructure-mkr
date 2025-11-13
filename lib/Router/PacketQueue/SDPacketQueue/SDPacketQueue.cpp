#ifdef PLATFORM_ARDUINO
#include "SDPacketQueue.hpp"

#include "SD.h"
#include "ErrorHandler/ErrorHandler.h"
#include "Logger/Logger.h"


SDPacketQueue::SDPacketQueue(const char* queuePath)
    : queuePath_(queuePath)
{
}

bool SDPacketQueue::begin()
{
    // Empezar con cola vacía cada reinicio
    if (SD.exists(queuePath_))
    {
        SD.remove(queuePath_);
    }
    File f = SD.open(queuePath_, FILE_WRITE);
    f.close();

    head_ = 0;
    tail_ = 0;

    LOG_CLASS_INFO("SDPacketQueue::begin() -> Queue ready (volatile, reset on boot)");
    return true;
}

void SDPacketQueue::clear()
{
    SD.remove(queuePath_);
    File f = SD.open(queuePath_, FILE_WRITE);
    f.close();
    head_ = tail_ = 0;
}

bool SDPacketQueue::isEmpty() const
{
    File f = SD.open(queuePath_, FILE_READ);
    if (!f) return true;
    const bool empty = (f.size() == 0);
    f.close();
    return empty;
}

bool SDPacketQueue::isEmptyForPort(uint8_t targetPortType)
{
    File f = SD.open(queuePath_, FILE_READ);
    if (!f) return true;

    while (f.available() >= HEADER_SIZE)
    {
        uint8_t header[HEADER_SIZE];
        f.read(header, HEADER_SIZE);
        const uint8_t portType = header[0];
        const uint16_t length = static_cast<uint16_t>(header[1]) |
            (static_cast<uint16_t>(header[2]) << 8);
        f.seek(f.position() + length);

        if (portType == targetPortType)
        {
            f.close();
            return false;
        }
    }
    f.close();
    return true;
}

bool SDPacketQueue::push(uint8_t portType, const uint8_t* data, uint16_t length)
{
    if (!data || length == 0) return false;

    File f = SD.open(queuePath_, FILE_WRITE);
    if (!f)
    {
        LOG_CLASS_ERROR("SDPacketQueue::push() -> Cannot open queue file");
        return false;
    }

    const uint8_t header[HEADER_SIZE] = {
        portType,
        static_cast<uint8_t>(length & 0xFF),
        static_cast<uint8_t>((length >> 8) & 0xFF)
    };

    f.write(header, HEADER_SIZE);
    f.write(data, length);
    f.flush();
    f.close();

    head_ += HEADER_SIZE + length;
    return true;
}

uint16_t SDPacketQueue::popAny(uint8_t* outBuffer, uint16_t maxSize)
{
    File in = SD.open(queuePath_, FILE_READ);
    if (!in) return 0;
    if (in.size() < HEADER_SIZE)
    {
        in.close();
        return 0;
    }

    uint8_t header[HEADER_SIZE];
    in.read(header, HEADER_SIZE);
    const uint16_t length = static_cast<uint16_t>(header[1]) |
                            (static_cast<uint16_t>(header[2]) << 8);

    if (length > maxSize)
    {
        in.close();
        return 0;
    }

    in.read(outBuffer, length);
    in.close();

    // Compactar el archivo (eliminar el primer paquete)
    File src = SD.open(queuePath_, FILE_READ);
    File tmp = SD.open("/queue.tmp", (O_WRITE | O_CREAT | O_TRUNC));

    if (!src || !tmp)
    {
        LOG_CLASS_ERROR("SDPacketQueue::popAny() -> Cannot open temp files");
        if (src) src.close();
        if (tmp) tmp.close();
        return 0;
    }

    src.seek(HEADER_SIZE + length);
    while (src.available())
        tmp.write(src.read());

    src.close();
    tmp.close();

    // Sustituir el archivo original copiando desde el temporal
    File src2 = SD.open("/queue.tmp", FILE_READ);
    File dst = SD.open(queuePath_, (O_WRITE | O_CREAT | O_TRUNC));

    if (src2 && dst)
    {
        while (src2.available())
            dst.write(src2.read());
    }
    else
    {
        LOG_CLASS_ERROR("SDPacketQueue::popAny() -> Failed to reopen queue files for copy");
    }

    if (src2) src2.close();
    if (dst) dst.close();

    SD.remove("/queue.tmp");

    tail_ += HEADER_SIZE + length;
    return length;
}


uint16_t SDPacketQueue::popForPort(uint8_t targetPortType, uint8_t* outBuffer, uint16_t maxSize)
{
    File f = SD.open(queuePath_, FILE_READ);
    if (!f) return 0;

    uint16_t foundLen = 0;
    uint32_t foundPos = 0;
    uint8_t header[HEADER_SIZE];

    while (f.available() >= HEADER_SIZE)
    {
        const uint32_t pos = f.position();
        f.read(header, HEADER_SIZE);
        const uint8_t portType = header[0];
        const uint16_t length = static_cast<uint16_t>(header[1]) |
                                (static_cast<uint16_t>(header[2]) << 8);

        if (portType == targetPortType)
        {
            foundPos = pos;
            foundLen = length;
            break;
        }

        f.seek(f.position() + length);
    }

    if (foundLen == 0 || foundLen > maxSize)
    {
        f.close();
        return 0;
    }

    f.seek(foundPos + HEADER_SIZE);
    f.read(outBuffer, foundLen);
    f.close();

    // Reescribir el archivo sin ese paquete
    File src = SD.open(queuePath_, FILE_READ);
    File tmp = SD.open("/queue.tmp", (O_WRITE | O_CREAT | O_TRUNC));

    if (!src || !tmp)
    {
        LOG_CLASS_ERROR("SDPacketQueue::popForPort() -> Cannot open temp files");
        if (src) src.close();
        if (tmp) tmp.close();
        return 0;
    }

    // Copiar todo antes del paquete
    for (uint32_t i = 0; i < foundPos; ++i)
        tmp.write(src.read());

    // Saltar el paquete
    src.seek(foundPos + HEADER_SIZE + foundLen);

    // Copiar el resto del archivo
    while (src.available())
        tmp.write(src.read());

    src.close();
    tmp.close();

    // Sustituir el archivo original copiando desde el temporal
    File src2 = SD.open("/queue.tmp", FILE_READ);
    File dst = SD.open(queuePath_, (O_WRITE | O_CREAT | O_TRUNC));

    if (src2 && dst)
    {
        while (src2.available())
            dst.write(src2.read());
    }
    else
    {
        LOG_CLASS_ERROR("SDPacketQueue::popForPort() -> Failed to reopen queue files for copy");
    }

    if (src2) src2.close();
    if (dst) dst.close();

    SD.remove("/queue.tmp");

    tail_ += HEADER_SIZE + foundLen;
    return foundLen;
}

#endif
