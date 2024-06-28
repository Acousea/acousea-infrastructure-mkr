#ifndef SUMMARYSERVICE_H
#define SUMMARYSERVICE_H

#include <Arduino.h>

// WARNING: Struct sizeof returns a multiple of 4 bytes (memory alignment)
typedef struct {
    uint16_t numClicks;
    uint16_t recordedMinutes;
    uint16_t numFiles;
    uint16_t reserved;
} Summary;

/**
 * @brief Service that provides a summary of the data collected by the device
 * It stores a summary object that represents the summary provided by the raspberry pi
 * It will have a method to check if a new summary is available
 * It will have a method to get the summary, which whill also delete the current summary
 */
class SummaryService
{

private:
    Summary summary;
    bool available = false;

public:
    SummaryService() : summary({0, 0, 0, 0}) {}

    /**
     * @brief Check if a new summary is available
     * @return true if a new summary is available, false otherwise
     */
    bool newSummaryAvailable()
    {
        return available;
    }

    /**
     * @brief Get the summary and delete the current summary
     * @return the summary object
     */
    Summary popSummary()
    {
        Summary temp = summary;
        summary = {0, 0, 0, 0};
        available = false;
        return temp;
    }

    /**
     * @brief Set the summary
     * @param newSummary the new summary to set
     */
    void setSummary(Summary newSummary)
    {
        summary = newSummary;
        available = true;
    }
};

#endif // SUMMARYSERVICE_H