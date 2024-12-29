
#ifndef ACOUSEA_MKR1310_NODES_REPORTINGCONFIGURATION_H
#define ACOUSEA_MKR1310_NODES_REPORTINGCONFIGURATION_H

#include <vector>
#include <cstdint>
#include "ErrorHandler/ErrorHandler.h"


class ReportingConfiguration {
public:
    enum class ReportType : unsigned char {
        COMPLETE = 'C',
        BASIC = 'B'
    };

    static ReportingConfiguration::ReportType reportTypeFromJson(uint8_t value);

    ReportingConfiguration(uint8_t modeId, uint16_t period, ReportType reportType);

    // Métodos de acceso
    [[nodiscard]] uint8_t getModeId() const;

    [[nodiscard]] uint16_t getPeriod() const;

    [[nodiscard]] ReportType getReportType() const;

    std::string getReportTypeString() const;

    // Serialización a bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const;

    // Deserialización desde bytes
    static ReportingConfiguration fromBytes(const std::vector<uint8_t> &data, size_t offset);

private:
    uint8_t modeId;         // ID del modo de operación
    uint16_t period;        // Periodo de reporte en segundos
    ReportType reportType;  // Tipo de reporte
};

#endif //ACOUSEA_MKR1310_NODES_REPORTINGCONFIGURATION_H
