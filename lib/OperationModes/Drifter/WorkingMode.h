#ifndef DRIFTERDRIFTINGMODE_H
#define DRIFTERDRIFTINGMODE_H

#include "../IOperationMode.h"

#include "../Services/SummaryService.h"
#include "../Packet/SummaryRequestPacket.h"
#include "../Packet/SummaryReportPacket.h"
#include "../RTC/RTCController.h"


/**
 * @brief Working mode class (DRIFTER)
 * The working mode is the third mode of operation, immediatly after the launching mode.
 * In this mode, the drifter has been successfully launched and is in the working state.
 *  - The router is used to send periodic status reports to the backend, including:
 *     - Current location data from the GPS.
 *     - Current battery status and system health.
 *     - AudioDetectionStats data from the IC-Listen sensor. 
 
 *  - The mode an indefinite duration, until the drifter is recovered.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_DRIFTING_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_DRIFTING_SEC` just for testing (It will not be used in real deployment).
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 * 
 *  - It will send periodic commands to the IC-Listen sensor to start/stop logging data. (commands sent to PI3 API)
 */

class DrifterWorkingMode : public IOperationMode {

private:
    Router* router;
    IGPS* gps;
    IBattery* battery;
    RTCController* rtcController;
    SummaryService* summaryService;

private: // Variables to establish periods
    unsigned long SBD_REPORTING_PERIOD;
    unsigned long LORA_REPORTING_PERIOD;
    unsigned long lastLoraReport = 0;
    unsigned long lastIridiumReport = 0;

public:
    DrifterWorkingMode(IDisplay* display, Router* router, IGPS* gps, IBattery* battery, RTCController* rtcController, SummaryService* summaryService)
    : IOperationMode(display), router(router), gps(gps), battery(battery), rtcController(rtcController), summaryService(summaryService),
        SBD_REPORTING_PERIOD(0), LORA_REPORTING_PERIOD(0)
    {}

    void init(const ReportingPeriods& rp) override {
        if (rp.sbd_reporting_period != 0) {
            SBD_REPORTING_PERIOD = rp.sbd_reporting_period * 60 * 1000; // Convert to milliseconds
        }
        if (rp.lora_reporting_period != 0) {
            LORA_REPORTING_PERIOD = rp.lora_reporting_period * 60 * 1000; // Convert to milliseconds
        }

        std::string initMessage = "Initializing Working Mode with SBD Reporting Period: " + std::to_string(rp.sbd_reporting_period) 
                                    + " min and LORA Reporting Period: " + std::to_string(rp.lora_reporting_period) + " min";
        display->print(initMessage.c_str());        
    }

    void run() override {
        display->print("Running Working Mode...");
        router->relayPorts();  
        display->print("Ports relayed...");

        unsigned long currentTime = millis();

        if (summaryService->newSummaryAvailable()) {
            sendSummaryReportWhenAvailable(currentTime);
        }

        if (LORA_REPORTING_PERIOD != 0 && currentTime - lastLoraReport >= LORA_REPORTING_PERIOD) {
            display->print("-----> Sending report request for LoRa...");
            sendSummaryReportRequest(Packet::PacketType::LORA_PACKET);
            lastLoraReport = currentTime;
        }   

        if (SBD_REPORTING_PERIOD != 0 && currentTime - lastIridiumReport >= SBD_REPORTING_PERIOD) {
            display->print("-----> Sending report request for Iridium...");
            sendSummaryReportRequest(Packet::PacketType::IRIDIUM_PACKET);
            lastIridiumReport = currentTime;
        }
    }

    void stop() override {
        display->print("Stopping Working Mode...");
        // Specific cleanup or stop code for drifting mode
    }

    void sendSummaryReportRequest(Packet::PacketType packetType) {
        Packet packet = SummaryRequestPacket(packetType); // Packet to the raspberry, should respond with a summary packet
        router->send(packet);
    }

