#ifndef ACOUSEA_MKR1310_NODES_ROUTINGCHUNK_H
#define ACOUSEA_MKR1310_NODES_ROUTINGCHUNK_H


#include <vector>
#include <cstdint>
#include <cstring>
#include "Routing/Address/Address.h"

class RoutingChunk {
public:
    RoutingChunk(Address sender, Address receiver, uint8_t ttl = 3);

    static RoutingChunk fromBytes(const std::vector<uint8_t> &data);

    static RoutingChunk fromBackendToNode(Address receiver);

    static RoutingChunk fromNodeToBackend(Address sender);

    [[nodiscard]] std::vector<uint8_t> toBytes() const;

    [[nodiscard]] static size_t getSize();

    [[nodiscard]] Address getSender() const;

    void setSender(const Address &senderAddress);

    [[nodiscard]] Address getReceiver() const;

    [[nodiscard]] uint8_t getTTL() const;

    void swapSenderReceiverAddresses();

private:
    Address sender;
    Address receiver;
    uint8_t ttl;
};


#endif //ACOUSEA_MKR1310_NODES_ROUTINGCHUNK_H
