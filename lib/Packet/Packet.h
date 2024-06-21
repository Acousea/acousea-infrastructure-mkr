#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>
#include <stdexcept>
#include <vector>

class Packet
{
public:
    // Constructor
    Packet(const uint8_t *data, size_t length);

    typedef enum : uint8_t {
        BACKEND = 0,         // 0b00000000
        LOCALIZER = 0x01,    // 0b00000001
        DRIFTER = 0x02,      // 0b00000010
        PI3 = 0x03,          // 0b00000011
        SENDER_MASK = 0xC0,  // 0b11000000
        RECEIVER_MASK = 0x30, // 0b00110000
        LORA_PACKET = 0x08,  // 0b00001000
        IRIDIUM_PACKET = 0x04, // 0b00000100        
    } Address;

    typedef enum: uint8_t {
        PING = '0',        
        ERROR = '1',
    } OpCode;

    // Getters
    bool isValid() const;
    uint8_t getSyncByte() const;
    uint8_t getSenderAddress() const;
    uint8_t getRecipientAddress() const;
    uint8_t getPacketType() const;
    uint8_t getOpCode() const;
    uint8_t getPayloadLength() const;
    const uint8_t *getPayload() const;
    const uint8_t *getFullPacket() const;
    std::vector<uint8_t> getFullPacketVector() const;
    size_t getFullPacketLength() const;

    // Setters
    void setSyncByte(uint8_t syncByte);
    void setSenderAddress(uint8_t senderAddress);
    void setRecipientAddress(uint8_t recipientAddress);
    void setOpCode(uint8_t opCode);
    void setPayloadLength(uint8_t payloadLength);
    void setPayload(const uint8_t *payload, uint8_t length);

public:
    const static uint8_t SYNC_BYTE = 0x20;
    const static uint8_t PACKET_HEADER_LENGTH = 4;
    const static uint8_t MAX_PAYLOAD_LENGTH = 96;
    const static uint8_t MAX_PACKET_LENGTH = 100;

private:
    bool isValidAddress(uint8_t address);
    bool validPacket = false; 

private:
    uint8_t syncByte;
    uint8_t senderAddress;
    uint8_t recipientAddress;  
    uint8_t packetType;  
    uint8_t opCode;
    uint8_t payloadLength;
    uint8_t payload[MAX_PAYLOAD_LENGTH];
    mutable uint8_t fullPacket[MAX_PACKET_LENGTH]; // Buffer para el paquete completo (4 bytes de cabecera + hasta 200 bytes de payload)
};

#endif // PACKET_H
