#include "NodeConfigurationRepository.h"

NodeConfigurationRepository::NodeConfigurationRepository(SDManager &sdManager, const char *filePath)
        : sdManager(sdManager), filePath(filePath) {}

bool NodeConfigurationRepository::begin() {
    if (sdManager.readFile(filePath).isEmpty()) {
        Serial.println("NodeConfigurationRepository::begin() -> No configuration file found. Creating default configuration.");
        if (!saveConfiguration(NodeConfiguration::getDefaultConfiguration())) {
            Serial.println("NodeConfigurationRepository::begin() -> Error saving default configuration.");
            return false;
        }
    }
    printData();
    Serial.println("NodeConfigurationRepository initialized.");
    return true;
}

bool NodeConfigurationRepository::reset() {
    Serial.println("NodeConfigurationRepository::reset() -> Resetting to default configuration.");
    return saveConfiguration(NodeConfiguration::getDefaultConfiguration());
}

void NodeConfigurationRepository::printData() const {
    NodeConfiguration configuration = getNodeConfiguration();
    Serial.println("NodeConfigurationRepository::printData() -> Current configuration:");
    configuration.print();
}

NodeConfiguration NodeConfigurationRepository::getNodeConfiguration() const {
    if (sdManager.readFile(filePath).isEmpty()) {
        Serial.println("NodeConfigurationRepository::getNodeConfiguration() -> No configuration file found. Returning default configuration.");
        return NodeConfiguration::getDefaultConfiguration();
    }

    String content = sdManager.readFile(filePath);
    if (content.isEmpty()) {
        Serial.println("NodeConfigurationRepository::getNodeConfiguration() -> Empty configuration file. Returning default configuration.");
        return NodeConfiguration::getDefaultConfiguration();
    }

    auto result = NodeConfiguration::fromJson(content.c_str());
    if (result.isError()) {
        Serial.println("NodeConfigurationRepository::getNodeConfiguration() -> Error parsing configuration. Returning default configuration.");
        Serial.println(result.getError().c_str());
        return NodeConfiguration::getDefaultConfiguration();
    }

    return result.getValue();
}

bool NodeConfigurationRepository::saveConfiguration(const NodeConfiguration &configuration) {
    std::string json = configuration.toJson();
    if (!sdManager.overwriteFile(filePath, json.c_str())) {
        Serial.println("NodeConfigurationRepository::saveConfiguration() -> Error saving configuration to file.");
        return false;
    }
    return true;
}

