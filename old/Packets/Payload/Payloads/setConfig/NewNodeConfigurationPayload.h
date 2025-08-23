#ifndef ACOUSEA_MKR1310_NODES_NEWNODECONFIGURATIONPAYLOAD_H
#define ACOUSEA_MKR1310_NODES_NEWNODECONFIGURATIONPAYLOAD_H

#include "Payload/Payload.h"
#include "Payload/Modules/factory/ModuleFactory.h"


class NewNodeConfigurationPayload : public Payload {
public:
    explicit NewNodeConfigurationPayload(const std::vector<std::unique_ptr<SerializableModule>> & vector);


    // Obtiene el tamaño en bytes del packetPayload
    [[nodiscard]] uint16_t getBytesSize() const override;

    // Serializa el packetPayload a un vector de bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const override;

    // Construye un packetPayload desde un vector de bytes
    static NewNodeConfigurationPayload fromBytes(const std::vector<uint8_t> &data);

    [[nodiscard]] const std::vector<SerializableModule> &getModules() const;

private:
    std::vector<SerializableModule> modules;
};

#endif //ACOUSEA_MKR1310_NODES_NEWNODECONFIGURATIONPAYLOAD_H
