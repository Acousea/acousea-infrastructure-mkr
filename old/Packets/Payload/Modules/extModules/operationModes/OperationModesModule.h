#ifndef ACOUSEA_MKR1310_NODES_OPERATIONMODESMODULE_H
#define ACOUSEA_MKR1310_NODES_OPERATIONMODESMODULE_H


#include "OperationMode.h"
#include "Payload/Modules/SerializableModule.h"
#include "Payload/Modules/JSONSerializable.h"

class OperationModesModule : public SerializableModule, public JSONSerializable {
public:

    static OperationModesModule from(const std::map<uint8_t, OperationMode> &operationModes);

    static OperationModesModule from(const std::map<uint8_t, OperationMode> &operationModes, uint8_t activeModeIdx);


    static OperationModesModule from(const std::vector<uint8_t> &value);

    static OperationModesModule fromJSON(const JsonArrayConst &doc);

    [[nodiscard]] JsonDocument toJson() const override;

    [[nodiscard]] uint8_t getActiveModeIdx() const;


    [[nodiscard]] const std::map<uint8_t, OperationMode> &getOperationModes() const;


private:
    std::map<uint8_t, OperationMode> operationModes;
    uint8_t activeModeIdx;


    explicit OperationModesModule(const std::vector<uint8_t> &value);

    explicit OperationModesModule(const std::map<uint8_t, OperationMode> &operationModes, uint8_t activeModeIdx);


    static std::vector<uint8_t> serializeValues(
            const std::map<uint8_t, OperationMode> &operationModes,
            uint8_t activeModeIdx);
};

#endif 
