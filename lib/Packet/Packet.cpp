#include "Packet.h"

// Constructor
Packet::Packet(const uint8_t* data, size_t length) {
    if (length < PACKET_HEADER_LENGTH) {
        SerialUSB.println("Packet::Constructor(): Data length is less than packet header length");
        return;
    }
    syncByte = data[0];    
    senderAddress = (data[1] & Address::SENDER_MASK) >> 6;
    recipientAddress = (data[1] & Address::RECEIVER_MASK) >> 4;    
    opCode = data[2];
    payloadLength = data[3];

    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        SerialUSB.println("Packet::Constructor(): Payload length is too long");
        return;
    }

    if (length < (size_t) (PACKET_HEADER_LENGTH + payloadLength)) {
        SerialUSB.println("Packet::Constructor(): Data length is less than packet header length + payload length");
        return;        
    }

    memcpy(payload, data + PACKET_HEADER_LENGTH, payloadLength);
    validPacket = true;
}

bool Packet::isValid() const {
    return validPacket;
}

bool Packet::isValidAddress(uint8_t address) {
    return address == Address::BACKEND || address == Address::LOCALIZER || address == Address::DRIFTER || address == Address::PI3;
}

// Getters
uint8_t Packet::getSyncByte() const {
    return syncByte;
}

uint8_t Packet::getSenderAddress() const {
    return senderAddress;
}

uint8_t Packet::getRecipientAddress() const {
    return recipientAddress;
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
    fullPacket[1] = (senderAddress << 6) | (recipientAddress << 4);
    fullPacket[2] = opCode;
    fullPacket[3] = payloadLength;
    memcpy(fullPacket + 4, payload, payloadLength);
    return fullPacket;
}

std::vector<uint8_t> Packet::getFullPacketVector() const {
    std::vector<uint8_t> fullPacketVec;
    fullPacketVec.push_back(syncByte);
    fullPacketVec.push_back((senderAddress << 6) | (recipientAddress << 4));
    fullPacketVec.push_back(opCode);
    fullPacketVec.push_back(payloadLength);
    fullPacketVec.insert(fullPacketVec.end(), payload, payload + payloadLength);
    return fullPacketVec;
}

size_t Packet::getFullPacketLength() const {
    return PACKET_HEADER_LENGTH + payloadLength;
}

// Setters
void Packet::setSyncByte(uint8_t syncByte) {
    this->syncByte = syncByte;
}

void Packet::setSenderAddress(uint8_t senderAddress) {
    if (!isValidAddress(senderAddress)) {
        SerialUSB.println("Packet::setSenderAddress(): Invalid sender address");
        return;       
    }
    this->senderAddress = senderAddress;
}

void Packet::setRecipientAddress(uint8_t recipientAddress) {
    if (!isValidAddress(recipientAddress)) {
        SerialUSB.println("Packet::setRecipientAddress(): Invalid recipient address");
        return;        
    }
    this->recipientAddress = recipientAddress;
}

void Packet::setOpCode(uint8_t opCode) {
    this->opCode = opCode;
}

void Packet::setPayloadLength(uint8_t payloadLength) {
    if (payloadLength > MAX_PAYLOAD_LENGTH) {
        SerialUSB.println("Packet::setPayloadLength(): Payload length is too long");
        return;        
    }
    this->payloadLength = payloadLength;
}

void Packet::setPayload(const uint8_t* payload, uint8_t length) {
    if (length > MAX_PAYLOAD_LENGTH) {
        SerialUSB.println("Packet::setPayload(): Payload length is too long");
        return;        
    }
    this->payloadLength = length;
    memcpy(this->payload, payload, length);
}
