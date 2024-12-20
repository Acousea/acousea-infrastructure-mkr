#include "OperationCode.h"

OperationCode OperationCode::completeStatusReport() {
    return OperationCode(COMPLETE_STATUS_REPORT);
}

OperationCode OperationCode::basicStatusReport() {
    return OperationCode(BASIC_STATUS_REPORT);
}

OperationCode OperationCode::setNodeDeviceConfig() {
    return OperationCode(SET_NODE_DEVICE_CONFIG);
}

OperationCode OperationCode::getUpdatedNodeDeviceConfig() {
    return OperationCode(GET_UPDATED_NODE_DEVICE_CONFIG);
}

OperationCode OperationCode::errorReport() {
    return OperationCode(ERROR_REPORT);
}

OperationCode::OperationCode(OperationCode::Code code) : code(code) {}

char OperationCode::getValue() const {
    return code;
}

OperationCode OperationCode::fromValue(char value) {
    for (Code c : {COMPLETE_STATUS_REPORT, BASIC_STATUS_REPORT, SET_NODE_DEVICE_CONFIG, GET_UPDATED_NODE_DEVICE_CONFIG}) {
        if (c == value) {
            return OperationCode(c);
        }
    }
//        throw std::invalid_argument("Invalid operation code: " + std::string(1, value));
}
