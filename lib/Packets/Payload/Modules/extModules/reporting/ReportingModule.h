#ifndef ACOUSEA_MKR1310_NODES_REPORTINGMODULE_H
#define ACOUSEA_MKR1310_NODES_REPORTINGMODULE_H

#include <map>
#include <vector>

#include <cstdint>
#include "ErrorHandler/ErrorHandler.h"
#include "Payload/Modules/SerializableModule.h"
#include "Payload/Modules/extModules/reporting/reportingConfiguration/ReportingConfiguration.h"

/** TODO: Reporting modules must serve to identify
 * - The technology used for reporting (LoRa, Iridium)
 * - The reporting periods for each mode
 * - The type of report to be sent (Complete, Basic, Summary)
 */
class ReportingModule : public SerializableModule {
public:
    enum class TechnologyType : uint8_t {
        LORA = 1,
        IRIDIUM = 2
    };


    ReportingModule(TechnologyType type, const std::map<uint8_t, ReportingConfiguration> &configurations);

    // Métodos de acceso
    [[nodiscard]] TechnologyType getTechnologyType() const;

    [[nodiscard]] const std::map<uint8_t, ReportingConfiguration> &getConfigurations() const;

    // Añadir o actualizar una configuración
    void setConfiguration(uint8_t modeId, const ReportingConfiguration &config);

    // Deserialización desde bytes
    static ReportingModule from(const std::vector<uint8_t> &data);

private:
    uint8_t technologyId;  // Tipo de tecnología
    std::map<uint8_t, ReportingConfiguration> configurations; // Configuraciones de reporte

    // Serializar valores
    static std::vector<uint8_t> serializeValues(
            TechnologyType type, const std::map<uint8_t,
            ReportingConfiguration> &configs
    );
};


#endif // ACOUSEA_MKR1310_NODES_REPORTINGMODULE_H
