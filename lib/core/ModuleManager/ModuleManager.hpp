#ifndef ACOUSEA_INFRASTRUCTURE_MKR_MODULE_MANAGER_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_MODULE_MANAGER_HPP

#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "IGPS.h"
#include "IBatteryController.h"
#include "RTCController.hpp"

class ModuleManager
{
    CLASS_NAME(ModuleManager)
    NodeConfigurationRepository& nodeConfigurationRepository;
    ModuleProxy& moduleProxy;
    IGPS& gps;
    IBatteryController& battery;
    RTCController& rtc;

public:
    // Constructor que recibe las dependencias necesarias
    ModuleManager(
        NodeConfigurationRepository& nodeConfigurationRepository,
        ModuleProxy& moduleProxy,
        IGPS& gps,
        IBatteryController& battery,
        RTCController& rtcController
    )
        : nodeConfigurationRepository(nodeConfigurationRepository),
          moduleProxy(moduleProxy),
          gps(gps),
          battery(battery),
          rtc(rtcController)
    {
    }

    [[nodiscard]] Result<void> getModules(acousea_NodeDevice_ModulesEntry* outModulesArr,
                                          pb_size_t& outModulesArrSize,
                                          const acousea_ModuleCode* reqModules,
                                          pb_size_t modulesCount);
    Result<void> setModules(pb_size_t modules_count, const acousea_SetNodeConfigurationPayload_ModulesEntry* modules);

private:
    [[nodiscard]] bool _fetchModuleEntry(
        acousea_NodeDevice_ModulesEntry& outModuleEntry,
        acousea_ModuleCode code,
        uint16_t whichTag,
        ModuleProxy::DeviceAlias alias) const;

    [[nodiscard]] Result<void> _setOperationModes(acousea_NodeConfiguration& nodeConfig,
                                                  const acousea_SetNodeConfigurationPayload_ModulesEntry& moduleEntry);
    [[nodiscard]] Result<void> _setReportTypesModule(acousea_NodeConfiguration& nodeConfig,
                                                     const acousea_SetNodeConfigurationPayload_ModulesEntry&
                                                     moduleEntry);
    [[nodiscard]] Result<void> _setReportingPeriods(acousea_NodeConfiguration& nodeConfig,
                                                    const acousea_SetNodeConfigurationPayload_ModulesEntry&
                                                    moduleEntry);
};

#endif //ACOUSEA_INFRASTRUCTURE_MKR_MODULE_MANAGER_HPP
