#ifndef ACOUSEA_MKR1310_NODES_MODULECODE_H
#define ACOUSEA_MKR1310_NODES_MODULECODE_H

#include "ErrorHandler/ErrorHandler.h"

class ModuleCode {
public:
    // Enumeración de tipos de tag
    enum class TYPES : char {
        BATTERY = 'B',
        LOCATION = 'L',
        NETWORK = 'N',
        OPERATION_MODES = 'O',
        OPERATION_MODES_GRAPH = 'G',
        REPORTING = 'P',
        REAL_TIME_CLOCK = 'R',
        STORAGE = 'S',
        AMBIENT = 'T',
        PAM_MODULE = 'M',
        ICLISTEN_COMPLETE = 'i',
        ICLISTEN_STATUS = 's',
        ICLISTEN_LOGGING_CONFIG = 'l',
        ICLISTEN_STREAMING_CONFIG = 'c',
        ICLISTEN_RECORDING_STATS = 'r',
    };

    // Constructor para inicializar desde un tipo específico
    explicit ModuleCode(TYPES type);

    // Devuelve el valor del tipo como char
    char getValue() const;

    static ModuleCode::TYPES enumFromValue(unsigned char code);

    // Devuelve el tipo de tag basado en un valor
    static ModuleCode fromValue(unsigned char code);


    // Sobrecarga de operador de comparación
    bool operator==(const ModuleCode& other) const;

    bool operator!=(const ModuleCode& other) const;

    // Convierte el tipo a string para depuración
    [[nodiscard]] std::string toString() const;

private:
    TYPES type;

    // Devuelve todos los tipos de tags disponibles
    static constexpr std::array<TYPES, 8> getAllTagTypes() {
        return {TYPES::BATTERY, TYPES::LOCATION, TYPES::NETWORK,
                TYPES::OPERATION_MODES, TYPES::REPORTING,
                TYPES::REAL_TIME_CLOCK, TYPES::STORAGE, TYPES::AMBIENT};
    }
};

#endif // ACOUSEA_MKR1310_NODES_MODULECODE_H
