#include "ReportingModule.h"

ReportingModule::ReportingModule(ReportingModule::TechnologyType type,
                                 const std::map<uint8_t, ReportingConfiguration> &configurations)
        : SerializableModule(ModuleCode::TYPES::REPORTING, serializeValues(type, configurations)),
          technologyId(static_cast<uint8_t>(type)),
          configurations(configurations) {}

ReportingModule::TechnologyType ReportingModule::getTechnologyType() const {
    return static_cast<TechnologyType>(technologyId);
}

const std::map<uint8_t, ReportingConfiguration> &ReportingModule::getConfigurations() const {
    return configurations;
}

void ReportingModule::setConfiguration(uint8_t modeId, const ReportingConfiguration &config) {
    configurations.emplace(modeId, config);
}

ReportingModule ReportingModule::from(const std::vector<uint8_t> &data) {
    if (data.empty()) {
        ErrorHandler::handleError("Invalid data size for ReportingModule");
//            throw std::invalid_argument("Invalid data size for ReportingModule");
    }
    TechnologyType type = static_cast<TechnologyType>(data[0]);
    std::map<uint8_t, ReportingConfiguration> configs;

    for (size_t i = 1; i + 4 <= data.size(); i += 4) {
        ReportingConfiguration config = ReportingConfiguration::fromBytes(data, i);
        configs.emplace(config.getModeId(), config);
    }

    return ReportingModule(type, configs);
}

std::vector<uint8_t> ReportingModule::serializeValues(ReportingModule::TechnologyType type,
                                                      const std::map<uint8_t, ReportingConfiguration> &configs) {
    std::vector<uint8_t> value = {static_cast<uint8_t>(type)};
    for (const auto &[modeId, config]: configs) {
        const auto configBytes = config.toBytes();
        value.insert(value.end(), configBytes.begin(), configBytes.end());
    }
    return value;
}
