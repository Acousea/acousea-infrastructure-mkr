#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP

#include <cstdint>

/**
 * @brief Interfaz común para colas de paquetes binarios.
 *
 * Las implementaciones concretas (FlashPacketQueue, SDPacketQueue, etc.)
 * se encargan del almacenamiento subyacente.
 */
class PacketQueue
{
public:
    virtual ~PacketQueue() = default;

    /**
     * @brief Inicializa el backend (memoria flash, SD, etc.)
     * @return true si la inicialización fue exitosa.
     */
    virtual bool begin() = 0;

    /**
     * @brief Indica si la cola está completamente vacía.
     */
    virtual bool isEmpty() const = 0;

    /**
     * @brief Indica si no hay paquetes pendientes para un puerto concreto.
     * @param targetPortType Tipo de puerto (PortType codificado como uint8_t)
     */
    virtual bool isEmptyForPort(uint8_t targetPortType) = 0;

    /**
     * @brief Inserta un nuevo paquete en la cola.
     * @param portType Código del puerto que lo genera.
     * @param data Puntero al buffer de datos (ya codificados, por ejemplo Protobuf).
     * @param length Longitud del buffer.
     * @return true si la inserción fue exitosa.
     */
    virtual bool push(uint8_t portType, const uint8_t* data, uint16_t length) = 0;

    /**
     * @brief Extrae el primer paquete de la cola, sin filtrar por puerto.
     * @param outBuffer Buffer destino.
     * @param maxSize Tamaño máximo del buffer destino.
     * @return Número de bytes leídos (0 si vacío o error).
     */
    virtual uint16_t popAny(uint8_t* outBuffer, uint16_t maxSize) = 0;

    /**
     * @brief Extrae el primer paquete perteneciente a un puerto específico.
     * @param targetPortType Tipo de puerto a buscar.
     * @param outBuffer Buffer destino donde se almacenará el paquete.
     * @param maxSize Tamaño máximo del buffer destino.
     * @return Longitud del paquete leído (0 si no hay ninguno).
     */
    virtual uint16_t popForPort(uint8_t targetPortType, uint8_t* outBuffer, uint16_t maxSize) = 0;

    /**
     * @brief Borra completamente la cola (todos los paquetes).
     */
    virtual void clear() = 0;
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_PACKETQUEUE_HPP
