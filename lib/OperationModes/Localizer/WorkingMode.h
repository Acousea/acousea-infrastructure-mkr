#ifndef LOCALIZERDRIFTINGMODE_H
#define LOCALIZERDRIFTINGMODE_H

#include "../IOperationMode.h"
#include "../Services/ReportService.h"
#include "../RTC/RTCController.h"

/**
 * @brief Working mode class (LOCALIZER) 
 */

class LocalizerWorkingMode : public IOperationMode {
private:
    Router* router;
    IGPS* gps;
    ReportService* reportService;
    RTCController* rtc;
public:
    // Constructor that receives a reference to the display
    LocalizerWorkingMode(IDisplay* display, Router* router, IGPS* gps, ReportService* reportService, RTCController* rtc) 
        : IOperationMode(display), router(router), gps(gps), reportService(reportService), rtc(rtc) {}

    void init(const ReportingPeriods& rp) override {        
        display->print("Initializing Working Mode..."); // Cambio de "Initializing Drifting Mode..." a "Initializing Drifting Mode...
        // Código de inicialización específico para el modo drifting
    }

    void run() override {
        display->print("Running Working Mode...");
        // Código específico para ejecutar en modo working
        router->relayPorts();
        GPSLocation location = gps->read();
        SummaryReport remote = reportService->popReport();


        float distance_m, bearing_deg;
        char line[25];

        snprintf(line, sizeof(line), "REM Lat: %12.6f", remote.drifter_module_stats.location.latitude);
        display->print(line);

        snprintf(line, sizeof(line), "REM Lon: %12.6f", remote.drifter_module_stats.location.longitude);
        display->print(line);

        snprintf(line, sizeof(line), "LOC Lat: %12.6f", location.latitude);
        display->print(line);

        snprintf(line, sizeof(line), "LOC Lon: %12.6f", location.longitude);
        display->print(line);

        // Compute distance and bearing from local to remote
        IGPS::HaverSine(double(location.latitude), double(location.longitude), 
                  double(remote.drifter_module_stats.location.latitude), double(remote.drifter_module_stats.location.longitude), 
                  distance_m, bearing_deg);

        snprintf(line, sizeof(line), "REM Dist: %lu m   ", uint32_t(distance_m));
        display->print(line);

        snprintf(line, sizeof(line), "REM bear: %3lu deg N ", uint32_t(bearing_deg));
        display->print(line);

        snprintf(line, sizeof(line), "REM batt: %lu%% ", uint32_t(remote.drifter_module_stats.BatteryPercentage));
        display->print(line);

        // Calculate time lag
        uint32_t timeDiff_sec;
        time_t now_epoch = rtc->getEpoch();
        if (now_epoch > remote.epoch_time) 
            timeDiff_sec = uint32_t(now_epoch - remote.epoch_time);
        else 
            timeDiff_sec = uint32_t(now_epoch - remote.epoch_time);

        snprintf(line, sizeof(line), "* %lu sec ago", timeDiff_sec);
        display->print(line);
    }

    void stop() override {
        display->print("Stopping Working Mode...");
        // Código de limpieza o parada específica para el modo drifting
    }
};

#endif // DRIFTINGMODE_H
