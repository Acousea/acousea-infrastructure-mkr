#ifndef SIMPLE_REPORT_PACKET_H
#define SIMPLE_REPORT_PACKET_H

#include "Packet.h"
#include "../GPS/IGPS.h"
#include "../Services/SummaryService.h"

/// FIXME: Sizeof  SimpleReport: 24 (8 + 1 + 8) = 17 -> 24 - 17 = 7 bytes of padding (memory alignment)
typedef struct SimpleReport {
    time_t epoch;
    uint8_t battery_percentage;
    GPSLocation location;    
} SimpleReport;


class SimpleReportPacket : public Packet {
public:
    
    SimpleReportPacket(SimpleReport report, Packet::PacketType packetType)
        : Packet(buildSimpleReportPacket(report, packetType), PACKET_HEADER_LENGTH) {}

private:
    static const uint8_t* buildSimpleReportPacket(SimpleReport report, Packet::PacketType packetType) {
        static uint8_t packetData[Packet::PACKET_HEADER_LENGTH + sizeof(report)];
        packetData[0] = Packet::SYNC_BYTE;
        packetData[1] = Packet::OpCode::SUMMARY_SIMPLE_REPORT;
        packetData[2] = SENDER(Packet::Address::DRIFTER) | RECEIVER(Packet::Address::BACKEND) | packetType;
        packetData[3] = sizeof(report);  // Payload length set to 1 for error code
        memcpy(packetData + Packet::PACKET_HEADER_LENGTH, &report, sizeof(report));       
        return packetData;
    }
};

#endif // SIMPLE_REPORT_PACKET_H
