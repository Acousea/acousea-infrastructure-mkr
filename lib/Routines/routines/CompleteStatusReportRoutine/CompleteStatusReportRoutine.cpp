#include "CompleteStatusReportRoutine.h"

#include <utility>


CompleteStatusReportRoutine::CompleteStatusReportRoutine(NodeConfigurationRepository& nodeConfigurationRepository,
                                                         std::optional<std::shared_ptr<ICListenService>> icListenService,
                                                         IGPS* gps,
                                                         IBatteryController* battery
)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(std::move(icListenService)),
      gps(gps),
      battery(battery)
{
}

Result<acousea_CommunicationPacket> CompleteStatusReportRoutine::execute()
{
    // Extract a Summary struct from the packet
    const auto batteryPercentage = battery->percentage();
    const auto batteryStatus = battery->status();
    const auto [latitude, longitude] = gps->read();
    // FIXME: SHOULD ADD NODE CONFIGURATION TO THE PACKET HERE
    const acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    // --- ICListen opcional ---
    std::optional<acousea_ICListenHF> icListenHFCompleteConfig;
    if (icListenService.has_value())
    {
        const auto icListenConfigResult = (*icListenService)->getCache()->retrieveICListenCompleteConfiguration();
        if (!icListenConfigResult.isSuccess())
        {
            (*icListenService)->getRequester()->fetchHFConfiguration();
            return Result<acousea_CommunicationPacket>::failure(
                "ICListenHF configuration not available yet. Requested from ICListenService."
            );
        }
        icListenHFCompleteConfig = icListenConfigResult.getValue();
    }

    Logger::logInfo("Building packet with "
        "[ battery: (percentage=" + std::to_string(batteryPercentage) + ", status=" + std::to_string(batteryStatus) +
        ", location: (LAT=" + std::to_string(latitude) + ", LONG=" + std::to_string(longitude) + ")" +
        (icListenHFCompleteConfig
             ? ", icListenConfig [SerialNumber]: " + std::string(icListenHFCompleteConfig->serialNumber)
             : ", icListenConfig: not present") +
        " ]");

    // -------- Módulos --------
    acousea_StatusReportPayload status = acousea_StatusReportPayload_init_default;
    status.modules_count = 0;

    {
        acousea_StatusReportPayload_ModulesEntry eBattery = acousea_StatusReportPayload_ModulesEntry_init_default;
        eBattery.key = acousea_ModuleCode_BATTERY_MODULE;
        eBattery.has_value = true;
        eBattery.value.which_module = acousea_ModuleWrapper_battery_tag;
        eBattery.value.module.battery.batteryPercentage = batteryPercentage;
        eBattery.value.module.battery.batteryStatus = batteryStatus;
        status.modules[status.modules_count++] = eBattery;
    }

    {
        acousea_StatusReportPayload_ModulesEntry eLocation = acousea_StatusReportPayload_ModulesEntry_init_default;
        eLocation.key = acousea_ModuleCode_LOCATION_MODULE;
        eLocation.has_value = true;
        eLocation.value.which_module = acousea_ModuleWrapper_location_tag;
        eLocation.value.module.location.latitude = static_cast<float>(latitude);
        eLocation.value.module.location.longitude = static_cast<float>(longitude);
        status.modules[status.modules_count++] = eLocation;
    }

    {
        if (icListenHFCompleteConfig)
        {
            acousea_StatusReportPayload_ModulesEntry eICListen = acousea_StatusReportPayload_ModulesEntry_init_default;
            eICListen.key = acousea_ModuleCode_ICLISTEN_HF;
            eICListen.has_value = true;
            eICListen.value.which_module = acousea_ModuleWrapper_icListenHF_tag;
            eICListen.value.module.icListenHF = *icListenHFCompleteConfig;
            status.modules[status.modules_count++] = eICListen;
        }
    }

    // --- PayloadWrapper ---
    acousea_PayloadWrapper pw = acousea_PayloadWrapper_init_default;
    pw.which_payload = acousea_PayloadWrapper_statusPayload_tag;
    pw.payload.statusPayload = status;

    // --- CommunicationPacket ---
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = 1;
    pkt.routing.receiver = 0;
    pkt.routing.ttl = 5;
    pkt.has_payload = true;
    pkt.payload = pw;

    return Result<acousea_CommunicationPacket>::success(pkt);
}
