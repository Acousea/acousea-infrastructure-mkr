#include "OperationMode.h"

OperationMode::OperationMode(uint8_t id, std::string name)
        : id(id), name(std::move(name)) {}

OperationMode OperationMode::create(uint8_t id, const std::string &name) {
    return {id, name};
}
