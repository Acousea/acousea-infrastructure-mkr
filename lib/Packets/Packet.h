#ifndef COMMUNICATIONPACKET_H
#define COMMUNICATIONPACKET_H

#include <utility>
#include <vector>
#include <sstream>
#include <iomanip>
#include <variant>


#include "Routing/OperationCode/OperationCode.h"
#include "Routing/RoutingChunk/RoutingChunk.h"
#include "Routing/Address/Address.h"
#include "Payload/Payloads/getConfig/GetUpdatedNodeConfigurationPayload.h"
#include "Payload/Payloads/setConfig/NewNodeConfigurationPayload.h"
#include "Payload/Payloads/complete/CompleteStatusReportPayload.h"
#include "Payload/Payloads/basic/BasicStatusReportPayload.h"
#include "Payload/Payloads/error/ErrorPayload.h"
#include "CRC/CRCUtils.h"
#include "Payload/Payloads/iclisten/ICListenCompleteStatusPayload.h"


// Define a variant type to hold different Payload types
using PayloadVariant = std::variant<
        GetUpdatedNodeConfigurationPayload,
        NewNodeConfigurationPayload,
        CompleteStatusReportPayload,
        BasicStatusReportPayload,
        ICListenCompleteStatusPayload,
        ErrorPayload
>;

class Packet {
public:
    static constexpr uint8_t SYNC_BYTE = 0x20;

    // Constructor with PayloadVariant
    Packet(const OperationCode &opCode, const RoutingChunk &routingChunk, PayloadVariant payload);

    Packet(const OperationCode &opCode, const RoutingChunk &routingChunk, PayloadVariant payload, uint16_t checksum);

    // Serializes the packet into a vector of bytes
    [[nodiscard]] std::vector<uint8_t> toBytes() const;

    // Constructs a packet from a vector of bytes
    static Packet fromBytes(const std::vector<uint8_t> &data);

    // Encodes the packet to a hex string
    [[nodiscard]] std::string encode() const;


    [[nodiscard]] const OperationCode &getOpCode() const;

    [[nodiscard]] OperationCode::Code getOpCodeEnum() const;

    [[nodiscard]] const RoutingChunk &getRoutingChunk() const;

    void setRoutingChunk(const RoutingChunk &chunk);

    [[nodiscard]] const PayloadVariant &getPayload() const;

    // Get the packetPayload as a specific type
    template<typename T>
    const T &getPayloadAs() const {
        return std::get<T>(payload);
    }

    [[nodiscard]] uint16_t getChecksum() const;

    void swapSenderReceiverAddresses();

private:
    OperationCode opCode;
    RoutingChunk routingChunk;
    PayloadVariant payload; // Holds the packetPayload as a variant
    uint16_t checksum;

    // Computes the checksum using CRCUtils
    void computeChecksum();
};

#endif // COMMUNICATIONPACKET_H
