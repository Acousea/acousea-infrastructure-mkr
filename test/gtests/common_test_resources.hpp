#ifndef ACOUSEA_INFRASTRUCTURE_MKR_COMMON_TEST_RESOURCES_HPP
#define ACOUSEA_INFRASTRUCTURE_MKR_COMMON_TEST_RESOURCES_HPP

#include "StorageManager/StorageManager.hpp"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"

// =====================================================================
// Mock StorageManager sin acceso a disco
// =====================================================================
class MockStorageManager : public StorageManager
{
public:
    std::string lastWrittenFile;
    std::vector<uint8_t> lastWrittenBytes;
    bool simulateReadEmpty = false;
    bool simulateWriteFail = false;
    bool simulateCorruptedRead = false;

    // --- Implementaciones requeridas ---
    bool begin() override { return true; }
    bool appendToFile(const char* path, const std::string& content) override { return true; }
    bool overwriteFile(const char* path, const std::string& content) override { return true; }
    bool deleteFile(const char* path) override { return true; }

    std::string readFile(const char* path) override
    {
        if (simulateReadEmpty) return "";
        return "dummy";
    }

    std::vector<uint8_t> readFileBytes(const char* path) override
    {
        if (simulateReadEmpty) return {};
        if (simulateCorruptedRead) return {0xFF, 0x00, 0xAA};
        return lastWrittenBytes;
    }

    bool writeFileBytes(const char* path, const uint8_t* data, size_t size) override
    {
        if (simulateWriteFail) return false;
        lastWrittenFile = path;
        lastWrittenBytes.assign(data, data + size);
        return true;
    }

    bool writeFileBytes(const char* path, const std::vector<uint8_t>& data) override
    {
        if (simulateWriteFail) return false;
        lastWrittenFile = path;
        lastWrittenBytes = data;
        return true;
    }
};

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
#endif //ACOUSEA_INFRASTRUCTURE_MKR_COMMON_TEST_RESOURCES_HPP
