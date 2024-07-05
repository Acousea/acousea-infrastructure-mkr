#ifndef DRIFTERLAUNCHINGMODE_H
#define DRIFTERLAUNCHINGMODE_H

#include "../IOperationMode.h"
#include "../Router/Router.h"
#include "../GPS/IGPS.h"
#include "../Packet/SimpleReportPacket.h"


/**
 * @brief Launching mode class (DRIFTER)
 * The launching mode is the second mode of operation, immediatly after the ignition mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 *  - The router is used to send the current location data from the GPS to the localizer. 
 *  - The mode a set duration of `LAUNCH_MODE_CYCLES`, after which it will transition to the DRIFTING MODE.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_LAUNCH_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_LAUNCH_SEC`.  
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 */

class DrifterLaunchingMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
    RTCController* rtcController;

private: // Variables to establish periods
    const unsigned long SBD_REPORTING_LAUNCHING_PERIOD_SEC = 1000000;
    const unsigned long LORA_REPORTING_LAUNCHING_PERIOD_SEC = 60; // Just for testing (0 means no reporting)
    unsigned long lastLoraReport = 0;
    unsigned long lastIridiumReport = 0;
    unsigned long lastCycle = 0;
    unsigned long lastCycleTime = 0;   
    
public:
    // Constructor that receives a reference to the display
    DrifterLaunchingMode(IDisplay* display, Router* router, IGPS* gps, RTCController* rtcController) 
    : IOperationMode(display), router(router), gps(gps), rtcController(rtcController) {}

    void init() override {        
        display->print("Initializing Launching Mode..."); // Cambio de "Initializing Launching Mode..." a "Initializing Launching Mode...
        // Código de inicialización específico para el modo launching
    }

    void run() override {
        display->print("Running Launching Mode...");      
        
        router->relayPorts();  

        display->print("Ports relayed...");        

        if (millis() - lastIridiumReport < SBD_REPORTING_LAUNCHING_PERIOD_SEC * 1000) {
            return;
        }   

        lastIridiumReport = millis();

        // TODO: Send these packets
        // Build SimpleReport struct
        // SimpleReport simpleReport;

        // simpleReport.battery_percentage = 100;
        // simpleReport.operation_mode = OPERATION_MODE::LAUNCHING_MODE;
        // simpleReport.epoch = rtcController->getEpoch();
        // simpleReport.location = gps->read();

        // // Build SimpleReportPackets
        // SimpleReportPacket simpleReportPacketIridium(simpleReport, Packet::PacketType::IRIDIUM_PACKET);
        // SimpleReportPacket simpleReportPacketLoRa(simpleReport, Packet::PacketType::LORA_PACKET);       

        display->print("Sending simple status report ");            
            
        
    }    

 
    void stop() override {
        display->print("Stopping Launching Mode...");
        // Código de limpieza o parada específica para el modo launching
    }    
    
};

#endif // LAUNCHINGMODE_H
