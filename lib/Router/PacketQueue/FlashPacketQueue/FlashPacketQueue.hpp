#ifndef ACOUSEA_INFRASTRUCTURE_MKR_FLASHPACKETQUEUE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_FLASHPACKETQUEUE_HPP

#include <cstdint>
#include <ClassName.h>

class FlashPacketQueue
{
    CLASS_NAME(FlashPacketQueue)

public:
    FlashPacketQueue(uint32_t baseAddr, uint32_t size);

    bool begin();

    [[nodiscard]] bool isEmpty() const;

    // ------------------------------------------------------------
    // Comprueba si hay al menos un paquete pendiente para un PortType
    // ------------------------------------------------------------
    [[nodiscard]] bool isEmptyForPort(uint8_t targetPortType);


    void clear();

    // ------------------------------------------------------------
    // Inserta un paquete (asociado a un PortType)
    // ------------------------------------------------------------
    [[nodiscard]] bool push(const uint8_t portType, const uint8_t* data, uint16_t length);
    // ------------------------------------------------------------
    // pop(): devuelve el siguiente paquete (de cualquier puerto)
    // Retorna la longitud real leída (0 = vacío o error)
    // ------------------------------------------------------------
    uint16_t popAny(uint8_t* outBuffer, uint16_t maxSize);

    // popForPort(): busca secuencialmente hasta encontrar un paquete con PortType coincidente
    uint16_t popForPort(uint8_t targetPortType, uint8_t* outBuffer, uint16_t maxSize);

private:
    const uint32_t base;
    const uint32_t capacity;

    uint32_t head = 0;
    uint32_t tail = 0;

    [[nodiscard]] uint32_t advance(uint32_t pos, uint32_t amount) const;

    void writeWrapped(const uint8_t* data, uint32_t len) const;

    void readWrapped(uint8_t* out, uint32_t len);

    // Lee datos desde una posición arbitraria (sin mover tail)
    void readWrappedAt(uint32_t pos, uint8_t* out, uint32_t len);
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_FLASHPACKETQUEUE_HPP
