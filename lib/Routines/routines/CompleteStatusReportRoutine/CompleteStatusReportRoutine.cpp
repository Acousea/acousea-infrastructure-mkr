#include "CompleteStatusReportRoutine.h"


CompleteStatusReportRoutine::CompleteStatusReportRoutine(IGPS* gps,
                                                         IBatteryController* battery,
                                                         NodeConfigurationRepository& nodeConfigurationRepository,
                                                         ICListenService& icListenService
)
    : IRoutine(getClassNameString()),
      gps(gps),
      battery(battery),
      nodeConfigurationRepository(nodeConfigurationRepository),
      icListenService(icListenService){
}

Result<acousea_CommunicationPacket> CompleteStatusReportRoutine::execute(){
    // Extract a Summary struct from the packet
    const acousea_NodeConfiguration nodeConfig = nodeConfigurationRepository.getNodeConfiguration();
    const auto batteryPercentage = battery->percentage();
    const auto batteryStatus = battery->status();
    const auto [latitude, longitude] = gps->read();

    const auto icListenConfigResult = icListenService.getCache()->getHFCompleteConfiguration();

    if (!icListenConfigResult.isSuccess()){
        icListenService.getRequester()->fetchHFConfiguration();
        return Result<acousea_CommunicationPacket>::failure(
            "ICListenHF configuration not available yet. Requesting from ICListenService.");
    }

    const acousea_ICListenHF icListenHFCompleteConfig = icListenConfigResult.getValue();

    Logger::logInfo("Building packet with ["
        " battery percentage: " + std::to_string(batteryPercentage) +
        ", battery status: " + std::to_string(batteryStatus) +
        ", ambient: (0, 0)" +
        ", location: (" + std::to_string(latitude) + ", " + std::to_string(longitude) +
        "), storage: (0, 0)" +
        ", icListenConfig [SerialNumber]: " + icListenHFCompleteConfig.serialNumber + "]"
    );


    // -------- 1) Módulos (Battery / Location / RTC) --------
    acousea_ModuleWrapper modBattery = acousea_ModuleWrapper_init_default;
    modBattery.which_module = acousea_ModuleWrapper_battery_tag;
    modBattery.module.battery.batteryPercentage = batteryPercentage;
    modBattery.module.battery.batteryStatus = batteryStatus;

    acousea_ModuleWrapper modLocation = acousea_ModuleWrapper_init_default;
    modLocation.which_module = acousea_ModuleWrapper_location_tag;
    modLocation.module.location.latitude = static_cast<float>(latitude);
    modLocation.module.location.longitude = static_cast<float>(longitude);

    acousea_ModuleWrapper modICListenConfig = acousea_ModuleWrapper_init_default;
    modICListenConfig.which_module = acousea_ModuleWrapper_icListenHF_tag;
    modICListenConfig.module.icListenHF = icListenHFCompleteConfig;


    // --- Map entries (key = ModuleCode, value = ModuleWrapper) ---
    acousea_StatusReportPayload_ModulesEntry eBattery = acousea_StatusReportPayload_ModulesEntry_init_default;
    eBattery.key = acousea_ModuleCode_BATTERY_MODULE;
    eBattery.has_value = true;
    eBattery.value = modBattery;

    acousea_StatusReportPayload_ModulesEntry eLocation = acousea_StatusReportPayload_ModulesEntry_init_default;
    eLocation.key = acousea_ModuleCode_LOCATION_MODULE;
    eLocation.has_value = true;
    eLocation.value = modLocation;

    acousea_StatusReportPayload_ModulesEntry eICListen = acousea_StatusReportPayload_ModulesEntry_init_default;
    eICListen.key = acousea_ModuleCode_ICLISTEN_HF;
    eICListen.has_value = true;
    eICListen.value = modICListenConfig;


    // -------- 2) BasicStatusReportPayload --------
    // --- Contexto persistente para el callback ---
    acousea_StatusReportPayload status = acousea_StatusReportPayload_init_default;
    status.modules_count = 3; // número de entradas
    status.modules[0] = eBattery;
    status.modules[1] = eLocation;
    status.modules[2] = eICListen;

    // --- PayloadWrapper (oneof) ---
    acousea_PayloadWrapper pw = acousea_PayloadWrapper_init_default;
    pw.which_payload = acousea_PayloadWrapper_statusPayload_tag;
    pw.payload.statusPayload = status;

    // --- CommunicationPacket ---
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    // Routing
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = 1; // addr;  // nodo origen, ajusta
    pkt.routing.receiver = 0; // backend destino, ajusta
    pkt.routing.ttl = 5;
    // Payload
    pkt.has_payload = true;
    pkt.payload = pw;

    return Result<acousea_CommunicationPacket>::success(pkt);
}
