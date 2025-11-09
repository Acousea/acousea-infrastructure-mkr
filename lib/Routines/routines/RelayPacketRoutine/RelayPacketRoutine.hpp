#ifndef ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP


#include "IRoutine.h"
#include "Router.h"

#include  "bindings/nodeDevice.pb.h"

#include <vector>
#include <optional>

/**
 * @brief Rutina responsable de reenviar paquetes no destinados a este nodo
 *        hacia otros puertos especificados (relay).
 */
class RelayPacketRoutine final : public IRoutine<acousea_CommunicationPacket>
{
private:
    Router& router;
    std::vector<IPort::PortType> relayPorts;

public:
    CLASS_NAME(RelayPacketRoutine)

    explicit RelayPacketRoutine(Router& router,
                                const std::vector<IPort::PortType>& relayPorts);

    Result<acousea_CommunicationPacket> execute(
        const std::optional<acousea_CommunicationPacket>& optPacket) override;
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP
