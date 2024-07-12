#ifndef SUMMARYSERVICE_H
#define SUMMARYSERVICE_H

#include <Arduino.h>
#include "SummaryReportPacket.h"
#include "IGPS.h"

typedef struct {    
    PAMDeviceStats pam_device;
    AudioDetectionStats audio_detection;
    StorageStats pi3_storage;    
    uint8_t pi3_temperature;
} SummaryResponse;
    
/**
 * @brief Service that provides a summary of the data collected by the device
 * It stores a summary object that represents the summary provided by the raspberry pi
 * It will have a method to check if a new summary is available
 * It will have a method to get the summary, which whill also delete the current summary
 */
class SummaryService {
private:
    SummaryResponse summary;
    bool available;

public:
    SummaryService() :  available(false) {}

    /**
     * @brief Check if a new summary is available
     * @return true if a new summary is available, false otherwise
     */
    bool newSummaryAvailable() const {
        return available;
    }

    /**
     * @brief Get the summary and delete the current summary
     * @return the summary object
     */
    SummaryResponse popSummary() {
        available = false;
        return summary;
    }

    /**
     * @brief Set the summary
     * @param newSummary the new summary to set
     */
    void setSummary(const SummaryResponse& newSummary) {
        summary = newSummary;       
        // Marcar el nuevo resumen como disponible
        available = true;
    }
};

#endif // SUMMARYSERVICE_H