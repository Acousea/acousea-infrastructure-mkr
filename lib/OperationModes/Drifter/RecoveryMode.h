#ifndef DRIFTERRECOVERYMODE_H
#define DRIFTERRECOVERYMODE_H

#include "../IOperationMode.h"

/**
 * @brief Recovery mode class (DRIFTER)
 * The launching mode is the last mode of operation, immediatly after the working mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 *  - The router is used to send the current location data from the GPS to the localizer.
 *  - The mode a set duration of `LAUNCH_MODE_CYCLES`, after which it will transition to the DRIFTING MODE.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_LAUNCH_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_LAUNCH_SEC`.
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 */

class DrifterRecoveryMode : public IOperationMode
{

private:
    Router *router;
    IGPS *gps;
    IBattery *battery;
    RTCController *rtcController;

private: // Variables to establish periods
    const unsigned long SBD_REPORTING_RECOVERY_PERIOD_SEC = 2147483647L; // 120
    const unsigned long LORA_REPORTING_RECOVERY_PERIOD_SEC = 60; // Just for testing (0 means no reporting)
    unsigned long lastLoraReport = 0;
    unsigned long lastIridiumReport = 0;
    unsigned long lastCycle = 0;
    unsigned long lastCycleTime = 0;

public:
    // Constructor that receives a reference to the display
    DrifterRecoveryMode(IDisplay *display, Router *router, IGPS *gps, IBattery *battery, RTCController *rtcController)
        : IOperationMode(display), router(router), gps(gps), battery(battery), rtcController(rtcController) {}

    void init() override
    {
        display->print("Initializing Recovery Mode..."); // Cambio de "Initializing Recovery Mode..." a "Initializing Recovery Mode...
        // Código de inicialización específico para el modo recovery
    }

    void run() override
    {
        display->print("Running Recovery Mode...");        
        
        router->relayPorts();
        display->print("Ports relayed...");

        // Send SimpleReport both by LORA and IRIDIUM, every REPORTING PERIOD       
        if (millis() - lastLoraReport > LORA_REPORTING_RECOVERY_PERIOD_SEC * 1000)
        {
            lastLoraReport = millis();
            display->print("Sending SimpleReport by LORA...");
            SimpleReport simpleReport = buildSimpleReport();
            SimpleReportPacket LoRASimpleReportPacket(simpleReport, Packet::PacketType::LORA_PACKET); 
            printSimpleReport(simpleReport);           
            router->send(LoRASimpleReportPacket);
        }

        if (millis() - lastIridiumReport > SBD_REPORTING_RECOVERY_PERIOD_SEC * 1000)
        {
            lastIridiumReport = millis();
            display->print("Sending SimpleReport by IRIDIUM...");            
            SimpleReport simpleReport = buildSimpleReport();
            SimpleReportPacket IridiumSimpleReportPacket(simpleReport, Packet::PacketType::IRIDIUM_PACKET);
            printSimpleReport(simpleReport);
            router->send(IridiumSimpleReportPacket);
        }
    }

    void stop() override
    {
        display->print("Stopping Recovery Mode...");
        // Código de limpieza o parada específica para el modo recovery
    }

private:
    SimpleReport buildSimpleReport()
    {
        GPSLocation location = gps->read();
        uint32_t epoch_time = rtcController->getEpoch();
        uint8_t battery_percentage = battery->percentage();
        uint8_t battery_status_and_operation_mode =((battery->status() << 4) & 0xF0) | (OPERATION_MODE::RECOVERY_MODE & 0x0F);
        SimpleReport simpleReport;
        simpleReport.epoch = epoch_time;
        simpleReport.location = location;
        simpleReport.battery_percentage = battery_percentage;
        simpleReport.battery_status_and_operation_mode = battery_status_and_operation_mode;
        return simpleReport;            
    }

        static void printSimpleReport(const SimpleReport &report)
    {
        SerialUSB.println("SimpleReport:");
        SerialUSB.print("  Epoch: ");
        SerialUSB.print(report.epoch);  
        SerialUSB.print("  Battery Percentage: ");
        SerialUSB.print(static_cast<int>(report.battery_percentage));
        SerialUSB.println("%");
        SerialUSB.print("  Battery Status and Operation Mode: ");
        SerialUSB.println(static_cast<int>(report.battery_status_and_operation_mode));
        SerialUSB.print("  Latitude: ");
        SerialUSB.println(report.location.latitude, 6);
        SerialUSB.print("  Longitude: ");
        SerialUSB.println(report.location.longitude, 6);
    }
};

#endif // RECOVERYMODE_H
