#ifndef SIMPLEREPORTRROUTINE_H
#define SIMPLEREPORTRROUTINE_H

#include "IRoutine.h"
#include "../Packet/Packet.h"
#include "../Packet/NullPacket.h"
#include "../Services/SimpleReportService.h"

/**
 * @brief This routine is used to receive a packet with a simple report and upload it to the report service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sender
 */
class SimpleReportRoutine : public IRoutine {

    SimpleReportService* reportService;

public:
    SimpleReportRoutine(SimpleReportService* reportService): reportService(reportService) {}

    Packet execute(const Packet& packet) override {
        // Extract a SimpleReport struct from the packet
        const uint8_t* payload = packet.getPayload();
        size_t payloadSize = packet.getPayloadLength();
        SerialUSB.println("Received simple report packet size: " + String(payloadSize) + " bytes");
        SerialUSB.println("Sizeof SimpleReport: " + String(sizeof(SimpleReport)) + " bytes");
        if (payloadSize != sizeof(SimpleReport)) {
            Serial.println("Invalid simple report packet size: Expected " +
                             String(sizeof(SimpleReport)) + " bytes. Specified " +
                             String(payloadSize) + " bytes.");
            return NullPacket();
        }

        SimpleReport report;
        memcpy(&report, payload, sizeof(SimpleReport));
        reportService->setReport(report);
        return NullPacket();
    }

};

#endif // SIMPLEREPORTRROUTINE_H
