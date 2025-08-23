#include "CompleteStatusReportPayload.h"

CompleteStatusReportPayload::CompleteStatusReportPayload(const BatteryModule &battery, const AmbientModule &ambient,
                                                         const LocationModule &location, const StorageModule &storage,
                                                         const PamModule &pamModule)
        : battery(battery), ambient(ambient), location(location), storage(storage), pamModule(pamModule) {}

uint16_t CompleteStatusReportPayload::getBytesSize() const {
    return static_cast<uint16_t>(
            battery.toBytes().size() +
            ambient.toBytes().size() +
            location.toBytes().size() +
            storage.toBytes().size() +
            pamModule.toBytes().size());
}

std::vector<uint8_t> CompleteStatusReportPayload::toBytes() const {
    std::vector<uint8_t> buffer;

    // Serializar cada módulo y añadir los bytes al buffer
    appendModuleBytes(buffer, battery.toBytes());
    appendModuleBytes(buffer, ambient.toBytes());
    appendModuleBytes(buffer, location.toBytes());
    appendModuleBytes(buffer, storage.toBytes());
    appendModuleBytes(buffer, pamModule.toBytes());

    return buffer;
}

void
CompleteStatusReportPayload::appendModuleBytes(std::vector<uint8_t> &buffer, const std::vector<uint8_t> &moduleBytes) {
    buffer.insert(buffer.end(), moduleBytes.begin(), moduleBytes.end());
}
