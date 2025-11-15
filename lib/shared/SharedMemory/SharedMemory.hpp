#ifndef ACOUSEA_SHARED_MEMORY_HPP
#define ACOUSEA_SHARED_MEMORY_HPP

#include "bindings/nodeDevice.pb.h"


class SharedMemory
{
public:
    // ---- NodeConfiguration ----
    static void setNodeConfiguration(const acousea_NodeConfiguration& cfg);

    static acousea_NodeConfiguration& nodeConfigurationRef() noexcept;

    static bool isNodeConfigurationValid() noexcept;

    static void resetNodeConfiguration() noexcept;

    // ---- CommunicationPacket ----
    static void setCommunicationPacket(const acousea_CommunicationPacket& pkt);

    static acousea_CommunicationPacket& communicationPacketRef() noexcept;

    static const acousea_CommunicationPacket& getCommunicationPacketConst() noexcept;

    static void resetCommunicationPacket() noexcept;

    static uint8_t* tmpBuffer() noexcept;

    static constexpr size_t tmpBufferSize() noexcept { return TMP_BUFFER_SIZE; }

    static void clearTmpBuffer() noexcept;

private:
    // ---- Datos compartidos ----
    static inline acousea_NodeConfiguration nodeConfiguration_ = acousea_NodeConfiguration_init_default;
    static inline bool nodeConfigurationValid_ = false;
    static inline acousea_CommunicationPacket communicationPacket_ = acousea_CommunicationPacket_init_default;

    static constexpr size_t TMP_BUFFER_SIZE = 2048;
    static inline uint8_t tmpBuffer_[TMP_BUFFER_SIZE] = {};
};

#endif
