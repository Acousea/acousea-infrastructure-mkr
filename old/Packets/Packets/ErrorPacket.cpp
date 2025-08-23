#include "ErrorPacket.h"

ErrorPacket::ErrorPacket(const RoutingChunk &routingChunk, ErrorCode errorCode)
        : Packet(OperationCode(OperationCode::Code::ERROR_REPORT),
                 routingChunk,
                 ErrorPayload(errorCode)) {}

ErrorPacket ErrorPacket::invalidOpcode(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_OPCODE);
}

ErrorPacket ErrorPacket::invalidPayload(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_PAYLOAD);
}

ErrorPacket ErrorPacket::invalidSenderAddress(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_SENDER_ADDRESS);
}

ErrorPacket ErrorPacket::invalidRecipientAddress(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_RECIPIENT_ADDRESS);
}

ErrorPacket ErrorPacket::invalidPacketLength(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_PACKET_LENGTH);
}

ErrorPacket ErrorPacket::invalidSyncByte(const RoutingChunk &routingChunk) {
    return ErrorPacket(routingChunk, ErrorCode::INVALID_SYNC_BYTE);
}
