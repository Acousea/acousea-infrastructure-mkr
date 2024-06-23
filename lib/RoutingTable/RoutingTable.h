#ifndef ROUTING_TABLE_H
#define ROUTING_TABLE_H

#include <Arduino.h>
#include <map>
#include "ICommunicator.h"
#include "Packet.h"

class RoutingTable {
public:
    // Constructor that accepts a map of routes
    RoutingTable(std::map<uint8_t, ICommunicator*> routes) : routes(routes) {}


    void addRoute(uint8_t address, ICommunicator* communicator) {
        routes[address] = communicator;
    }

    ICommunicator* getRoute(uint8_t address) const {
        auto it = routes.find(Packet::Address::RECEIVER_AND_PACKET_TYPE_MASK & address);
        if (it != routes.end()) {
            return it->second;
        }
        return nullptr;
    }

private:
    std::map<uint8_t, ICommunicator*> routes;
};

#endif // ROUTING_TABLE_H
