#ifndef REPORTRROUTINE_H
#define REPORTRROUTINE_H

#include "IRoutine.h"
#include "../Packet/Packet.h"
#include "../Packet/NullPacket.h"
#include "../Services/ReportService.h"

/**
 * @brief This routine is used to receive a packet with a summary report and upload it to the report service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sender
 */
class ReportRoutine : public IRoutine {

    ReportService* reportService;

public:
    ReportRoutine(ReportService* reportService): reportService(reportService) {}

    Packet execute(const Packet& packet) override {
        // Extract a SummaryReport struct from the packet
        const uint8_t* payload = packet.getPayload();
        size_t payloadSize = packet.getPayloadLength();
        SerialUSB.println("Received report packet size: " + String(payloadSize) + " bytes");
        SerialUSB.println("Sizeof SummaryReport: " + String(sizeof(SummaryReport)) + " bytes");
        if (payloadSize != sizeof(SummaryReport)) {
            Serial.println("Invalid report packet size: Expected " +
                             String(sizeof(SummaryReport)) + " bytes. Specified " +
                             String(payloadSize) + " bytes.");
            return NullPacket();
        }

        SummaryReport report;
        memcpy(&report, payload, sizeof(SummaryReport));
        reportService->setReport(report);
        return NullPacket();
    }

};

#endif // REPORTRROUTINE_H