    void sendSummaryReportWhenAvailable(unsigned long currentTime) {
        if (!summaryService->newSummaryAvailable()) { // Wait for the summary packet
            display->print("No summary available...");
            return;
        }

        display->print("AudioDetectionStats available. Building and sending report packet...");

        // Get the available summary
        SummaryResponse summary = summaryService->popSummary();

        // Build the summary report
        SummaryReport report = buildSummaryReport(summary);

        // Print the summary report
        printSummaryReport(report);

        // Send the report packet via LoRa if applicable
        if (LORA_REPORTING_PERIOD != 0 && currentTime - lastLoraReport >= LORA_REPORTING_PERIOD) {
            display->print("=====> Sending report packet via LoRa...");
            Packet reportPacketLora = SummaryReportPacket(report, Packet::PacketType::LORA_PACKET);
            router->send(reportPacketLora);
            lastLoraReport = currentTime;
        }

        // Send the report packet via Iridium if applicable
        if (SBD_REPORTING_PERIOD != 0 && currentTime - lastIridiumReport >= SBD_REPORTING_PERIOD) {
            display->print("=====> Sending report packet via Iridium...");
            Packet reportPacketIridium = SummaryReportPacket(report, Packet::PacketType::IRIDIUM_PACKET);
            router->send(reportPacketIridium);
            lastIridiumReport = currentTime;
        }
    }

    SummaryReport buildSummaryReport(const SummaryResponse& summary) {
        // Get GPS and RTC data
        GPSLocation location = gps->read();
        uint32_t epoch_time = rtcController->getEpoch();

        // Build the DrifterModuleStats with the available data
        DrifterModuleStats drifter_module_stats;
        drifter_module_stats.temperature = summary.pi3_temperature;
        drifter_module_stats.BatteryPercentage = battery->percentage();
        drifter_module_stats.BatteryStatus_OpMode = ((battery->status() << 4) & 0xF0) | (0x0F & OPERATION_MODE::WORKING_MODE);
        drifter_module_stats.location = location;
        drifter_module_stats.pi3_storage = summary.pi3_storage;

        // Build the SummaryReport
        SummaryReport report;
        report.epoch_time = epoch_time;
        report.pam_device = summary.pam_device;
        report.drifter_module_stats = drifter_module_stats;
        report.audio_detection = summary.audio_detection;

        return report;
    }

    void printSummaryReport(const SummaryReport &report) {
        SerialUSB.println("SummaryReport:");
        SerialUSB.print("  Epoch Time: ");
        SerialUSB.println(report.epoch_time);

        SerialUSB.println("  PAM Device Stats:");
        SerialUSB.print("    Unit Status and Battery Status: ");
        SerialUSB.println(static_cast<int>(report.pam_device.UnitStatus_BatteryStatus));
        SerialUSB.print("    Battery Percentage: ");
        SerialUSB.println(static_cast<int>(report.pam_device.BatteryPercentage));
        SerialUSB.print("    Temperature: ");
        SerialUSB.println(static_cast<int>(report.pam_device.temperature));
        SerialUSB.print("    Humidity: ");
        SerialUSB.println(static_cast<int>(report.pam_device.humidity));

        SerialUSB.println("  Drifter Module Stats:");
        SerialUSB.print("    Temperature: ");
        SerialUSB.println(static_cast<int>(report.drifter_module_stats.temperature));
        SerialUSB.print("    Battery Percentage: ");
        SerialUSB.println(static_cast<int>(report.drifter_module_stats.BatteryPercentage));
        SerialUSB.print("    Battery Status and Operation Mode: ");
        SerialUSB.println(static_cast<int>(report.drifter_module_stats.BatteryStatus_OpMode));
        SerialUSB.print("    Latitude: ");
        SerialUSB.println(report.drifter_module_stats.location.latitude, 6);
        SerialUSB.print("    Longitude: ");
        SerialUSB.println(report.drifter_module_stats.location.longitude, 6);
        
        SerialUSB.println("  PI3 Storage Stats:");
        SerialUSB.print("    Total Storage (MB): ");
        SerialUSB.println(report.drifter_module_stats.pi3_storage.total_storage_mb);
        SerialUSB.print("    Used Storage (MB): ");
        SerialUSB.println(report.drifter_module_stats.pi3_storage.used_storage_mb);

        SerialUSB.println("  Audio Detection Stats:");
        SerialUSB.print("    Total Number of Detections: ");
        SerialUSB.println(report.audio_detection.totalNumDetections);
        SerialUSB.print("    Recorded Minutes: ");
        SerialUSB.println(report.audio_detection.recordedMinutes);
        SerialUSB.print("    Processed Minutes: ");
        SerialUSB.println(report.audio_detection.processedMinutes);
        SerialUSB.print("    Number of Files: ");
        SerialUSB.println(report.audio_detection.numFiles);
    }
};

#endif // DRIFTINGMODE_H
