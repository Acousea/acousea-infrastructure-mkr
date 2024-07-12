#ifndef SIMPLE_REPORT_PACKET_H
#define SIMPLE_REPORT_PACKET_H

#include "Packet.h"
#include "../GPS/IGPS.h"
#include "../Services/SummaryService.h"

typedef struct SimpleReport {
    uint32_t epoch;
    GPSLocation location;        
    uint8_t battery_percentage;
    uint8_t battery_status_and_operation_mode;
    uint16_t reserved = 0; // 2 bytes of padding
} SimpleReport;


class SimpleReportPacket : public Packet {
public:
    
    SimpleReportPacket(SimpleReport report, Packet::PacketType packetType)
        : Packet(buildSimpleReportPacket(report, packetType), PACKET_HEADER_LENGTH + sizeof(report)) {}

private:
    static const uint8_t* buildSimpleReportPacket(SimpleReport report, Packet::PacketType packetType) {
        static uint8_t packetData[Packet::PACKET_HEADER_LENGTH + sizeof(report)];
        packetData[0] = Packet::SYNC_BYTE;
        packetData[1] = Packet::OpCode::SUMMARY_SIMPLE_REPORT;
        packetData[2] = SENDER(Packet::Address::DRIFTER) | RECEIVER(Packet::Address::BACKEND) | packetType;
        packetData[3] = sizeof(report);  // Payload length set to 1 for error code
        memcpy(packetData + Packet::PACKET_HEADER_LENGTH, &report, sizeof(report));       
        // Print packet data
        SerialUSB.print("PacketData BUILD_SIMPLE_REPORT: ");
        for (unsigned int i = 0; i < sizeof(packetData); i++) {
            SerialUSB.print(packetData[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();
        return packetData;
    }
};

#endif // SIMPLE_REPORT_PACKET_H
