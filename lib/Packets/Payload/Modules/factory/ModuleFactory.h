#ifndef ACOUSEA_MKR1310_NODES_MODULEFACTORY_H
#define ACOUSEA_MKR1310_NODES_MODULEFACTORY_H

#include "Payload/Modules/implementation/battery/BatteryModule.h"
#include "Payload/Modules/implementation/location/LocationModule.h"
#include "Payload/Modules/implementation/network/NetworkModule.h"
#include "Payload/Modules/implementation/operationModes/OperationModesModule.h"
#include "Payload/Modules/implementation/rtc/RTCModule.h"
#include "Payload/Modules/implementation/reporting/ReportingModule.h"
#include "Payload/Modules/implementation/storage/StorageModule.h"
#include "Payload/Modules/implementation/ambient/AmbientModule.h"

class ModuleFactory {
public:
    using ModuleCreator = std::function<SerializableModule(const std::vector<uint8_t> &)>;

    // Create a single module from a buffer
    static SerializableModule createModule(const std::vector<uint8_t> &buffer);

    // Create multiple modules from a buffer
    static std::vector<SerializableModule> createModules(const std::vector<uint8_t> &buffer);

    // Registry of module creators
    static const std::map<ModuleCode::TYPES, ModuleCreator> moduleCreators;
private:
};




#endif //ACOUSEA_MKR1310_NODES_MODULEFACTORY_H
