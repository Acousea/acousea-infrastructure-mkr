#ifndef SET_REPORTING_PERIODS_ROUTINE_H
#define SET_REPORTING_PERIODS_ROUTINE_H

#include "IRoutine.h"
#include "../OperationModes/ReportingPeriodManager/ReportingPeriodManager.h"
#include <vector>

class SetReportingPeriodsRoutine : public IRoutine {
private:
    ReportingPeriodManager &reportingPeriodManager;

    // Helper function to decode uint16_t from little endian bytes
    uint16_t decodeFromLittleEndian(const uint8_t* payload, size_t index) {
        return static_cast<uint16_t>(payload[index]) | (static_cast<uint16_t>(payload[index + 1]) << 8);
    }

    // Helper function to encode uint16_t to little endian bytes
    void encodeToLittleEndian(uint16_t value, std::vector<uint8_t>& payload) {
        payload.push_back(static_cast<uint8_t>(value & 0xFF));
        payload.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }

public:
    SetReportingPeriodsRoutine(ReportingPeriodManager &manager)
        : reportingPeriodManager(manager) {}

    Packet execute(const Packet &packet) override {
        // Leer los nuevos periodos de reporte del paquete
        const uint8_t* payload = packet.getPayload();
        uint16_t launchingSbdPeriod = decodeFromLittleEndian(payload, 0);
        uint16_t launchingLoraPeriod = decodeFromLittleEndian(payload, 2);
        uint16_t workingSbdPeriod = decodeFromLittleEndian(payload, 4);
        uint16_t workingLoraPeriod = decodeFromLittleEndian(payload, 6);
        uint16_t recoveringSbdPeriod = decodeFromLittleEndian(payload, 8);
        uint16_t recoveringLoraPeriod = decodeFromLittleEndian(payload, 10);

        // Actualizar los periodos de reporte en el ReportingPeriodManager
        reportingPeriodManager.updateCustomValues("Launching", launchingSbdPeriod, launchingLoraPeriod);
        reportingPeriodManager.updateCustomValues("Working", workingSbdPeriod, workingLoraPeriod);
        reportingPeriodManager.updateCustomValues("Recovering", recoveringSbdPeriod, recoveringLoraPeriod);

        // Crear un payload de respuesta con la configuración actual
        std::vector<uint8_t> responsePayload = { Packet::ResponseCode::ACKNOWLEDGE };

        // Codificar los periodos de reporte en little endian
        encodeToLittleEndian(launchingSbdPeriod, responsePayload);
        encodeToLittleEndian(launchingLoraPeriod, responsePayload);
        encodeToLittleEndian(workingSbdPeriod, responsePayload);
        encodeToLittleEndian(workingLoraPeriod, responsePayload);
        encodeToLittleEndian(recoveringSbdPeriod, responsePayload);
        encodeToLittleEndian(recoveringLoraPeriod, responsePayload);

        // Devolver un paquete de respuesta con los periodos de reporte actuales
        return Packet(packet.getOpCode(), packet.getSwappedAddresses(), responsePayload);
    }
};

#endif // SET_REPORTING_PERIODS_ROUTINE_H
