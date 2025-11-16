#ifndef ACOUSEA_INFRASTRUCTURE_MKR_BINARYFRAME_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_BINARYFRAME_HPP

#include <cstdint>
#include <cstddef>

namespace BinaryFrame
{
    // Format:
    // [0]   START_BYTE
    // [1..4] timestamp (uint32 little endian)
    // [5..6] length (uint16 little endian)
    // [7..7+len-1] payload
    // [7+len] END_BYTE

    constexpr uint8_t START_BYTE = 0xAA;
    constexpr uint8_t END_BYTE = 0x55;

    constexpr size_t HEADER_SIZE = 1 + 4 + 2; // start + ts + len
    constexpr size_t FOOTER_SIZE = 1; // end byte

    struct FrameView
    {
        const uint8_t* payload = nullptr;
        uint16_t payloadLength = 0;
        uint32_t timestamp = 0;
    };

    struct Header
    {
        uint8_t startByte;
        uint32_t timestamp;
        uint16_t payloadLength;
    };

    /**
     * Returns the required buffer size to wrap a payload of the given length.
     */
    constexpr size_t requiredSize(const uint16_t payloadLen) noexcept
    {
        return HEADER_SIZE + static_cast<size_t>(payloadLen) + FOOTER_SIZE;
    }

    bool parseHeader(const uint8_t* buffer, size_t bufferSize, Header& outHeader) noexcept;

    bool encodeHeader(uint8_t* outBuffer, size_t outBufferSize, const Header& header) noexcept;

    /** Writes the header and footer into the buffer without copying the payload.
     *
     * It is assumed that:
     *  - payload starts at frameBuffer + HEADER_SIZE
     *  - payload is already written there by the caller
     *  - bufferSize >= HEADER_SIZE + payloadLen + FOOTER_SIZE
     */
    [[nodiscard]] bool wrapInPlace(uint8_t* outFrameBuffer,
                                   size_t outFrameBufferSize,
                                   const uint8_t* payloadBuffer,
                                   uint16_t payloadLen,
                                   uint32_t timestamp) noexcept;

    /**
     * Returs the unwrapped frame view from the given buffer.
     * Does not copy the payload: only returns pointer and metadata.
     *
     * buffer      -> pointer to the beginning of the frame (start byte)
     * bufferSize  -> number of valid bytes from buffer
     */
    [[nodiscard]] bool unwrap(const uint8_t* buffer,
                              size_t bufferSize,
                              FrameView& outFrameView) noexcept;
} // BinaryFrame

#endif //ACOUSEA_INFRASTRUCTURE_MKR_BINARYFRAME_HPP
