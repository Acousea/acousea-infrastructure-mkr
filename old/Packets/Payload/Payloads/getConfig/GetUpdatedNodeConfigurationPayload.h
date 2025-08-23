#ifndef ACOUSEA_MKR1310_NODES_GETUPDATEDNODECONFIGURATIONPAYLOAD_H
#define ACOUSEA_MKR1310_NODES_GETUPDATEDNODECONFIGURATIONPAYLOAD_H

#include "Payload/Payload.h"
#include "Payload/Modules/moduleCode/ModuleCode.h"

class GetUpdatedNodeConfigurationPayload : public Payload {
public:
    explicit GetUpdatedNodeConfigurationPayload(const std::vector<ModuleCode> &moduleCodes);

    // Obtiene el tamaño en bytes del packetPayload
    [[nodiscard]] uint16_t getBytesSize() const override;

    // Serializa el packetPayload a un vector de bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    // Construye un packetPayload desde un vector de bytes
    static GetUpdatedNodeConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

private:
    std::vector<ModuleCode> moduleCodes;
};


#endif //ACOUSEA_MKR1310_NODES_GETUPDATEDNODECONFIGURATIONPAYLOAD_H
