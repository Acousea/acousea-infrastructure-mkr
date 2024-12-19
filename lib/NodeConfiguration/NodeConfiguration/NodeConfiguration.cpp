#include "NodeConfiguration.h"

NodeConfiguration::NodeConfiguration(const Address &localAddress,
                                     const std::optional<OperationModesGraphModule> &operationGraphModule,
                                     const std::optional<LoRaReportingModule> &loraModule,
                                     const std::optional<IridiumReportingModule> &iridiumModule) :
        localAddress(localAddress),
        operationGraphModule(operationGraphModule),
        loraModule(loraModule),
        iridiumModule(iridiumModule) {}


const std::optional<OperationModesGraphModule> &NodeConfiguration::getOperationGraphModule() const {
    return operationGraphModule;
}

const std::optional<LoRaReportingModule> &NodeConfiguration::getLoraModule() const {
    return loraModule;
}

const std::optional<IridiumReportingModule> &NodeConfiguration::getIridiumModule() const {
    return iridiumModule;
}

NodeConfiguration NodeConfiguration::getDefaultConfiguration() {
    return NodeConfiguration(
            Address(255),
            OperationModesGraphModule::from({{0, OperationModesGraphModule::Transition(0, 1)},}),
            LoRaReportingModule(
                    {{0, ReportingConfiguration(0, 15, ReportingConfiguration::ReportType::BASIC
                    )}}),
            IridiumReportingModule(
                    {{0, ReportingConfiguration(0, 15, ReportingConfiguration::ReportType::BASIC)}}
            )
    );
}

Result<NodeConfiguration> NodeConfiguration::fromJson(const std::string &json) {
    JsonDocument doc;  // Usamos JsonDocument sin especificar un tamaño fijo

    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return Result<NodeConfiguration>::failure("Failed to parse JSON: " + std::string(error.c_str()));
    }

    if (!doc.containsKey("localAddress")) {
        return Result<NodeConfiguration>::failure("Missing localAddress field");
    }

    Address localAddress = Address(doc["localAddress"].as<uint8_t>());

    std::optional<OperationModesGraphModule> operationGraph = std::nullopt;
    if (doc.containsKey("operationGraphModule")) {
        auto operationGraphArray = doc["operationGraphModule"].as<JsonArray>();
        operationGraph.emplace(OperationModesGraphModule::fromJSON(operationGraphArray));
    }

    std::optional<LoRaReportingModule> loraModule = std::nullopt;
    if (doc.containsKey("loraModule")) {
        auto loraArray = doc["loraModule"].as<JsonArray>();
        loraModule.emplace(LoRaReportingModule::fromJSON(loraArray));
    }

    std::optional<IridiumReportingModule> iridiumModule = std::nullopt;
    if (doc.containsKey("iridiumModule")) {
        auto iridiumArray = doc["iridiumModule"].as<JsonArray>();
        iridiumModule.emplace(IridiumReportingModule::fromJSON(iridiumArray));
    }

    return Result<NodeConfiguration>::success(
            NodeConfiguration(localAddress, operationGraph, loraModule, iridiumModule)
    );
}

std::string NodeConfiguration::toJson() const {
    JsonDocument doc;

    doc["localAddress"] = localAddress.getValue();

    if (operationGraphModule.has_value()) {
        JsonDocument operationGraphJson = operationGraphModule->toJson();
        doc["operationGraphModule"] = operationGraphJson.as<JsonArray>();
    }

    if (loraModule.has_value()) {
        JsonDocument loraJson = loraModule->toJson();
        doc["loraModule"] = loraJson.as<JsonArray>();
    }

    if (iridiumModule.has_value()) {
        JsonDocument iridiumJson = iridiumModule->toJson();
        doc["iridiumModule"] = iridiumJson.as<JsonArray>();
    }

    // Serializar el documento principal
    std::string result;
    serializeJson(doc, result);
    return result;
}

void NodeConfiguration::print() const {
    Serial.println("NodeConfiguration JSON Representation:");
    std::string jsonRepresentation = toJson();
    Serial.println(jsonRepresentation.c_str());
}

const Address &NodeConfiguration::getLocalAddress() const {
    return localAddress;
}

void NodeConfiguration::setLocalAddress(const Address &address) {
    this->localAddress = address;
}

void NodeConfiguration::setOperationGraphModule(const OperationModesGraphModule &module) {
    operationGraphModule.emplace(module); // Construye un nuevo objeto dentro del optional
}

void NodeConfiguration::setLoraModule(const LoRaReportingModule &module) {
    loraModule.emplace(module);
}

void NodeConfiguration::setIridiumModule(const IridiumReportingModule &module) {
    iridiumModule.emplace(module);
}

