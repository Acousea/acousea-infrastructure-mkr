#include "CompleteSummaryReportRoutine.h"

CompleteSummaryReportRoutine::CompleteSummaryReportRoutine(NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(getClassNameString()), nodeConfigurationRepository(nodeConfigurationRepository) {}

Result<Packet> CompleteSummaryReportRoutine::execute() {
    // Extract a Summary struct from the packet
    NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    return Result<Packet>::success(
            ErrorPacket::invalidOpcode(RoutingChunk::fromNodeToBackend(nodeConfig.getLocalAddress())));

}

CompleteSummaryReportRoutine::CompleteSummaryReportRoutine(const std::string &name, IGPS *gps,
                                                           IBatteryController *battery, RTCController *rtc,
                                                           NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(name), gps(gps), battery(battery), rtc(rtc),
          nodeConfigurationRepository(nodeConfigurationRepository) {}
