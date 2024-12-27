#ifndef ACOUSEA_MKR1310_NODES_SERIALIZABLEMODULE_H
#define ACOUSEA_MKR1310_NODES_SERIALIZABLEMODULE_H


#include "Payload/Modules/moduleCode/ModuleCode.h"
#include <cstdint>
#include <vector>
#include <cstring>
#include <map>

class SerializableModule {
protected:
    const uint8_t TYPE; // Campo común para el tipo de parámetro
    const std::vector<uint8_t> VALUE; // Campo común para el valor del parámetro

public:
    SerializableModule(ModuleCode::TYPES type, const std::vector<uint8_t>& value)
            : TYPE(static_cast<uint8_t>(type)), VALUE(value) {}

    // Constructor de movimiento
    SerializableModule(SerializableModule&& other) noexcept
        : TYPE(std::move(other.TYPE)), VALUE(std::move(other.VALUE)) {}

    // Operador de asignación por movimiento
    SerializableModule& operator=(SerializableModule&& other) noexcept;

    // Constructor y operador de copia eliminados
    SerializableModule(const SerializableModule&) noexcept;

    SerializableModule& operator=(const SerializableModule&) noexcept;

    [[nodiscard]] uint8_t getType() const;

    [[nodiscard]] std::vector<uint8_t> toBytes() const;

    [[nodiscard]] int getFullLength() const;

    [[nodiscard]] std::string encode() const;

    // Virtual destructor to allow polymorphic destruction
    // virtual ~SerializableModule() = default;
};

#endif //ACOUSEA_MKR1310_NODES_SERIALIZABLEMODULE_H
