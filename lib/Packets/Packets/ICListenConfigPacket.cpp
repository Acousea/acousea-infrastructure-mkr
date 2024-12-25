#include "ICListenConfigPacket.h"


ICListenConfigPacket::ICListenConfigPacket(const RoutingChunk &routingChunk, const ICListenHF &icListenHF) :
        Packet(OperationCode(OperationCode::Code::ICLISTEN_CONFIG),
               routingChunk,
               ICListenCompleteStatusPayload(icListenHF)) {}



