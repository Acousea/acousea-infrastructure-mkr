#include "LoRaReportingPeriodModule.h"

LoRaReportingModule::LoRaReportingModule(const std::map<uint8_t, ReportingConfiguration> &configs)
        : ReportingModule(TechnologyType::LORA, configs) {}

LoRaReportingModule LoRaReportingModule::fromJSON(const JsonArrayConst &doc) {
    std::map<uint8_t, ReportingConfiguration> configs;
    for (const auto &item : doc) {
        uint8_t mode = item["mode"];
        uint16_t period = item["period"];
        auto reportType = ReportingConfiguration::reportTypeFromJson(item["reportType"].as<unsigned char>());
        configs.emplace(mode, ReportingConfiguration(mode, period, reportType));
    }
    return LoRaReportingModule(configs);
}

JsonDocument LoRaReportingModule::toJson() const {
    JsonDocument doc; // Nuevo JsonDocument
    JsonArray array = doc.to<JsonArray>();

    for (const auto &[modeId, config] : getConfigurations()) {
        JsonObject obj = array.add<JsonObject>();
        obj["mode"] = config.getModeId();
        obj["period"] = config.getPeriod();
        obj["reportType"] = static_cast<unsigned char>(config.getReportType());
    }

    return doc; // Devolvemos el documento completo
}
