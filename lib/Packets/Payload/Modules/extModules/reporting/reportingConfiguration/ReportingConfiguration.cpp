#include "ReportingConfiguration.h"

ReportingConfiguration::ReportType ReportingConfiguration::reportTypeFromJson(uint8_t value) {
    switch (value) {
        case 'C':
            return ReportingConfiguration::ReportType::COMPLETE;
        case 'B':
            return ReportingConfiguration::ReportType::BASIC;
        default:
            ErrorHandler::handleError("Invalid report type value " + std::to_string(value));
//                throw std::invalid_argument("Invalid report type value");
    }
}

ReportingConfiguration::ReportingConfiguration(uint8_t modeId, uint16_t period,
                                               ReportingConfiguration::ReportType reportType)
        : modeId(modeId), period(period), reportType(reportType) {}

uint8_t ReportingConfiguration::getModeId() const { return modeId; }

uint16_t ReportingConfiguration::getPeriod() const { return period; }

ReportingConfiguration::ReportType ReportingConfiguration::getReportType() const { return reportType; }

std::string ReportingConfiguration::getReportTypeString() const {
    switch (reportType) {
        case ReportType::COMPLETE:
            return "COMPLETE";
        case ReportType::BASIC:
            return "BASIC";
    }
    return "UNKNOWN";
}

std::vector<uint8_t> ReportingConfiguration::toBytes() const {
    return {
            modeId,
            static_cast<uint8_t>(period >> 8),
            static_cast<uint8_t>(period & 0xFF),
            static_cast<uint8_t>(reportType)
    };
}

ReportingConfiguration ReportingConfiguration::fromBytes(const std::vector<uint8_t> &data, size_t offset) {
    if (offset + 4 > data.size()) {
        ErrorHandler::handleError("Invalid data size for ReportingConfiguration");
//            throw std::invalid_argument("Invalid data size for ReportingConfiguration");
    }
    uint8_t modeId = data[offset];
    uint16_t period = (data[offset + 1] << 8) | data[offset + 2];
    ReportType reportType = static_cast<ReportType>(data[offset + 3]);
    return {modeId, period, reportType};
}
