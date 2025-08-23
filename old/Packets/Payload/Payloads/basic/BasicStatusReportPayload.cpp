#include "BasicStatusReportPayload.h"

BasicStatusReportPayload::BasicStatusReportPayload(const BatteryModule &battery,
                                                   const LocationModule &location,
                                                   const RTCModule &rtc)
    : battery(battery), location(location), rtc(rtc) {
}

uint16_t BasicStatusReportPayload::getBytesSize() const {
    return static_cast<uint16_t>(battery.toBytes().size() +
                                 location.toBytes().size() +
                                 rtc.toBytes().size());
}

std::vector<uint8_t> BasicStatusReportPayload::toBytes() const {
    std::vector<uint8_t> buffer;
    const auto batteryBytes = battery.toBytes();
    const auto locationBytes = location.toBytes();
    const auto rtcBytes = rtc.toBytes();

    buffer.insert(buffer.end(), batteryBytes.begin(), batteryBytes.end());
    buffer.insert(buffer.end(), locationBytes.begin(), locationBytes.end());
    buffer.insert(buffer.end(), rtcBytes.begin(), rtcBytes.end());

    return buffer;
}
