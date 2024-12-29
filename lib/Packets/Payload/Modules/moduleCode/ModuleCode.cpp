#include "ModuleCode.h"

std::string ModuleCode::toString() const {
    std::string typeStr;
    switch (type) {
        case TYPES::BATTERY:
            typeStr = "BATTERY";
            break;
        case TYPES::LOCATION:
            typeStr = "LOCATION";
            break;
        case TYPES::NETWORK:
            typeStr = "NETWORK";
            break;
        case TYPES::OPERATION_MODES:
            typeStr = "OPERATION_MODES";
            break;
        case TYPES::OPERATION_MODES_GRAPH:
            typeStr = "OPERATION_MODES_GRAPH";
            break;
        case TYPES::REPORTING:
            typeStr = "REPORTING";
            break;
        case TYPES::REAL_TIME_CLOCK:
            typeStr = "REAL_TIME_CLOCK";
            break;
        case TYPES::STORAGE:
            typeStr = "STORAGE";
            break;
        case TYPES::AMBIENT:
            typeStr = "AMBIENT";
            break;
        case TYPES::PAM_MODULE:
            typeStr = "PAM_MODULE";
            break;
        case TYPES::ICLISTEN_COMPLETE:
            typeStr = "ICLISTEN_COMPLETE";
            break;
        case TYPES::ICLISTEN_LOGGING_CONFIG:
            typeStr = "ICLISTEN_LOGGING";
            break;
        case TYPES::ICLISTEN_RECORDING_STATS:
            typeStr = "ICLISTEN_RECORDING_STATS";
            break;
        case TYPES::ICLISTEN_STATUS:
            typeStr = "ICLISTEN_STATUS";
            break;
        case TYPES::ICLISTEN_STREAMING_CONFIG:
            typeStr = "ICLISTEN_STREAMING_CONFIG";
            break;
        default:
            typeStr = "UNKNOWN";
            break;
    }
    return typeStr;
}

bool ModuleCode::operator!=(const ModuleCode &other) const {
    return type != other.type;
}

bool ModuleCode::operator==(const ModuleCode &other) const {
    return type == other.type;
}


ModuleCode ModuleCode::fromValue(unsigned char code) {
    char charCode = static_cast<char>(code);
    for (const auto& tagType : getAllTagTypes()) {
        if (static_cast<char>(tagType) == charCode) {
            return ModuleCode(tagType);
        }
    }
    // Manejo de error: reinicia el programa o toma la acción necesaria
    ErrorHandler::handleError("Invalid module code: " + std::to_string(code));
//        throw std::invalid_argument("Invalid tag type: " + std::to_string(code));
}

ModuleCode::TYPES ModuleCode::enumFromValue(unsigned char code) {
    for (const auto& tagType : getAllTagTypes()) {
        if (static_cast<char>(tagType) == code) {
            return tagType;
        }
    }
    ErrorHandler::handleError("Invalid tag type: " + std::to_string(code));
//        throw std::invalid_argument("Invalid tag type: " + std::to_string(code));
}

char ModuleCode::getValue() const {
    return static_cast<char>(type);
}

ModuleCode::ModuleCode(ModuleCode::TYPES type) : type(type) {}


