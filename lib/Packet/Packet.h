#ifndef PACKET_H
#define PACKET_H

#include <Arduino.h>
#include <stdexcept>
#include <vector>


// Define receiver macro to shift the address to the left
#define RECEIVER(x) (x << 4)
#define SENDER(x) (x << 6)

// Define macros to get the sender and receiver addresses shifting the bits to the right
#define GET_SENDER(x) (x & 0xC0) >> 6
#define GET_RECEIVER(x) (x & 0x30) >> 4

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
        RECEIVER_AND_PACKET_TYPE_MASK = 0x3C, // 0b00111100
    } Address;

    typedef enum: uint8_t {
        PING = '0',        
        ERROR = '1',
    } OpCode;

    void swapSenderReceiverAddresses() const;
    void print() const;
    // Getters
    bool isValid() const;
    uint8_t getSyncByte() const;    
    uint8_t getAddresses() const;    
    uint8_t getOpCode() const;
    uint8_t getPayloadLength() const;
    const uint8_t *getPayload() const;
    const uint8_t *getFullPacket() const;
    std::vector<uint8_t> getFullPacketVector() const;
    size_t getFullPacketLength() const;

    // Setters
    void setSyncByte(uint8_t syncByte);
    void setAddresses(uint8_t addresses);        
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
    mutable uint8_t addresses;    
    uint8_t opCode;
    uint8_t payloadLength;
    uint8_t payload[MAX_PAYLOAD_LENGTH];
    mutable uint8_t fullPacket[MAX_PACKET_LENGTH]; // Buffer para el paquete completo (4 bytes de cabecera + hasta 200 bytes de payload)
};

#endif // PACKET_H
