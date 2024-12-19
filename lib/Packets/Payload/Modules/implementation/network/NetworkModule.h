#ifndef ACOUSEA_MKR1310_NODES_NETWORKMODULE_H
#define ACOUSEA_MKR1310_NODES_NETWORKMODULE_H


#include "Payload/Modules/SerializableModule.h"


class NetworkModule : public SerializableModule {
public:
    
    static NetworkModule from(uint8_t localAddress, const std::map<uint8_t, uint8_t>& routingTable, uint8_t defaultGateway);

    
    static NetworkModule from(const std::vector<uint8_t>& value);

    
    [[nodiscard]] uint8_t getLocalAddress() const;

    [[nodiscard]] uint8_t getDefaultGateway() const;

    [[nodiscard]] const std::map<uint8_t, uint8_t>& getRoutingTable() const;

private:
    
    explicit NetworkModule(const std::vector<uint8_t>& value);

    NetworkModule(uint8_t localAddress, const std::map<uint8_t, uint8_t>& routingTable, uint8_t defaultGateway);

    static std::vector<uint8_t> serializeValues(uint8_t localAddress, const std::map<uint8_t, uint8_t>& routingTable, uint8_t defaultGateway);

private:
    uint8_t localAddress;                          
    std::map<uint8_t, uint8_t> routingTable;       
    uint8_t defaultGateway;                        

};

#endif 
