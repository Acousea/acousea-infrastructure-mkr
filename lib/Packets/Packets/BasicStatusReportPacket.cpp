#include "BasicStatusReportPacket.h"

BasicStatusReportPacket::BasicStatusReportPacket(const RoutingChunk &routingChunk, const BatteryModule &battery,
                                     const LocationModule &location, const RTCModule &rtc)
        : Packet(OperationCode(OperationCode::Code::BASIC_STATUS_REPORT), routingChunk,
                 BasicStatusReportPayload(battery, location, rtc)) {}
