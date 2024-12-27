#ifndef ACOUSEA_MKR1310_NODES_BASICREPORTPACKET_H
#define ACOUSEA_MKR1310_NODES_BASICREPORTPACKET_H

#include <Packet.h>

class BasicStatusReportPacket : public Packet {
public:
    BasicStatusReportPacket(const RoutingChunk &routingChunk,
                      const BatteryModule &battery,
                      const LocationModule &location,
                      const RTCModule &rtc);

};

#endif // ACOUSEA_MKR1310_NODES_BASICREPORTPACKET_H
