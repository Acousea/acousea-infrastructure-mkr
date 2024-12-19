#include "BasicSummaryReportRoutine.h"

BasicSummaryReportRoutine::BasicSummaryReportRoutine(IGPS *gps, IBatteryController *battery, RTCController *rtc,
                                                     NodeConfigurationRepository &nodeConfigurationRepository)
        : IRoutine(getClassNameString()), gps(gps), battery(battery), rtc(rtc),
          nodeConfigurationRepository(nodeConfigurationRepository) {}

Result<Packet> BasicSummaryReportRoutine::execute() {
    // Extract a SummaryReport struct from the packet
    auto batteryPercentage = battery->percentage();
    auto batteryStatus = battery->status();
    auto location = gps->read();
    auto epoch = rtc->getEpoch();
    auto nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    auto statusPacket = BasicReportPacket(
            RoutingChunk::fromNodeToBackend(nodeConfig.getLocalAddress()),
            BatteryModule::from(batteryPercentage, batteryStatus),
            LocationModule::from(location.latitude, location.longitude),
            RTCModule::from(epoch)
    );
    return Result<Packet>::success(statusPacket);
}
