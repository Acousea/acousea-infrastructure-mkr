#ifndef LOCALIZERRECOVERYMODE_H
#define LOCALIZERRECOVERYMODE_H

#include "../IOperationMode.h"
#include "../Router/Router.h"
#include "../GPS/IGPS.h"
#include "../Services/SimpleReportService.h"
#include "../RTC/RTCController.h" // Asegúrate de incluir la biblioteca RTC

/**
 * @brief Recovery mode class (DRIFTER)
 * The recovery mode is the last mode of operation, immediately after the drifting mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 *  - The router is used to send the current location data from the GPS to the localizer. 
 *  - The mode has a set duration of `LAUNCH_MODE_CYCLES`, after which it will transition to the DRIFTING MODE.
 *  - It has a set IRIDIUM reporting period of `SBD_REPORTING_LAUNCH_SEC`.
 *  - It has a set LORA reporting period of `LORA_REPORTING_LAUNCH_SEC`.  
 *  - It will listen for incoming packets from both LORA and IRIDIUM ports.
 */

class LocalizerRecoveryMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
    SimpleReportService* reportService;
    RTCController* rtc; // Añadimos el uso de RTC

public:
    // Constructor que recibe una referencia al display y otros servicios
    LocalizerRecoveryMode(IDisplay* display, Router* router, IGPS* gps, SimpleReportService* reportService, RTCController* rtc) 
        : IOperationMode(display), router(router), gps(gps), reportService(reportService), rtc(rtc) {}

    void init(const ReportingPeriods& rp) override {        
        display->print("Initializing Recovery Mode..."); 
        // Código de inicialización específico para el modo recovery
    }

    void run() override {
        display->print("Running Recovery Mode...");

        if (!reportService->newReportAvailable()) {
            display->print("No new report available.");
            return;
        }

        GPSLocation local = gps->read();
        SimpleReport report = reportService->popReport();

        float distance_m, bearing_deg;
        char line[25];

        snprintf(line, sizeof(line), "REM Lat: %12.6f", report.location.latitude);
        display->print(line);

        snprintf(line, sizeof(line), "REM Lon: %12.6f", report.location.longitude);
        display->print(line);

        snprintf(line, sizeof(line), "LOC Lat: %12.6f", local.latitude);
        display->print(line);

        snprintf(line, sizeof(line), "LOC Lon: %12.6f", local.longitude);
        display->print(line);

       // Compute distance and bearing from local to remote
        IGPS::HaverSine(double(local.latitude), double(local.longitude), 
                  double(report.location.latitude), double(report.location.longitude), 
                  distance_m, bearing_deg);         

        snprintf(line, sizeof(line), "REM Dist: %lu m   ", uint32_t(distance_m));
        display->print(line);

        snprintf(line, sizeof(line), "REM bear: %3lu deg N ", uint32_t(bearing_deg));
        display->print(line);

        snprintf(line, sizeof(line), "REM batt: %lu%% ", uint32_t(report.battery_percentage));
        display->print(line);

        // Calculate time lag
        uint32_t timeDiff_sec;
        time_t now_epoch = rtc->getEpoch();
        if (now_epoch > report.epoch) 
            timeDiff_sec = uint32_t(now_epoch - report.epoch);
        else 
            timeDiff_sec = uint32_t(now_epoch - report.epoch);

        snprintf(line, sizeof(line), "* %lu sec ago", timeDiff_sec);
        display->print(line);

        router->relayPorts();
    }

    void stop() override {
        display->print("Stopping Recovery Mode...");
        // Código de limpieza o parada específica para el modo recovery
    }
};

#endif // LOCALIZERRECOVERYMODE_H
