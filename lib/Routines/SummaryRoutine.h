#ifndef SUMMARYROUTINE_H
#define SUMMARYROUTINE_H

#include "IRoutine.h"
#include "../Packet/Packet.h"
#include "../Packet/NullPacket.h"
#include "../Services/SummaryService.h"

/**
 * @brief This routine is used to receive a packet with a summary and upload it to the summary service
 * so that it can be retrieved by the corresponding operation mode that sends reports to the backend
 * It returns a NullPacket since it does not need to send any response to the sender
 */
class SummaryRoutine : public IRoutine {

    SummaryService* summaryService;

public:
    SummaryRoutine(SummaryService* summaryService): summaryService(summaryService) {}

    Packet execute(const Packet& packet) override {
        // Extract a Summary struct from the packet
        const uint8_t* payload = packet.getPayload();
        size_t payloadSize = packet.getPayloadLength();
        SerialUSB.println("Received summary packet size: " + String(payloadSize) + " bytes");
        SerialUSB.println("Sizeof Summary: " + String(sizeof(SummaryResponse)) + " bytes");
        if (payloadSize != sizeof(SummaryResponse)) {
            Serial.println("Invalid summary packet size: Expected " +
                             String(sizeof(SummaryResponse)) + " bytes. Specified " +
                             String(payloadSize) + " bytes.");
            return NullPacket();
        }

        SummaryResponse summary;
        memcpy(&summary, payload, sizeof(SummaryResponse));
        summaryService->setSummary(summary);
        return NullPacket();
    }

};

#endif // SUMMARYROUTINE_H
