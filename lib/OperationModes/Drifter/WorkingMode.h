#ifndef DRIFTERDRIFTINGMODE_H
#define DRIFTERDRIFTINGMODE_H

#include "../IOperationMode.h"

#include "../Services/SummaryService.h"
#include "../Packet/SummaryRequestPacket.h"
#include "../Packet/ReportPacket.h"
#include "../RTC/RTCController.h"

/**
 * @brief Working mode class (DRIFTER)
 * The working mode is the third mode of operation, immediatly after the launching mode.
 * In this mode, the drifter has been successfully launched and is in the working state.
 *  - The router is used to send periodic status reports to the backend, including:
 *     - Current location data from the GPS.
 *     - Current battery status and system health.
 *     - Summary data from the IC-Listen sensor. 
 
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
    RTCController* rtcController;
    SummaryService* summaryService;

private: // Variables to establish periods
    const unsigned SBD_REPORTING_DRIFTING_PERIOD_SEC = 900;
    const unsigned long LORA_REPORTING_DRIFTING_PERIOD_SEC = 60;  // Just for testing (0 means no reporting)
    unsigned long lastLoraReport = 0;
    unsigned long lastIridiumReport = 0;
    unsigned long lastCycle = 0;
    unsigned long lastCycleTime = 0;

public:       
    DrifterWorkingMode(IDisplay* display, Router* router, IGPS* gps, RTCController* rtcController, SummaryService* summaryService) 
    : IOperationMode(display), router(router), gps(gps), rtcController(rtcController), summaryService(summaryService) {}


    void init() override {        
        display->print("Initializing Drifting Mode..."); // Cambio de "Initializing Drifting Mode..." a "Initializing Drifting Mode...
        // Código de inicialización específico para el modo drifting
    }

    void run() override {
        display->print("Running Drifting Mode...");
        router->relayPorts();  

        display->print("Ports relayed...");        

        if (millis() - lastIridiumReport >= SBD_REPORTING_DRIFTING_PERIOD_SEC * 1000) {
            display->print("Sending report request...");
            // sendReportRequest();
            lastIridiumReport = millis();
        }   
        // sendReportWhenAvailable();       
    }

    void stop() override {
        display->print("Stopping Drifting Mode...");
        // Código de limpieza o parada específica para el modo drifting
    }

    void sendReportRequest(){
        Packet packet = SummaryRequestPacket(Packet::PacketType::IRIDIUM_PACKET); // -> Packet to the raspberry, should respond with a summary packet
        router->send(packet);
    }

    void sendReportWhenAvailable(){                
        if (!summaryService->newSummaryAvailable()) { // Wait for the the summary packet                  
            display->print("No summary available...");
            return;           
        } 

        display->print("Summary available. Building and sending report packet...");        
                     
        // Finally we send the location and the summary to the backend as a report through the iridium port (or also through the lora port)
        GPSLocation location = gps->read();
        Summary summary = summaryService->popSummary();     
        time_t dateTimestamp = rtcController->getEpoch();
        Report report = {dateTimestamp, OPERATION_MODE::WORKING_MODE, location, summary};
        Packet reportPacket = ReportPacket(report, Packet::PacketType::IRIDIUM_PACKET);

        router->send(reportPacket);
    }
};

#endif // DRIFTINGMODE_H
