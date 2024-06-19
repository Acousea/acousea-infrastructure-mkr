#include "Packet.h"

// Constructor
Packet::Packet(const uint8_t* data, size_t length) {
    if (length < 4) {
        throw std::invalid_argument("Data length is too short");
    }
    syncByte = data[0];
    address = data[1];
    
    opCode = data[2];
    payloadLength = data[3];

    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        throw std::invalid_argument("Payload length is too long");
    }

    if (length < (size_t) (PACKET_HEADER_LENGTH + payloadLength)) {
        throw std::invalid_argument("Data length does not match payload length");
    }

    memcpy(payload, data + 4, payloadLength);
}

bool Packet::isValidAddress(uint8_t address) {
    return address == Address::BACKEND || address == Address::LOCALIZER || address == Address::DRIFTER || address == Address::PI3;
}

// Getters
uint8_t Packet::getSyncByte() const {
    return syncByte;
}

uint8_t Packet::getAddress() const {
    return address;
}

uint8_t Packet::getOpCode() const {
    return opCode;
}

uint8_t Packet::getPayloadLength() const {
    return payloadLength;
}

const uint8_t* Packet::getPayload() const {
    return payload;
}

const uint8_t* Packet::getFullPacket() const {
    fullPacket[0] = syncByte;
    fullPacket[1] = address;
    fullPacket[2] = opCode;
    fullPacket[3] = payloadLength;
    memcpy(fullPacket + 4, payload, payloadLength);
    return fullPacket;
}

std::vector<uint8_t> Packet::getFullPacketVector() const {
    std::vector<uint8_t> fullPacket;
    fullPacket.push_back(syncByte);
    fullPacket.push_back(address);
    fullPacket.push_back(opCode);
    fullPacket.push_back(payloadLength);
    fullPacket.insert(fullPacket.end(), payload, payload + payloadLength);
    return fullPacket;
}

size_t Packet::getFullPacketLength() const {
    return 4 + payloadLength;
}

// Setters
void Packet::setSyncByte(uint8_t syncByte) {
    this->syncByte = syncByte;
}

void Packet::setAddress(uint8_t address) {
    if (!isValidAddress(address)) {
        throw std::invalid_argument("Invalid address");
    }
    this->address = address;
}

void Packet::setOpCode(uint8_t opCode) {
    this->opCode = opCode;
}

void Packet::setPayloadLength(uint8_t payloadLength) {
    if (payloadLength > 200) {
        throw std::invalid_argument("Payload length is too long");
    }
    this->payloadLength = payloadLength;
}

void Packet::setPayload(const uint8_t* payload, uint8_t length) {
    if (length > 200) {
        throw std::invalid_argument("Payload length is too long");
    }
    this->payloadLength = length;
    memcpy(this->payload, payload, length);
}
