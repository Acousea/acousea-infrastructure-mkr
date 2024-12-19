#include "CompleteSummaryReportRoutine.h"

CompleteSummaryReportRoutine::CompleteSummaryReportRoutine(NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(getClassNameString()), nodeConfigurationRepository(nodeConfigurationRepository) {}

Result<Packet> CompleteSummaryReportRoutine::execute() {
    // Extract a Summary struct from the packet
    NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    return Result<Packet>::success(
            ErrorPacket::invalidOpcode(RoutingChunk::fromNodeToBackend(nodeConfig.getLocalAddress())));

}
