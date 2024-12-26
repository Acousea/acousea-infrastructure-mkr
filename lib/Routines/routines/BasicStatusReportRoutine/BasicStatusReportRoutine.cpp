#include "BasicStatusReportRoutine.h"

BasicStatusReportRoutine::BasicStatusReportRoutine(IGPS *gps, IBatteryController *battery, RTCController *rtc,
                                                     NodeConfigurationRepository &nodeConfigurationRepository)
    : IRoutine(getClassNameString()), gps(gps), battery(battery), rtc(rtc),
      nodeConfigurationRepository(nodeConfigurationRepository) {
}

Result<Packet> BasicStatusReportRoutine::execute() {
    // Extract a SummaryReport struct from the packet
    Logger::logInfo("BasicSummaryReportRoutine: Executing...");
    const auto batteryPercentage = battery->percentage();
    const auto batteryStatus = battery->status();
    const auto [latitude, longitude] = gps->read();
    const auto epoch = rtc->getEpoch();
    const auto nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    Logger::logInfo("Building packet with ["
                    " battery percentage: " + std::to_string(batteryPercentage) +
                    ", battery status: " + std::to_string(batteryStatus) +
                    ", location: (" + std::to_string(latitude) + ", " + std::to_string(longitude) +
                    "), epoch: " + std::to_string(epoch) + "]");

    const auto statusPacket = BasicStatusReportPacket(
        RoutingChunk::fromNodeToBackend(nodeConfig.getLocalAddress()),
        BatteryModule::from(batteryPercentage, batteryStatus),
        LocationModule::from(latitude, longitude),
        RTCModule::from(epoch)
    );
    return Result<Packet>::success(statusPacket);
}
