#ifndef ACOUSEA_MKR1310_NODES_MODULEFACTORY_H
#define ACOUSEA_MKR1310_NODES_MODULEFACTORY_H

#include "Payload/Modules/extModules/battery/BatteryModule.h"
#include "Payload/Modules/extModules/location/LocationModule.h"
#include "Payload/Modules/extModules/network/NetworkModule.h"
#include <memory>
#include "Payload/Modules/extModules/operationModes/OperationModesModule.h"
#include "Payload/Modules/extModules/rtc/RTCModule.h"
#include "Payload/Modules/extModules/reporting/ReportingModule.h"
#include "Payload/Modules/extModules/storage/StorageModule.h"
#include "Payload/Modules/extModules/ambient/AmbientModule.h"
#include "Payload/Modules/pamModules/ICListen/ICListenHF.h"
#include "Payload/Modules/pamModules/ICListen/implementation/logging/ICListenLoggingConfig.h"
#include "Payload/Modules/pamModules/ICListen/implementation/stats/ICListenRecordingStats.h"
#include "Payload/Modules/pamModules/ICListen/implementation/status/ICListenStatus.h"
#include "Payload/Modules/pamModules/ICListen/implementation/streaming/ICListenStreamingConfig.h"

class ModuleFactory {
public:
    using ModuleCreator = std::function<std::unique_ptr<SerializableModule>(const std::vector<uint8_t>&)>;

    // Create a single module from a buffer
    static std::unique_ptr<SerializableModule> createModule(const std::vector<uint8_t>& buffer);

    // Create multiple modules from a buffer
    static std::vector<std::unique_ptr<SerializableModule>> createModules(const std::vector<uint8_t>& buffer);

    // Registry of module creators
    static const std::map<ModuleCode::TYPES, ModuleCreator> moduleCreators;
};




#endif //ACOUSEA_MKR1310_NODES_MODULEFACTORY_H
