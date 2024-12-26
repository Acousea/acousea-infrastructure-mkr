#include "CompleteStatusReportRoutine.h"

CompleteStatusReportRoutine::CompleteStatusReportRoutine(NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(getClassNameString()), nodeConfigurationRepository(nodeConfigurationRepository) {}

Result<Packet> CompleteStatusReportRoutine::execute() {
    // Extract a Summary struct from the packet
    NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    return Result<Packet>::success(
            ErrorPacket::invalidOpcode(RoutingChunk::fromNodeToBackend(nodeConfig.getLocalAddress())));

}

CompleteStatusReportRoutine::CompleteStatusReportRoutine(const std::string &name, IGPS *gps,
                                                           IBatteryController *battery, RTCController *rtc,
                                                           NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(name), gps(gps), battery(battery), rtc(rtc),
          nodeConfigurationRepository(nodeConfigurationRepository) {}
