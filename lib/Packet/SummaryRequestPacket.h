#ifndef SUMMARY_REQUEST_PACKET_H
#define SUMMARY_REQUEST_PACKET_H

#include "Packet.h"

class SummaryRequestPacket : public Packet {
public:
    
    SummaryRequestPacket(Packet::PacketType packetType)
        : Packet(buildSummaryRequestPacket(packetType), PACKET_HEADER_LENGTH) {}

private:
    static const uint8_t* buildSummaryRequestPacket(Packet::PacketType packetType) {
        static uint8_t errorData[Packet::PACKET_HEADER_LENGTH + 1];
        errorData[0] = Packet::SYNC_BYTE;
        errorData[1] = Packet::OpCode::SUMMARY_REPORT;
        errorData[2] = SENDER(Packet::Address::DRIFTER) | RECEIVER(Packet::Address::PI3) | packetType;        
        errorData[3] = 0x00;  // Payload length set to 1 for error code        
        return errorData;
    }
};

#endif // SUMMARY_REQUEST_PACKET_H
