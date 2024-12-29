#include "RoutingChunk.h"

RoutingChunk::RoutingChunk(Address sender, Address receiver, uint8_t ttl)
        : sender(sender), receiver(receiver), ttl(ttl) {}

RoutingChunk RoutingChunk::fromBytes(const std::vector<uint8_t> &data) {
    if (data.size() < getSize()) {
        ErrorHandler::handleError("Buffer does not have enough bytes to create a RoutingChunk");
    }
    Address sender = Address::fromValue(data[0]);
    Address receiver = Address::fromValue(data[1]);
    uint8_t ttl = data[2];
    return RoutingChunk(sender, receiver, ttl);
}

RoutingChunk RoutingChunk::fromBackendToNode(Address receiver) {
    return RoutingChunk(Address::backend(), receiver);
}

RoutingChunk RoutingChunk::fromNodeToBackend(Address sender) {
    return RoutingChunk(sender, Address::backend());
}

RoutingChunk RoutingChunk::broadcastFrom(const Address sender){
    return RoutingChunk(sender, Address::broadcastAddress());
}


std::vector<uint8_t> RoutingChunk::toBytes() const {
    std::vector<uint8_t> data(getSize());
    data[0] = sender.getValue();
    data[1] = receiver.getValue();
    data[2] = ttl;
    return data;
}


size_t RoutingChunk::getSize() {
    return Address::getSize() * 2 + sizeof(uint8_t);
}

Address RoutingChunk::getSender() const {
    return sender;
}

void RoutingChunk::setSender(const Address &senderAddress) {
    sender = senderAddress;
}

Address RoutingChunk::getReceiver() const {
    return receiver;
}

uint8_t RoutingChunk::getTTL() const {
    return ttl;
}

void RoutingChunk::swapSenderReceiverAddresses() {
    std::swap(sender, receiver);
}
