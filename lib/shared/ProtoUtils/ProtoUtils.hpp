#ifndef ACOUSEA_INFRASTRUCTURE_MKR_PROTOUTILS_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_PROTOUTILS_HPP

#include <cstdint>

#include "bindings/nodeDevice.pb.h"
#include "Result.h"
#include <vector>

namespace ProtoUtils
{
    constexpr std::size_t ACOUSEA_MAX_MODULE_COUNT  = static_cast<std::size_t>(acousea_ModuleCode_ICLISTEN_HF);

    namespace CommunicationPacket
    {
        Result<std::vector<std::uint8_t>> encode(const acousea_CommunicationPacket& pkt);
        Result<acousea_CommunicationPacket> decode(const std::vector<uint8_t>& raw);

        Result<size_t> encodeInto(const acousea_CommunicationPacket& pkt, uint8_t* buffer, size_t bufferSize);
        Result<void> decodeInto(const uint8_t* data, size_t length, acousea_CommunicationPacket* out);

    }

    namespace NodeConfiguration
    {
        Result<std::vector<uint8_t>> encode(const acousea_NodeConfiguration& m);
        Result<acousea_NodeConfiguration> decode(const uint8_t* data, size_t length);

        Result<size_t> encodeInto(const acousea_NodeConfiguration& conf, uint8_t* buffer, size_t bufferSize);
        Result<void> decodeInto(const uint8_t* data, size_t length, acousea_NodeConfiguration* out);
    }
} // ProtoUtils

#endif //ACOUSEA_INFRASTRUCTURE_MKR_PROTOUTILS_HPP
