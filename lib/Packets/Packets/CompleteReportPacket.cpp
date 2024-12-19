#include "CompleteReportPacket.h"

CompleteReportPacket::CompleteReportPacket(const RoutingChunk &routingChunk, const BatteryModule &battery,
                                           const AmbientModule &ambient, const LocationModule &location,
                                           const StorageModule &storage, const PamModule &pamModule)
        : Packet(OperationCode(OperationCode::Code::COMPLETE_STATUS_REPORT), routingChunk,
                 CompleteStatusReportPayload(battery, ambient, location, storage, pamModule)) {}
