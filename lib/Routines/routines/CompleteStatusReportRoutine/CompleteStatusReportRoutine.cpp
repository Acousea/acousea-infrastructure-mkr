#include "CompleteStatusReportRoutine.h"


CompleteStatusReportRoutine::CompleteStatusReportRoutine(IGPS *gps,
                                                         IBatteryController *battery,
                                                         NodeConfigurationRepository &nodeConfigurationRepository,
                                                         ICListenService &icListenService
)
    : IRoutine(getClassNameString()),
      gps(gps),
      battery(battery),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(icListenService) {
}

Result<Packet> CompleteStatusReportRoutine::execute() {
    // Extract a Summary struct from the packet
    const NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    const auto batteryPercentage = battery->percentage();
    const auto batteryStatus = battery->status();
    const auto [latitude, longitude] = gps->read();

    const auto icListenConfig = icListenService.getCache()->getHFCompleteConfiguration();

    if (!icListenConfig.isSuccess()) {
        icListenService.getRequester()->fetchHFConfiguration();
        return Result<Packet>::failure("ICListenHF configuration not available yet. Requesting from ICListenService.");
    }

    Logger::logInfo("Building packet with ["
                    " battery percentage: " + std::to_string(batteryPercentage) +
                    ", battery status: " + std::to_string(batteryStatus) +
                    ", ambient: (0, 0)" +
                    ", location: (" + std::to_string(latitude) + ", " + std::to_string(longitude) +
                    "), storage: (0, 0)" +
                    ", icListenConfig: " + icListenConfig.getValue().toString() + "]"
    );


    return Result<Packet>::success(
        CompleteStatusReportPacket(
            RoutingChunk::fromNodeToBackend(Address(nodeConfig.getLocalAddress())),
            BatteryModule::from(batteryStatus, batteryPercentage),
            AmbientModule::from(0, 0),
            LocationModule::from(latitude, longitude),
            StorageModule::from(0, 0),
            icListenConfig.getValue()
        )
    );
}
