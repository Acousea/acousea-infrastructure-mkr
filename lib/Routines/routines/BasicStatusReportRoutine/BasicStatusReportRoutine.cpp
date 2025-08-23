#include "BasicStatusReportRoutine.h"

#include "pb_encode.h"

BasicStatusReportRoutine::BasicStatusReportRoutine(
        IGPS *gps,
        IBatteryController *battery,
        RTCController *rtc,
        NodeConfigurationRepository &nodeConfigurationRepository
)
        : IRoutine(getClassNameString()), gps(gps), battery(battery), rtc(rtc),
          nodeConfigurationRepository(nodeConfigurationRepository) {
}

Result<_acousea_CommunicationPacket> BasicStatusReportRoutine::execute() {
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

    // -------- 1) Módulos (Battery / Location / RTC) --------
    acousea_ModuleWrapper modBattery = acousea_ModuleWrapper_init_default;
    modBattery.which_module = acousea_ModuleWrapper_battery_tag;
    modBattery.module.battery.batteryPercentage = batteryPercentage;
    modBattery.module.battery.batteryStatus     = batteryStatus;

    acousea_ModuleWrapper modLocation = acousea_ModuleWrapper_init_default;
    modLocation.which_module = acousea_ModuleWrapper_location_tag;
    modLocation.module.location.latitude  = static_cast<float>(latitude);
    modLocation.module.location.longitude = static_cast<float>(longitude);

    acousea_ModuleWrapper modRtc = acousea_ModuleWrapper_init_default;
    modRtc.which_module = acousea_ModuleWrapper_rtc_tag;   // o el tag real que tengas
    modRtc.module.rtc.epochSeconds = epoch;

    // --- Map entries (key = ModuleCode, value = ModuleWrapper) ---
    acousea_StatusReportPayload_ModulesEntry eBattery = acousea_StatusReportPayload_ModulesEntry_init_default;
    eBattery.key = acousea_ModuleCode_BATTERY_MODULE;
    eBattery.has_value = true;
    eBattery.value = modBattery;

    acousea_StatusReportPayload_ModulesEntry eLocation = acousea_StatusReportPayload_ModulesEntry_init_default;
    eLocation.key = acousea_ModuleCode_LOCATION_MODULE;
    eLocation.has_value = true;
    eLocation.value = modLocation;

    acousea_StatusReportPayload_ModulesEntry eRtc = acousea_StatusReportPayload_ModulesEntry_init_default;
    eRtc.key = acousea_ModuleCode_RTC_MODULE;
    eRtc.has_value = true;
    eRtc.value = modRtc;

    // -------- 2) BasicStatusReportPayload --------
    // --- Contexto persistente para el callback ---
    acousea_StatusReportPayload status = acousea_StatusReportPayload_init_default;
    status.modules_count = 3; // número de entradas
    status.modules[0] = eBattery;
    status.modules[1] = eLocation;
    status.modules[2] = eRtc;

    // --- PayloadWrapper (oneof) ---
    acousea_PayloadWrapper pw = acousea_PayloadWrapper_init_default;
    pw.which_payload = acousea_PayloadWrapper_statusPayload_tag;
    pw.payload.statusPayload = status;

    // --- CommunicationPacket ---
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    // Routing
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender   = 1; // addr;  // nodo origen, ajusta
    pkt.routing.receiver = 0;   // backend destino, ajusta
    pkt.routing.ttl      = 5;
    // Payload
    pkt.has_payload = true;
    pkt.payload = pw;


    return Result<acousea_CommunicationPacket>::success(pkt);
}
