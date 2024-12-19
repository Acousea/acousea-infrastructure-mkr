#ifndef ACOUSEA_MKR1310_NODES_OPERATIONMODESGRAPHMODULE_H
#define ACOUSEA_MKR1310_NODES_OPERATIONMODESGRAPHMODULE_H

#include <vector>
#include <cstdint>
#include <map>

#include "Payload/Modules/JSONSerializable.h"
#include "Payload/Modules/SerializableModule.h"


class OperationModesGraphModule : public SerializableModule, public JSONSerializable {
public:
    // Clase interna Transition
    class Transition {
    public:
        uint8_t nextMode;
        uint16_t duration;

        Transition(uint8_t nextMode, uint16_t duration)
                : nextMode(nextMode), duration(duration) {}

        static Transition fromBytes(const std::vector<uint8_t> &value, size_t offset) {
            if (offset + 3 > value.size()) {
                ErrorHandler::handleError("Invalid offset for Transition deserialization");
//                throw std::runtime_error("Invalid offset for Transition deserialization");
            }
            uint8_t nextMode = value[offset];
            uint16_t duration = (value[offset + 1] << 8) | value[offset + 2];
            return Transition(nextMode, duration);
        }

        [[nodiscard]] std::vector<uint8_t> toBytes() const {
            std::vector<uint8_t> bytes;
            bytes.push_back(nextMode);
            bytes.push_back(static_cast<uint8_t>(duration >> 8)); // High byte
            bytes.push_back(static_cast<uint8_t>(duration & 0xFF)); // Low byte
            return bytes;
        }
    };

    // Métodos estáticos de creación
    static OperationModesGraphModule from(const std::map<uint8_t, Transition> &graph);

    static OperationModesGraphModule from(const std::vector<uint8_t> &value);

    static OperationModesGraphModule fromJSON(const JsonArrayConst &doc);


    [[nodiscard]] JsonDocument toJson() const override;



    // Getter del grafo
    [[nodiscard]] const std::map<uint8_t, Transition> &getGraph() const;

private:
    std::map<uint8_t, Transition> graph; // Nodo actual -> Transition (Nodo siguiente y duración)

    // Constructor desde bytes
    explicit OperationModesGraphModule(const std::vector<uint8_t> &value);

    // Constructor desde grafo
    explicit OperationModesGraphModule(const std::map<uint8_t, Transition> &graph);

    // Serialización del grafo a bytes
    static std::vector<uint8_t> serializeGraph(const std::map<uint8_t, Transition> &graph);
};

#endif // ACOUSEA_MKR1310_NODES_OPERATIONMODESGRAPHMODULE_H
