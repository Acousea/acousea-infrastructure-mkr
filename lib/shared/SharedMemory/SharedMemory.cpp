#include "SharedMemory.hpp"
#include <cstring>


void SharedMemory::setNodeConfiguration(const acousea_NodeConfiguration& cfg)
{
    nodeConfiguration_ = cfg;
    nodeConfigurationValid_ = true;
}

acousea_NodeConfiguration& SharedMemory::nodeConfigurationRef() noexcept
{
    return nodeConfiguration_;
}

bool SharedMemory::isNodeConfigurationValid() noexcept
{
    return nodeConfigurationValid_;
}

void SharedMemory::resetNodeConfiguration() noexcept
{
    memset(&nodeConfiguration_, 0, sizeof(nodeConfiguration_));
    nodeConfiguration_ = acousea_NodeConfiguration_init_default;
    nodeConfigurationValid_ = false;
}

void SharedMemory::setCommunicationPacket(const acousea_CommunicationPacket& pkt)
{
    communicationPacket_ = pkt;
}

acousea_CommunicationPacket& SharedMemory::communicationPacketRef() noexcept
{
    return communicationPacket_;
}

const acousea_CommunicationPacket& SharedMemory::getCommunicationPacketConst() noexcept
{
    return communicationPacket_;
}


void SharedMemory::resetCommunicationPacket() noexcept
{
    memset(&communicationPacket_, 0, sizeof(communicationPacket_));
    communicationPacket_ = acousea_CommunicationPacket_init_default;
}

uint8_t* SharedMemory::tmpBuffer() noexcept
{
    return tmpBuffer_;
}


void SharedMemory::clearTmpBuffer() noexcept
{
    memset(tmpBuffer_, 0, sizeof(tmpBuffer_));
}
