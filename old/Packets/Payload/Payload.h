#ifndef ACOUSEA_MKR1310_NODES_PAYLOAD_H
#define ACOUSEA_MKR1310_NODES_PAYLOAD_H

#include <vector>
#include <cstdint>
#include <vector>

class Payload {
public:
    virtual ~Payload() = default;

    // Devuelve el tamaño en bytes del packetPayload
    virtual uint16_t getBytesSize() const = 0;

    // Serializa el packetPayload a un vector de bytes
    virtual std::vector<uint8_t> toBytes() const = 0;
};

#endif //ACOUSEA_MKR1310_NODES_PAYLOAD_H
