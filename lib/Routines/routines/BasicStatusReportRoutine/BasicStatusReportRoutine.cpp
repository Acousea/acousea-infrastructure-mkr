#include "BasicStatusReportRoutine.h"

#include "pb_encode.h"

BasicStatusReportRoutine::BasicStatusReportRoutine(
    NodeConfigurationRepository& nodeConfigurationRepository,
    IGPS* gps,
    IBatteryController* battery,
    RTCController* rtc

)
    : IRoutine(getClassNameString()),
      nodeConfigurationRepository(nodeConfigurationRepository),
      gps(gps),
      battery(battery),
      rtc(rtc)
{
}

Result<_acousea_CommunicationPacket> BasicStatusReportRoutine::execute(const std::optional<_acousea_CommunicationPacket>& none)
{
    Logger::logInfo("BasicSummaryReportRoutine: Executing...");

    const auto batteryPercentage = battery->percentage();
    const auto batteryStatus = battery->status();
    const auto [latitude, longitude] = gps->read();
    const auto epoch = rtc->getEpoch();
    // FIXME: SHOULD ADD NODE CONFIGURATION TO THE PACKET HERE
    const auto nodeConfig = nodeConfigurationRepository.getNodeConfiguration();

    Logger::logInfo("Building packet with ["
        " battery percentage: " + std::to_string(batteryPercentage) +
        ", battery status: " + std::to_string(batteryStatus) +
        ", location: (" + std::to_string(latitude) + ", " + std::to_string(longitude) +
        "), epoch: " + std::to_string(epoch) + "]");

    // -------- 1) Preparar módulos (Battery / Location / RTC) --------
    acousea_StatusReportPayload status = acousea_StatusReportPayload_init_default;
    status.modules_count = 0;

    // Battery
    {
        acousea_ModuleWrapper mod = acousea_ModuleWrapper_init_default;
        mod.which_module = acousea_ModuleWrapper_battery_tag;
        mod.module.battery.batteryPercentage = batteryPercentage;
        mod.module.battery.batteryStatus = batteryStatus;

        acousea_StatusReportPayload_ModulesEntry entry = acousea_StatusReportPayload_ModulesEntry_init_default;
        entry.key = acousea_ModuleCode_BATTERY_MODULE;
        entry.has_value = true;
        entry.value = mod;

        status.modules[status.modules_count++] = entry;
    }

    // Location
    {
        acousea_ModuleWrapper mod = acousea_ModuleWrapper_init_default;
        mod.which_module = acousea_ModuleWrapper_location_tag;
        mod.module.location.latitude = static_cast<float>(latitude);
        mod.module.location.longitude = static_cast<float>(longitude);

        acousea_StatusReportPayload_ModulesEntry entry = acousea_StatusReportPayload_ModulesEntry_init_default;
        entry.key = acousea_ModuleCode_LOCATION_MODULE;
        entry.has_value = true;
        entry.value = mod;

        status.modules[status.modules_count++] = entry;
    }

    // RTC
    {
        acousea_ModuleWrapper mod = acousea_ModuleWrapper_init_default;
        mod.which_module = acousea_ModuleWrapper_rtc_tag;
        mod.module.rtc.epochSeconds = epoch;

        acousea_StatusReportPayload_ModulesEntry entry = acousea_StatusReportPayload_ModulesEntry_init_default;
        entry.key = acousea_ModuleCode_RTC_MODULE;
        entry.has_value = true;
        entry.value = mod;

        status.modules[status.modules_count++] = entry;
    }

    // -------- 2) PayloadWrapper --------
    acousea_PayloadWrapper pw = acousea_PayloadWrapper_init_default;
    pw.which_payload = acousea_PayloadWrapper_statusPayload_tag;
    pw.payload.statusPayload = status;

    // -------- 3) CommunicationPacket --------
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = 1; // origen (ajustar)
    pkt.routing.receiver = 0; // destino (ajustar)
    pkt.routing.ttl = 5;

    pkt.has_payload = true;
    pkt.payload = pw;

    return Result<acousea_CommunicationPacket>::success(pkt);
}
