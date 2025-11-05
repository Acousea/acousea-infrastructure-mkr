#ifndef ACOUSEA_INFRASTRUCTURE_MKR_TESTABLENODECONFIGURATIONREPOSITORY_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_TESTABLENODECONFIGURATIONREPOSITORY_HPP

#include "NodeConfigurationRepository/NodeConfigurationRepository.h"

class TestableNodeConfigurationRepository : public NodeConfigurationRepository
{
public:
    using NodeConfigurationRepository::NodeConfigurationRepository;
    using NodeConfigurationRepository::makeDefault; // exposes the private method
    using NodeConfigurationRepository::encodeProto; // exposes the private method
    using NodeConfigurationRepository::decodeProto; // exposes the private method

    // =====================================================================
    // Helpers
    // =====================================================================
    explicit TestableNodeConfigurationRepository(StorageManager& sm)
        : NodeConfigurationRepository(sm)
    { }

    // Genera una configuración válida con Battery, Location y ICListenHF
    static acousea_NodeConfiguration makeValidNodeConfig()
    {
        acousea_NodeConfiguration cfg = TestableNodeConfigurationRepository::makeDefault();

        auto& rt = cfg.reportTypesModule.reportTypes[0];
        rt.includedModules_count = 3;
        rt.includedModules[0] = acousea_ModuleCode_BATTERY_MODULE;
        rt.includedModules[1] = acousea_ModuleCode_LOCATION_MODULE;
        rt.includedModules[2] = acousea_ModuleCode_ICLISTEN_HF;

        return cfg;
    }
};


#endif //ACOUSEA_INFRASTRUCTURE_MKR_TESTABLENODECONFIGURATIONREPOSITORY_HPP
