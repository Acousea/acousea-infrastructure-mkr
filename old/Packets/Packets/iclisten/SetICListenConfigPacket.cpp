#include "SetICListenConfigPacket.h"


SetICListenConfigPacket::SetICListenConfigPacket(const RoutingChunk &routingChunk, SetICListenConfigurationPayload payload) :
        Packet(OperationCode(OperationCode::Code::SET_ICLISTEN_CONFIG),
               routingChunk,
               payload) {}



