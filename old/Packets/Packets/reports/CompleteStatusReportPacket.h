#ifndef ACOUSEA_MKR1310_NODES_COMPLETEREPORTPACKET_H
#define ACOUSEA_MKR1310_NODES_COMPLETEREPORTPACKET_H

#include "Packet.h"

class CompleteStatusReportPacket : public Packet {
public:
    CompleteStatusReportPacket(const RoutingChunk &routingChunk,
                         const BatteryModule &battery,
                         const AmbientModule &ambient,
                         const LocationModule &location,
                         const StorageModule &storage,
                         const PamModule &pamModule);
};

#endif //ACOUSEA_MKR1310_NODES_COMPLETEREPORTPACKET_H


