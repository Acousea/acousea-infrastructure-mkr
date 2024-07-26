#ifndef SIMPLEREPORTSERVICE_H
#define SIMPLEREPORTSERVICE_H

#include <Arduino.h>
#include "SimpleReportPacket.h"

class SimpleReportService {
private:
    SimpleReport report;
    bool available;

public:
    SimpleReportService() : available(false) {}

    /**
     * @brief Check if a new report is available
     * @return true if a new report is available, false otherwise
     */
    bool newReportAvailable() const {
        return available;
    }

    /**
     * @brief Get the report and delete the current report
     * @return the report object
     */
    SimpleReport popReport() {
        available = false;
        return report;
    }

    /**
     * @brief Set the report
     * @param newReport the new report to set
     */
    void setReport(const SimpleReport& newReport) {
        report = newReport;
        // Mark the new report as available
        available = true;
    }
};

#endif // SIMPLEREPORTSERVICE_H
