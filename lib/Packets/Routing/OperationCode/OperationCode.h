#ifndef ACOUSEA_MKR1310_NODES_OPERATIONCODE_H
#define ACOUSEA_MKR1310_NODES_OPERATIONCODE_H


#include <string>
#include <vector>

class OperationCode {
public:
    enum Code : char {
        ERROR_REPORT = 'E',
        COMPLETE_STATUS_REPORT = 'S',
        BASIC_STATUS_REPORT = 's',
        SET_NODE_DEVICE_CONFIG = 'C',
        GET_UPDATED_NODE_DEVICE_CONFIG = 'U'
    };
    static OperationCode completeStatusReport();

    static OperationCode basicStatusReport();

    static OperationCode setNodeDeviceConfig();

    static OperationCode getUpdatedNodeDeviceConfig();

    static OperationCode errorReport();

    explicit OperationCode(Code code);

    [[nodiscard]] char getValue() const;

    static OperationCode fromValue(char value);

private:
    Code code;
};


#endif //ACOUSEA_MKR1310_NODES_OPERATIONCODE_H
