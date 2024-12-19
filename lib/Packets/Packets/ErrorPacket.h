#ifndef ERROR_PACKET_H
#define ERROR_PACKET_H

#include "Packet.h"
#include "Payload/Payloads/error/ErrorPayload.h"
#include "Routing/ErrorCode/ErrorCode.h"

#include "Packet.h"
#include "Payload/Payloads/error/ErrorPayload.h"
#include "Routing/ErrorCode/ErrorCode.h"

class ErrorPacket : public Packet {
public:
    ErrorPacket(const RoutingChunk &routingChunk, ErrorCode errorCode);

    // Métodos estáticos para crear ErrorPackets con códigos de error específicos
    static ErrorPacket invalidOpcode(const RoutingChunk &routingChunk);

    static ErrorPacket invalidPayload(const RoutingChunk &routingChunk);

    static ErrorPacket invalidSenderAddress(const RoutingChunk &routingChunk);

    static ErrorPacket invalidRecipientAddress(const RoutingChunk &routingChunk);

    static ErrorPacket invalidPacketLength(const RoutingChunk &routingChunk);

    static ErrorPacket invalidSyncByte(const RoutingChunk &routingChunk);
};


#endif // ERROR_PACKET_H
