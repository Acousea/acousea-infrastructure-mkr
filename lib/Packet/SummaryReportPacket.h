#ifndef REPORT_PACKET_H
#define REPORT_PACKET_H

#include "Packet.h"
#include "../GPS/IGPS.h"
#include "../Services/SummaryService.h"

// WARNING: Struct sizeof returns a multiple of 4 bytes (memory alignment)
typedef struct {
    uint32_t used_storage_mb;
    uint32_t total_storage_mb;
} StorageStats;

typedef struct {
    uint16_t totalNumDetections;        
    uint16_t recordedMinutes;
    uint16_t processedMinutes;
    uint16_t numFiles;
} AudioDetectionStats;

typedef struct {
    uint8_t UnitStatus_BatteryStatus;
    uint8_t BatteryPercentage;
    uint8_t temperature;
    uint8_t humidity;    
}  PAMDeviceStats;

typedef struct {
    uint8_t temperature;
    uint8_t BatteryPercentage;
    uint8_t BatteryStatus_OpMode;
    uint8_t reserved = 0; // 1 byte of padding
    GPSLocation location;
    StorageStats pi3_storage;
} DrifterModuleStats;

typedef struct {
    uint32_t epoch_time;
    PAMDeviceStats pam_device;
    DrifterModuleStats drifter_module_stats;
    AudioDetectionStats audio_detection;
} SummaryReport;


class SummaryReportPacket : public Packet {
public:
    
    SummaryReportPacket(SummaryReport report, Packet::PacketType packetType)
        : Packet(buildReportPacket(report, packetType), PACKET_HEADER_LENGTH + sizeof(report)) {}

private:
    static const uint8_t* buildReportPacket(SummaryReport report, Packet::PacketType packetType) {
        static uint8_t packetData[Packet::PACKET_HEADER_LENGTH + sizeof(report)];
        packetData[0] = Packet::SYNC_BYTE;
        packetData[1] = Packet::OpCode::SUMMARY_REPORT;
        packetData[2] = SENDER(Packet::Address::DRIFTER) | RECEIVER(Packet::Address::BACKEND) | packetType;
        packetData[3] = sizeof(report);  // Payload length set to 1 for error code
        memcpy(packetData + Packet::PACKET_HEADER_LENGTH, &report, sizeof(report));  
        // Print packet data
        SerialUSB.print("PacketData BUILD_SUMMARY_REPORT: ");
        for (unsigned int i = 0; i < sizeof(packetData); i++) {
            SerialUSB.print(packetData[i], HEX);
            SerialUSB.print(" ");
        }
        SerialUSB.println();     
        return packetData;
    }
};

#endif // REPORT_PACKET_H
