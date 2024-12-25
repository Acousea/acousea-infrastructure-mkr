#include "OperationModesGraphModule.h"

OperationModesGraphModule OperationModesGraphModule::from(const std::map<uint8_t, Transition> &graph) {
    return OperationModesGraphModule(graph);
}

OperationModesGraphModule OperationModesGraphModule::from(const std::vector<uint8_t> &value) {
    if (value.size() % 4 != 0 || value.size() < 4) {
        ErrorHandler::handleError("Invalid value size for OperationModesGraphModule deserialization");
    }
    return OperationModesGraphModule(value);
}

OperationModesGraphModule OperationModesGraphModule::fromJSON(const JsonArrayConst &doc) {
    std::map<uint8_t, Transition> graph;
    for (const auto &item: doc) {
        uint8_t currentMode = item["currentMode"];
        uint8_t nextMode = item["nextMode"];
        uint16_t duration = item["duration"];
        graph.insert_or_assign(currentMode, Transition(nextMode, duration));
    }
    return OperationModesGraphModule(graph);
}

JsonDocument OperationModesGraphModule::toJson() const {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (const auto &[currentMode, transition] : graph) {
        JsonObject obj = array.add<JsonObject>();
        obj["currentMode"] = currentMode;
        obj["nextMode"] = transition.nextMode;
        obj["duration"] = transition.duration;
    }

    return doc;
}

const std::map<uint8_t, OperationModesGraphModule::Transition> &OperationModesGraphModule::getGraph() const {
    return graph;
}

OperationModesGraphModule::OperationModesGraphModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::OPERATION_MODES_GRAPH, value) {
    for (size_t i = 0; i < value.size(); i += 4) {
        uint8_t currentMode = value[i];
        Transition transition = Transition::fromBytes(value, i + 1);
        graph.insert_or_assign(currentMode, transition);
    }
}

OperationModesGraphModule::OperationModesGraphModule(const std::map<uint8_t, Transition> &graph)
        : SerializableModule(ModuleCode::TYPES::OPERATION_MODES_GRAPH, serializeGraph(graph)), graph(graph) {}

std::vector<uint8_t> OperationModesGraphModule::serializeGraph(const std::map<uint8_t, Transition> &graph) {
    std::vector<uint8_t> value;
    for (const auto &[currentMode, transition]: graph) {
        value.push_back(currentMode);
        auto transitionBytes = transition.toBytes();
        value.insert(value.end(), transitionBytes.begin(), transitionBytes.end());
    }
    return value;
}
