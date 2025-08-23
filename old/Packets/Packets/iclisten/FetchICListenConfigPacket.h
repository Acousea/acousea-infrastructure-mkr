#ifndef API_ICLISTENCONFIGPACKET_H
#define API_ICLISTENCONFIGPACKET_H
#include <Packet.h>


class FetchICListenConfigPacket : public Packet {
public:
    FetchICListenConfigPacket(const RoutingChunk &routingChunk, const FetchICListenConfigurationPayload& payload);
};

#endif //API_ICLISTENCONFIGPACKET_H
