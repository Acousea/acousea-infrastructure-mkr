#include "Router.h"


Router::Router(const std::vector<IPort *> &relayedPorts)
        : relayedPorts(relayedPorts) {
    for (const auto &port: relayedPorts) {
        receivedPackets[port->getType()] = std::deque<Packet>();
    }
}

void Router::addRelayedPort(IPort *port) {
    relayedPorts.push_back(port);
}

Router::RouterSender Router::sendFrom(Address senderAddress) {
    return RouterSender(senderAddress, this);
}

std::map<IPort::PortType, std::deque<Packet>> Router::readPorts(const Address &localAddress) {
    for (auto &port: relayedPorts) {
        if (!port->available()) {
            continue;
        }
        SerialUSB.println("Router::readPorts() -> Port available");
        std::vector<std::vector<uint8_t>> rawPacketBytes = port->read();
        for (const auto &rawData: rawPacketBytes) {
            Packet packet = Packet::fromBytes(rawData);
            receivedPackets[port->getType()].push_back(packet);
        }
    }
    return receivedPackets;
}

void Router::sendSBD(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::SBDPort) {
            relayedPort->send(packet.toBytes());
            return;
        }
    }
}

void Router::sendLoRa(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::LoraPort) {
            relayedPort->send(packet.toBytes());
            return;
        }
    }
}

void Router::sendSerial(const Packet &packet) {
    for (auto &relayedPort: relayedPorts) {
        if (relayedPort->getType() == IPort::PortType::SerialPort) {
            relayedPort->send(packet.toBytes());
            return;
        }
    }
}

