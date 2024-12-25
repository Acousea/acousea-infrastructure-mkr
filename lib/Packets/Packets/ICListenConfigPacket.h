#ifndef API_ICLISTENCONFIGPACKET_H
#define API_ICLISTENCONFIGPACKET_H


#include "../Packet.h"
#include "../Payload/Modules/pamModules/ICListen/ICListenHF.h"
#include "../Payload/Payloads/iclisten/ICListenCompleteStatusPayload.h"

class ICListenConfigPacket : public Packet {
public:
    ICListenConfigPacket(const RoutingChunk &routingChunk,
                         const ICListenHF &icListenHF);
};

#endif //API_ICLISTENCONFIGPACKET_H
