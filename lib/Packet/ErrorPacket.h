#ifndef ERROR_PACKET_H
#define ERROR_PACKET_H

#include "Packet.h"

class ErrorPacket : public Packet {
public:
    enum class ErrorCode : uint8_t {
        INVALID_OPCODE = 0x01,
        INVALID_PAYLOAD = 0x02,
        INVALID_SENDER_ADDRESS = 0x03,
        INVALID_RECIPIENT_ADDRESS = 0x04,
        INVALID_PACKET_LENGTH = 0x05,
        INVALID_SYNC_BYTE = 0x06
    };

    ErrorPacket(uint8_t senderAddress, uint8_t recipientAddress, ErrorCode errorCode)
        : Packet(buildErrorPacket(senderAddress, recipientAddress, static_cast<uint8_t>(errorCode)), 
                                  PACKET_HEADER_LENGTH + 1) {}

private:
    static const uint8_t* buildErrorPacket(uint8_t senderAddress, uint8_t recipientAddress, uint8_t errorCode) {
        static uint8_t errorData[Packet::PACKET_HEADER_LENGTH + 1];
        errorData[0] = Packet::SYNC_BYTE;
        errorData[1] = (senderAddress << 6) | (recipientAddress << 4);
        errorData[2] = Packet::OpCode::ERROR;
        errorData[3] = 0x01;  // Payload length set to 1 for error code
        errorData[4] = errorCode;  // Error code as payload
        return errorData;
    }
};

#endif // ERROR_PACKET_H
