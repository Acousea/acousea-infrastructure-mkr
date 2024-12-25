#include "Packet.h"


Packet::Packet(const OperationCode &opCode, const RoutingChunk &routingChunk, PayloadVariant payload)
        : opCode(opCode), routingChunk(routingChunk), payload(std::move(payload)) {
    computeChecksum();
}

Packet::Packet(const OperationCode &opCode, const RoutingChunk &routingChunk, PayloadVariant payload, uint16_t checksum)
        : opCode(opCode), routingChunk(routingChunk), payload(std::move(payload)), checksum(checksum) {}

std::vector<uint8_t> Packet::toBytes() const {
    std::vector<uint8_t> buffer;
    buffer.push_back(SYNC_BYTE);
    buffer.push_back(static_cast<uint8_t>(opCode.getValue()));
    const auto routingBytes = routingChunk.toBytes();
    buffer.insert(buffer.end(), routingBytes.begin(), routingBytes.end());

    std::visit([&buffer](const auto &payload) {
        buffer.push_back(static_cast<uint8_t>(payload.getBytesSize()));
        const auto payloadBytes = payload.toBytes();
        buffer.insert(buffer.end(), payloadBytes.begin(), payloadBytes.end());
    }, payload);

    buffer.push_back(static_cast<uint8_t>(checksum >> 8));
    buffer.push_back(static_cast<uint8_t>(checksum & 0xFF));
    return buffer;
}

Packet Packet::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() < 7) { // SYNC_BYTE + OP_CODE + ROUTING_CHUNK (3 bytes) + PAYLOAD_LENGTH + CRC (2 bytes)
        ErrorHandler::handleError("Data length is insufficient for a valid Packets");
    }

    if (data[0] != SYNC_BYTE) {
        ErrorHandler::handleError("Invalid sync byte");
    }

    if (!CRCUtils::verifyCRC(data)) {
        ErrorHandler::handleError("CRC verification failed");
    }

    OperationCode opCode = OperationCode::fromValue(static_cast<char>(data[1]));
    RoutingChunk routingChunk = RoutingChunk::fromBytes({data.begin() + 2, data.begin() + 5});
    uint8_t payloadLength = data[5];
    if (data.size() < (unsigned int) 7 + payloadLength) {
        ErrorHandler::handleError("Payload length mismatch");
    }

    std::vector<uint8_t> payloadData(data.begin() + 6, data.begin() + 6 + payloadLength);
    uint16_t crc = (data[data.size() - 2] << 8) | data[data.size() - 1];

    switch (opCode.getValue()) {
        case OperationCode::Code::GET_UPDATED_NODE_DEVICE_CONFIG: {
            return {opCode, routingChunk, GetUpdatedNodeConfigurationPayload::fromBytes(payloadData), crc};
        }
        case OperationCode::Code::SET_NODE_DEVICE_CONFIG: {
            return {opCode, routingChunk, NewNodeConfigurationPayload::fromBytes(payloadData), crc};
        }
        default:
            ErrorHandler::handleError("Invalid operation code");
//                throw std::runtime_error("Invalid operation code");
    }
}

std::string Packet::encode() const {
    const auto packetBytes = toBytes();
    std::ostringstream oss;
    for (const auto &byte: packetBytes) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    return oss.str();
}

const OperationCode &Packet::getOpCode() const {
    return opCode;
}

OperationCode::Code Packet::getOpCodeEnum() const {
    return static_cast<OperationCode::Code>(opCode.getValue());
}

const RoutingChunk &Packet::getRoutingChunk() const {
    return routingChunk;
}

void Packet::setRoutingChunk(const RoutingChunk &chunk) {
    routingChunk = chunk;
}

const PayloadVariant &Packet::getPayload() const {
    return payload;
}

uint16_t Packet::getChecksum() const {
    return checksum;
}

void Packet::swapSenderReceiverAddresses() {
    routingChunk.swapSenderReceiverAddresses();
}

void Packet::computeChecksum() {
    std::vector<uint8_t> buffer;
    buffer.push_back(SYNC_BYTE);
    buffer.push_back(static_cast<uint8_t>(opCode.getValue()));
    const auto routingBytes = routingChunk.toBytes();
    buffer.insert(buffer.end(), routingBytes.begin(), routingBytes.end());

    std::visit([&buffer](const auto &payload) {
        buffer.push_back(static_cast<uint8_t>(payload.getBytesSize()));
        const auto payloadBytes = payload.toBytes();
        buffer.insert(buffer.end(), payloadBytes.begin(), payloadBytes.end());
    }, payload);

    checksum = CRCUtils::calculate16BitCRC(buffer);
}
