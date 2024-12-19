#include "Router.h"


Router::Router(PacketProcessor *processor, IDisplay *display, const std::vector<IPort *> &relayedPorts,
               const NodeConfigurationRepository &nodeConfigurationRepository) :
        processor(processor), display(display), relayedPorts(relayedPorts),
        nodeConfigurationRepository(nodeConfigurationRepository) {}

        void Router::addRelayedPort(IPort *port) {
    relayedPorts.push_back(port);
}

Router::RouterSender Router::sender() {
    NodeConfiguration nodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();
    return RouterSender(nodeConfiguration.getLocalAddress(), this);
}

void Router::readPorts() {
    for (auto &relayedPort: relayedPorts) {
        if (!relayedPort->available()) {
            continue;
        }
        display->print("Router::readPorts() -> Port available");
        Result<Packet> result = relayedPort->read();
        if (result.isError()) {
            display->print("Router::readPorts() -> Error reading requestPacket");
            continue;
        }
        Packet requestPacket = result.getValue();
        processPacket(relayedPort, requestPacket);
    }
}

void Router::processPacket(IPort *port, Packet &requestPacket) {
    NodeConfiguration nodeConfiguration = nodeConfigurationRepository.getNodeConfiguration();
    uint8_t receiverAddress = requestPacket.getRoutingChunk().getReceiver().getValue();
    if (!(receiverAddress == nodeConfiguration.getLocalAddress().getValue()) || processor == nullptr) {
        display->print("Router::readPorts() -> Receiver address is not for this device. Local address: " +
                       String(nodeConfiguration.getLocalAddress().getValue()) +
                       " Receiver address: " + String(receiverAddress));
        return;
    }
    display->print("Router::route-if() -> Processing requestPacket...");
    Packet responsePacket = processor->process(requestPacket);
    port->send(responsePacket);
}

void Router::sendSBD(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::SBDPort) {
            relayedPort->send(packet);
            return;
        }
    }
}

void Router::sendLoRa(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::LoraPort) {
            relayedPort->send(packet);
            return;
        }
    }
}

void Router::sendSerial(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::SerialPort) {
            relayedPort->send(packet);
            return;
        }
    }
}
