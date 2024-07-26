#ifndef REPORTSERVICE_H
#define REPORTSERVICE_H

#include <Arduino.h>
#include "SummaryReportPacket.h"
#include "IGPS.h"

class ReportService {
private:
    SummaryReport report;
    bool available;

public:
    ReportService() : available(false) {}

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
    SummaryReport popReport() {
        available = false;
        return report;
    }

    /**
     * @brief Set the report
     * @param newReport the new report to set
     */
    void setReport(const SummaryReport& newReport) {
        report = newReport;
        // Marcar el nuevo reporte como disponible
        available = true;
    }
};

#endif // REPORTSERVICE_H
