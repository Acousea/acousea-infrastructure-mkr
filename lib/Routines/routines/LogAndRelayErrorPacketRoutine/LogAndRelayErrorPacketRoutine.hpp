#ifndef ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP


#include "IRoutine.h"
#include "Router.h"

#include  "bindings/nodeDevice.pb.h"

#include <vector>
#include <optional>

/**
 * @brief Routine that logs an error when an error packet is received
 */
class LogAndRelayErrorPacketRoutine final : public IRoutine<acousea_CommunicationPacket>
{
    Router& router;

public:
    CLASS_NAME(LogErrorRoutine)

    explicit LogAndRelayErrorPacketRoutine(Router& router);

    Result<acousea_CommunicationPacket*> execute(acousea_CommunicationPacket* optPacket) override;
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_RELAYPACKETROUTINE_HPP
