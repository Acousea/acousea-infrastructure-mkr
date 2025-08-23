#include "FetchICListenConfigPacket.h"

#include <Payload/Payloads/iclisten/FetchICListenConfigurationPayload.h>


FetchICListenConfigPacket::FetchICListenConfigPacket(const RoutingChunk &routingChunk,
    const FetchICListenConfigurationPayload& payload
    ) : Packet(
    OperationCode(OperationCode::Code::GET_ICLISTEN_CONFIG),
    routingChunk,
    payload) {
}
