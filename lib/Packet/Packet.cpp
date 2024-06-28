#include "Packet.h"

// Constructor that receives an opCode, addresses, and payload
Packet::Packet(uint8_t opCode, uint8_t addresses, const uint8_t* payload, uint8_t length) {
    if (length > MAX_PAYLOAD_LENGTH) {
        SerialUSB.println("Packet::Constructor(): Payload length is too long");
        return;
    }
    this->syncByte = SYNC_BYTE;
    this->opCode = opCode;
    if (!isValidAddress(addresses)) {
        SerialUSB.println("Packet::Constructor(): Invalid addresses");
        return;        
    }
    this->addresses = addresses;
    this->payloadLength = length;
    memcpy(this->payload, payload, length);
    validPacket = true;
}

// Constructor that receives opCode, addresses and std::vector<uint8_t> payload
Packet::Packet(uint8_t opCode, uint8_t addresses, const std::vector<uint8_t>& payload) {
    if (payload.size() > MAX_PAYLOAD_LENGTH) {
        SerialUSB.println("Packet::Constructor(): Payload length is too long");
        return;
    }
    this->syncByte = SYNC_BYTE;
    this->opCode = opCode;
    if (!isValidAddress(addresses)) {
        SerialUSB.println("Packet::Constructor(): Invalid addresses");
        return;        
    }
    this->addresses = addresses;
    this->payloadLength = payload.size();
    memcpy(this->payload, payload.data(), payload.size());
    validPacket = true;
}


// Constructor
Packet::Packet(const uint8_t* data, size_t length) {
    if (length < PACKET_HEADER_LENGTH) {
        SerialUSB.println("Packet::Constructor(): Data length is less than packet header length");
        return;
    }
    syncByte = data[0];    
    opCode = data[1];
    addresses = data[2];
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
    uint8_t sender = (address & Packet::Address::SENDER_MASK) >> 6;
    uint8_t receiver = (address & Packet::Address::RECEIVER_MASK) >> 4;
    uint8_t packetType = (address & Packet::Address::PACKET_TYPE_MASK);
    return (sender == Packet::Address::BACKEND || sender == Packet::Address::LOCALIZER || sender == Packet::Address::DRIFTER || sender == Packet::Address::PI3) &&
            (receiver == Packet::Address::BACKEND || receiver == Packet::Address::LOCALIZER || receiver == Packet::Address::DRIFTER || receiver == Packet::Address::PI3) &&
            (packetType == Packet::PacketType::LORA_PACKET || packetType == Packet::PacketType::IRIDIUM_PACKET);
}

void Packet::swapSenderReceiverAddresses() const {
    addresses = SENDER(GET_RECEIVER(addresses)) | RECEIVER(GET_SENDER(addresses)) | (addresses & 0x0F);
}

// Getters
uint8_t Packet::getSyncByte() const {
    return syncByte;
}

uint8_t Packet::getAddresses() const {
    return addresses;
}

uint8_t Packet::getSwappedAddresses() const {
    return SENDER(GET_RECEIVER(addresses)) | RECEIVER(GET_SENDER(addresses)) | (addresses & 0x0F);
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
    fullPacket[1] = opCode;
    fullPacket[2] = addresses;
    fullPacket[3] = payloadLength;
    memcpy(fullPacket + 4, payload, payloadLength);
    return fullPacket;
}

std::vector<uint8_t> Packet::getFullPacketVector() const {
    std::vector<uint8_t> fullPacketVec;
    fullPacketVec.push_back(syncByte);
    fullPacketVec.push_back(opCode);
    fullPacketVec.push_back(addresses);
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

void Packet::setAddresses(uint8_t addresses){
    if (!isValidAddress(addresses)) {
        SerialUSB.println("Packet::setAddresses(): Invalid addresses");
        return;        
    }
    this->addresses = addresses;

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


void Packet::print() const {
    SerialUSB.print(syncByte, HEX);
    SerialUSB.print(" ");
    SerialUSB.print(opCode, HEX);
    SerialUSB.print(" ");
    SerialUSB.print(addresses, HEX);
    SerialUSB.print(" ");
    SerialUSB.print(payloadLength, HEX);
    SerialUSB.print(" ");
    for (uint8_t i = 0; i < payloadLength; i++) {
        SerialUSB.print(payload[i], HEX);
        SerialUSB.print(" ");
    }
    SerialUSB.println();
}