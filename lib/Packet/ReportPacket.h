#ifndef REPORT_PACKET_H
#define REPORT_PACKET_H

#include "Packet.h"
#include "../GPS/IGPS.h"
#include "../Services/SummaryService.h"

typedef struct Report {
    time_t dateTimestamp;
    uint8_t operation_mode;
    GPSLocation location;
    Summary summary;
} Report;


class ReportPacket : public Packet {
public:
    
    ReportPacket(Report report, Packet::PacketType packetType)
        : Packet(buildReportPacket(report, packetType), PACKET_HEADER_LENGTH) {}

private:
    static const uint8_t* buildReportPacket(Report report, Packet::PacketType packetType) {
        static uint8_t packetData[Packet::PACKET_HEADER_LENGTH + sizeof(report)];
        packetData[0] = Packet::SYNC_BYTE;
        packetData[1] = Packet::OpCode::SUMMARY_REPORT;
        packetData[2] = SENDER(Packet::Address::DRIFTER) | RECEIVER(Packet::Address::BACKEND) | packetType;
        packetData[3] = sizeof(report);  // Payload length set to 1 for error code
        memcpy(packetData + Packet::PACKET_HEADER_LENGTH, &report, sizeof(report));       
        return packetData;
    }
};

#endif // REPORT_PACKET_H
