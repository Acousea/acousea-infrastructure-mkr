#ifndef API_SET_ICLISTENCONFIGPACKET_H
#define API_SET_ICLISTENCONFIGPACKET_H


#include "Packet.h"
#include "Payload/Modules/pamModules/ICListen/ICListenHF.h"
#include "Payload/Payloads/iclisten/SetICListenConfigurationPayload.h"

class SetICListenConfigPacket : public Packet {
public:
    SetICListenConfigPacket(const RoutingChunk &routingChunk, SetICListenConfigurationPayload payload);
};

#endif // API_SET_ICLISTENCONFIGPACKET_H
