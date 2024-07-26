#ifndef LOCALIZERLAUNCHINGMODE_H
#define LOCALIZERLAUNCHINGMODE_H

#include "../IOperationMode.h"
#include "../Router/Router.h"
#include "../GPS/IGPS.h"
#include "../Services/SimpleReportService.h"
#include "../RTC/RTCController.h" // Asegúrate de incluir la biblioteca RTC

/**
 * @brief Launching mode class (LOCALIZER)
 * The launching mode is the second mode of operation, immediately after the ignition mode.
 * In this mode, the drifter has been recently launched and is in the process of testing all communication systems are working properly.
 * The localizer will receive the location data from the GPS and send it to the backend.
 */

class LocalizerLaunchingMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
    SimpleReportService* reportService;
    RTCController* rtc; // Añadimos el uso de RTC
public:
    // Constructor que recibe una referencia al display y otros servicios
    LocalizerLaunchingMode(IDisplay* display, Router* router, IGPS* gps, SimpleReportService* reportService, RTCController* rtc) 
    : IOperationMode(display), router(router), gps(gps), reportService(reportService), rtc(rtc) {}

    void init(const ReportingPeriods& rp) override {        
        display->print("Initializing Launching Mode..."); 
        // Código de inicialización específico para el modo launching
    }

    void run() override {
        display->print("Running Launching Mode..."); 

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
        display->print("Stopping Launching Mode...");
        // Código de limpieza o parada específica para el modo launching
    }
};

#endif // LOCALIZERLAUNCHINGMODE_H
