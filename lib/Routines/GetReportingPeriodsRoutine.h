#ifndef GET_REPORTING_PERIODS_ROUTINE_H
#define GET_REPORTING_PERIODS_ROUTINE_H

#include "IRoutine.h"
#include "../OperationModes/ReportingPeriodManager/ReportingPeriodManager.h"

class GetReportingPeriodsRoutine : public IRoutine {
private:
    ReportingPeriodManager &reportingPeriodManager;

        // Helper function to convert uint16_t to little endian bytes
    void encodeToLittleEndian(uint16_t value, std::vector<uint8_t>& payload) {
        payload.push_back(static_cast<uint8_t>(value & 0xFF));
        payload.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    }

public:
    GetReportingPeriodsRoutine(ReportingPeriodManager &manager)
        : reportingPeriodManager(manager) {}

Packet execute(const Packet &packet) override {
        // Obtener los periodos de reporte actuales para cada modo de operación
        ReportingPeriods launchingPeriods = reportingPeriodManager.getReportingPeriods("Launching");
        ReportingPeriods workingPeriods = reportingPeriodManager.getReportingPeriods("Working");
        ReportingPeriods recoveringPeriods = reportingPeriodManager.getReportingPeriods("Recovering");

        // Crear un payload de respuesta con la configuración actual
        std::vector<uint8_t> responsePayload = { Packet::ResponseCode::ACKNOWLEDGE };

        // Codificar los periodos de reporte en little endian
        encodeToLittleEndian(launchingPeriods.sbd_reporting_period, responsePayload);
        encodeToLittleEndian(launchingPeriods.lora_reporting_period, responsePayload);
        encodeToLittleEndian(workingPeriods.sbd_reporting_period, responsePayload);
        encodeToLittleEndian(workingPeriods.lora_reporting_period, responsePayload);
        encodeToLittleEndian(recoveringPeriods.sbd_reporting_period, responsePayload);
        encodeToLittleEndian(recoveringPeriods.lora_reporting_period, responsePayload);

        // Devolver un paquete de respuesta con los periodos de reporte actuales
        return Packet(packet.getOpCode(), packet.getSwappedAddresses(), responsePayload);
    }
};

#endif // GET_REPORTING_PERIODS_ROUTINE_H
