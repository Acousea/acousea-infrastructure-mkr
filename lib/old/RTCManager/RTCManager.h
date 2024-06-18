#ifndef RTCMANAGER_H
#define RTCMANAGER_H

#include <time.h>
#include <Arduino.h>
#include <RTCZero.h>

class RTCManager {
public:
    RTCManager(RTCZero &rtc) : _rtc(rtc) {}

    bool setDateTime(const char *date_str, const char *time_str) {
        char month_str[4];
        char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        uint16_t i, mday, month, hour, min, sec, year;

        if (sscanf(date_str, "%3s %hu %hu", month_str, &mday, &year) != 3) return false;
        if (sscanf(time_str, "%hu:%hu:%hu", &hour, &min, &sec) != 3) return false;

        for (i = 0; i < 12; i++) {
            if (!strncmp(month_str, months[i], 3)) {
                month = i + 1;
                break;
            }
        }
        if (i == 12) return false;

        _rtc.setTime((uint8_t)hour, (uint8_t)min, (uint8_t)sec);
        _rtc.setDate((uint8_t)mday, (uint8_t)month, (uint8_t)(year - 2000));
        return true;
    }

    void getDateTime(char *dateTimeStr, size_t dateTimeStr_sz) {
        struct tm stm;
        time_t epoch = _rtc.getEpoch();
        gmtime_r(&epoch, &stm);
        snprintf(dateTimeStr, dateTimeStr_sz,"%4u/%02u/%02u %02u:%02u:%02u",
                 stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday, 
                 stm.tm_hour, stm.tm_min, stm.tm_sec);
    }

    void getDateStr(char *dateStr, size_t dateStr_sz) {
        struct tm stm;
        time_t epoch = _rtc.getEpoch();
        gmtime_r(&epoch, &stm);
        snprintf(dateStr, dateStr_sz,"%4u%02u%02u",
                 stm.tm_year + 1900, stm.tm_mon + 1, stm.tm_mday);
    }

private:
    RTCZero &_rtc;
};

#endif