#include "BinaryFrame.hpp"
#include "SharedMemory/SharedMemory.hpp"

namespace BinaryFrame
{


    bool wrapInPlace(uint8_t* outFrameBuffer, const size_t outFrameBufferSize,
                     const uint8_t* payloadBuffer, const uint16_t payloadLen,
                     const uint32_t timestamp) noexcept
    {
        if (!outFrameBuffer)
        {
            return false;
        }

        if (const size_t needed = requiredSize(payloadLen); outFrameBufferSize < needed)
        {
            return false;
        }

        if (outFrameBuffer == payloadBuffer) // If they're the same, just move the outFrameBuffer data
        {
            memmove(outFrameBuffer + HEADER_SIZE, outFrameBuffer, payloadLen);
        }
        else // If they're different, copy the payloadData to the required position on the outFrameBuffer
        {
            memcpy(outFrameBuffer + HEADER_SIZE, payloadBuffer, payloadLen);
        }

        // Start byte
        size_t pos = 0;
        outFrameBuffer[pos++] = START_BYTE;

        // Timestamp little endian
        outFrameBuffer[pos++] = static_cast<uint8_t>(timestamp & 0xFF);
        outFrameBuffer[pos++] = static_cast<uint8_t>((timestamp >> 8) & 0xFF);
        outFrameBuffer[pos++] = static_cast<uint8_t>((timestamp >> 16) & 0xFF);
        outFrameBuffer[pos++] = static_cast<uint8_t>((timestamp >> 24) & 0xFF);

        // Length little endian
        outFrameBuffer[pos++] = static_cast<uint8_t>(payloadLen & 0xFF);
        outFrameBuffer[pos++] = static_cast<uint8_t>((payloadLen >> 8) & 0xFF);

        // Footer al final
        outFrameBuffer[HEADER_SIZE + payloadLen] = END_BYTE;

        return true;
    }

    bool unwrap(const uint8_t* buffer, const size_t bufferSize, FrameView& outFrameView) noexcept
    {
        if (!buffer)
        {
            return false;
        }

        if (bufferSize < HEADER_SIZE + FOOTER_SIZE)
        {
            return false;
        }

        size_t pos = 0;

        if (buffer[pos++] != START_BYTE)
            return false;

        // Timestamp
        uint32_t ts = 0;
        ts |= static_cast<uint32_t>(buffer[pos++]);
        ts |= static_cast<uint32_t>(buffer[pos++]) << 8;
        ts |= static_cast<uint32_t>(buffer[pos++]) << 16;
        ts |= static_cast<uint32_t>(buffer[pos++]) << 24;

        // Length
        const uint16_t readLength = static_cast<uint16_t>(buffer[pos]) | static_cast<uint16_t>(buffer[pos + 1]) << 8;
        pos += 2;

        if (const size_t needed = requiredSize(readLength); bufferSize < needed)
        {
            return false;
        }

        // Payload start pointer (after length)
        const uint8_t* payloadPtr = buffer + pos;
        pos += readLength; // apunta a footer

        if (buffer[pos] != END_BYTE)
        {
            return false;
        }

        outFrameView.payload = payloadPtr;
        outFrameView.payloadLength = readLength;
        outFrameView.timestamp = ts;


        return true;
    }
} // BinaryFrame
