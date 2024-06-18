#include <Arduino.h>
#include "config.h"
#include <Adafruit_SSD1306.h>
#include "../GPSUtils/GPSUtils.h"

class DisplayManager
{
public:
    DisplayManager(Adafruit_SSD1306 &display) : _display(display) {}

    void updateDisplay(local_data_t local, remote_data_t remote)
    {
        double distance_m, bearing_deg;
        char line[25];

        snprintf(line, sizeof(line), "REM Lat: %12.6f", remote.latitude_deg);
        printOnDisplay(false, 1, false, 0, 0, line);

        snprintf(line, sizeof(line), "REM Lon: %12.6f", remote.longitude_deg);
        printOnDisplay(false, 1, false, 0, 8, line);

        snprintf(line, sizeof(line), "LOC Lat: %12.6f", local.latitude_deg);
        printOnDisplay(false, 1, false, 0, 16, line);

        snprintf(line, sizeof(line), "LOC Lon: %12.6f", local.longitude_deg);
        printOnDisplay(false, 1, false, 0, 24, line);

        GPSUtils::HaverSine(local.latitude_deg, local.longitude_deg,
                            remote.latitude_deg, remote.longitude_deg,
                            distance_m, bearing_deg);

        snprintf(line, sizeof(line), "REM Dist: %lu m   ", uint32_t(distance_m));
        printOnDisplay(false, 1, 0, false, 32, line);

        snprintf(line, sizeof(line), "REM bear: %3lu deg N ", uint32_t(bearing_deg));
        printOnDisplay(false, 1, false, 0, 40, line);

        snprintf(line, sizeof(line), "REM batt: %lu%% ", uint32_t(remote.battery_soc));
        printOnDisplay(false, 1, false, 0, 48, line);

        uint32_t timeDiff_sec;
        time_t now_epoch = rtc.getEpoch();
        if (now_epoch > remote.epoch)
            timeDiff_sec = uint32_t(now_epoch - remote.epoch);
        else
            timeDiff_sec = uint32_t(now_epoch - remote.received_epoch);

        snprintf(line, sizeof(line), "                        ");
        printOnDisplay(false, 1, false, 0, 56, line);
        snprintf(line, sizeof(line), "* %lu sec ago", timeDiff_sec);
        printOnDisplay(false, 1, false, 0, 56, line);
    }

    void printOnDisplay(bool erase, uint8_t textSize, bool inverted,
                        uint8_t cursor_x, uint8_t cursor_y, char *text)
    {
        if (erase)
            display.clearDisplay();

        display.setTextSize(textSize); // Normal 1:1 pixel scale

        if (inverted)
        { // Draw 'inverse' text
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
        }
        else
        { // Draw white text
            display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        }

        display.setCursor(cursor_x, cursor_y); // Start at top-left corner
        display.println(text);
        display.display();
    }

private:
    Adafruit_SSD1306 &_display;
};
