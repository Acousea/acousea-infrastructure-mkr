#ifndef NULL_PACKET_H
#define NULL_PACKET_H

#include "Packet.h"
/**
 * @brief Null packet class. 
 * This packet is used to create a packet which will not be routed by the router, niether processed by the processor.
 */
class NullPacket : public Packet {
public:

    NullPacket() : Packet(buildNullPacket(), PACKET_HEADER_LENGTH) {}

private:
    static const uint8_t* buildNullPacket() {
        static uint8_t packetData[Packet::PACKET_HEADER_LENGTH];
        packetData[0] = Packet::SYNC_BYTE;
        packetData[1] = Packet::OpCode::NULL_PACKET;
        packetData[2] = Packet::Address::NULL_ADDRESS;
        packetData[3] = 0x00;  // Payload length set to 1 for error code        
        return packetData;
    }
};

#endif // NULL_PACKET_H
