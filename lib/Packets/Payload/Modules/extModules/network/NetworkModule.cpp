#include "NetworkModule.h"

NetworkModule
NetworkModule::from(uint8_t localAddress, const std::map<uint8_t, uint8_t> &routingTable, uint8_t defaultGateway) {
    return NetworkModule(localAddress, routingTable, defaultGateway);
}

NetworkModule NetworkModule::from(const std::vector<uint8_t> &value) {
    if (value.size() < 2) {
        ErrorHandler::handleError("Invalid value size for NetworkModule");
    }
    return NetworkModule(value);
}

uint8_t NetworkModule::getLocalAddress() const {
    return localAddress;
}

uint8_t NetworkModule::getDefaultGateway() const {
    return defaultGateway;
}

const std::map<uint8_t, uint8_t> &NetworkModule::getRoutingTable() const {
    return routingTable;
}

NetworkModule::NetworkModule(const std::vector<uint8_t> &value)
        : SerializableModule(ModuleCode::TYPES::NETWORK, value) {
    if (VALUE.size() < 2) {
        ErrorHandler::handleError("Invalid value size for NetworkModule");
    }
    localAddress = VALUE[0];
    size_t i = 1;
    while (i + 2 <= VALUE.size()) {
        uint8_t destination = VALUE[i];
        uint8_t nextHop = VALUE[i + 1];
        routingTable[destination] = nextHop;
        i += 2;
    }
    defaultGateway = VALUE[i];
}

NetworkModule::NetworkModule(uint8_t localAddress, const std::map<uint8_t, uint8_t> &routingTable,
                             uint8_t defaultGateway)
        : SerializableModule(ModuleCode::TYPES::NETWORK, serializeValues(localAddress, routingTable, defaultGateway)),
          localAddress(localAddress), routingTable(routingTable), defaultGateway(defaultGateway) {}

std::vector<uint8_t>
NetworkModule::serializeValues(uint8_t localAddress, const std::map<uint8_t, uint8_t> &routingTable,
                               uint8_t defaultGateway) {
    std::vector<uint8_t> value;
    value.push_back(localAddress);
    for (const auto& route : routingTable) {
        value.push_back(route.first);
        value.push_back(route.second);
    }
    value.push_back(defaultGateway);
    return value;
}
