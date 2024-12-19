#ifndef ACOUSEA_MKR1310_NODES_OPERATIONMODE_H
#define ACOUSEA_MKR1310_NODES_OPERATIONMODE_H


#include <string>
#include <map>
#include <cstdint>
#include <vector>
#include <utility>

/**
 *  From an operation mode we will know:
 *  - The active technologies (LoRa, Iridium, ...) and their respective reporting periods
 *  - The kind of reports that are going to be sent
 *  - The duration of the operation mode (number of cycles)
 *  - A graph of flow between operation modes
 */
class OperationMode {
public:
    uint8_t id;
    std::string name;

    OperationMode(uint8_t id, std::string name);

    static OperationMode create(uint8_t id, const std::string &name);
};

#endif //ACOUSEA_MKR1310_NODES_OPERATIONMODE_H
