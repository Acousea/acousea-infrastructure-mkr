#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif



#include <gtest/gtest.h>
#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ErrorHandler/ErrorHandler.h"

// ................. Common test resources ..................
#include "../common_test_resources/InMemoryStorageManager.hpp"
#include "../common_test_resources/TestableNodeConfigurationRepository.hpp"


// =====================================================================
// Fixture para NodeConfigurationRepository
// =====================================================================
class NodeConfigurationRepositoryTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Evitar aborts durante los tests
        ErrorHandler::setHandler([](const char* msg)
        {
            fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg);
        });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};

// =====================================================================
// TESTS
// =====================================================================

// encode/decode deben ser simétricos
TEST_F(NodeConfigurationRepositoryTest, EncodeDecodeSymmetry)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 123;

    auto enc = TestableNodeConfigurationRepository::encodeProto(cfg);
    ASSERT_TRUE(enc.isSuccess());

    const auto& bytes = enc.getValue();
    auto dec = TestableNodeConfigurationRepository::decodeProto(bytes.data(), bytes.size());
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 123);
}

// saveConfiguration escribe bytes válidos
TEST_F(NodeConfigurationRepositoryTest, SaveConfigurationWritesToStorageManager)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 42;

    bool ok = repo.saveConfiguration(cfg);
    EXPECT_TRUE(ok);

    EXPECT_TRUE(storageManager.exists("nodeconf"));

    uint8_t buffer[1024];
    size_t bytesRead = storageManager.readFileBytes("nodeconf", buffer, sizeof(buffer));
    EXPECT_GT(bytesRead, 0u);

    auto dec = TestableNodeConfigurationRepository::decodeProto(buffer, bytesRead);
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 42);
}

// saveConfiguration falla si writeFileBytes devuelve false
TEST_F(NodeConfigurationRepositoryTest, SaveConfigurationFailsWhenStorageIsClearedMidWrite)
{
    class FailingStorage : public InMemoryStorageManager {
    public:
        bool failNextWrite = true;
        bool writeFileBytes(const char* path, const uint8_t* data, size_t length) override {
            if (failNextWrite) { failNextWrite = false; return false; }
            return InMemoryStorageManager::writeFileBytes(path, data, length);
        }
    };

    FailingStorage storageManager;
    NodeConfigurationRepository repo(storageManager);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    bool ok = repo.saveConfiguration(cfg);
    EXPECT_FALSE(ok);
}

// getNodeConfiguration devuelve default si storage está vacío
TEST_F(NodeConfigurationRepositoryTest, GetNodeConfigurationReturnsDefaultIfEmpty)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    auto cfg = repo.getNodeConfiguration();
    EXPECT_EQ(cfg.localAddress, 255);
    EXPECT_TRUE(cfg.has_reportTypesModule);
    EXPECT_TRUE(cfg.has_operationModesModule);
    EXPECT_TRUE(cfg.has_iridiumModule);
}

// getNodeConfiguration devuelve default si bytes corruptos
TEST_F(NodeConfigurationRepositoryTest, GetNodeConfigurationReturnsDefaultIfCorrupted)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    // escribimos bytes basura para simular corrupción
    const uint8_t corrupt[] = {0xFF, 0x00, 0xAA, 0xBB};
    storageManager.writeFileBytes("nodeconf", corrupt, sizeof(corrupt));

    auto cfg = repo.getNodeConfiguration();
    EXPECT_EQ(cfg.localAddress, 255);
}


// init() crea default si no hay fichero
TEST_F(NodeConfigurationRepositoryTest, InitCreatesDefaultWhenMissing)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    repo.init();

    EXPECT_TRUE(storageManager.exists("nodeconf"));

    uint8_t buffer[1024];
    size_t bytesRead = storageManager.readFileBytes("nodeconf", buffer, sizeof(buffer));
    EXPECT_GT(bytesRead, 0u);

    auto dec = TestableNodeConfigurationRepository::decodeProto(buffer, bytesRead);
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 255);
}


// reset() guarda makeDefault
TEST_F(NodeConfigurationRepositoryTest, ResetStoresDefaultConfig)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    repo.reset();

    uint8_t buffer[1024];
    size_t bytesRead = storageManager.readFileBytes("nodeconf", buffer, sizeof(buffer));
    ASSERT_GT(bytesRead, 0u);

    auto dec = TestableNodeConfigurationRepository::decodeProto(buffer, bytesRead);
    ASSERT_TRUE(dec.isSuccess());
    auto cfg = dec.getValueConst();

    EXPECT_EQ(cfg.localAddress, 255);
    EXPECT_TRUE(cfg.has_reportTypesModule);
    EXPECT_EQ(cfg.reportTypesModule.reportTypes_count, 1);
    EXPECT_STREQ(cfg.reportTypesModule.reportTypes[0].name, "BasicRep");
}


// printNodeConfiguration no lanza
TEST_F(NodeConfigurationRepositoryTest, PrintNodeConfigurationDoesNotCrash)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    auto cfg = TestableNodeConfigurationRepository::makeDefault();
    EXPECT_NO_THROW(repo.printNodeConfiguration(cfg));
}
// printNodeConfiguration genera salida coherente
TEST_F(NodeConfigurationRepositoryTest, PrintNodeConfigurationPrintsRichConfiguration)
{
    InMemoryStorageManager storageManager;
    NodeConfigurationRepository repo(storageManager);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 99;

    cfg.has_operationModesModule = true;
    cfg.operationModesModule.modes_count = 2;
    cfg.operationModesModule.activeModeId = 1;

    acousea_OperationMode& mode0 = cfg.operationModesModule.modes[0];
    mode0.id = 1;
    strcpy(mode0.name, "Standby");
    mode0.reportTypeId = 1;
    mode0.has_transition = true;
    mode0.transition.targetModeId = 2;
    mode0.transition.duration = 120;

    acousea_OperationMode& mode1 = cfg.operationModesModule.modes[1];
    mode1.id = 2;
    strcpy(mode1.name, "Active");
    mode1.reportTypeId = 2;
    mode1.has_transition = false;

    cfg.has_reportTypesModule = true;
    cfg.reportTypesModule.reportTypes_count = 2;

    acousea_ReportType& rpt0 = cfg.reportTypesModule.reportTypes[0];
    rpt0.id = 1;
    strcpy(rpt0.name, "BasicReport");
    rpt0.includedModules_count = 2;
    rpt0.includedModules[0] = acousea_ModuleCode_AMBIENT_MODULE;
    rpt0.includedModules[1] = acousea_ModuleCode_BATTERY_MODULE;

    acousea_ReportType& rpt1 = cfg.reportTypesModule.reportTypes[1];
    rpt1.id = 2;
    strcpy(rpt1.name, "FullReport");
    rpt1.includedModules_count = 3;
    rpt1.includedModules[0] = acousea_ModuleCode_NETWORK_MODULE;
    rpt1.includedModules[1] = acousea_ModuleCode_LOCATION_MODULE;
    rpt1.includedModules[2] = acousea_ModuleCode_STORAGE_MODULE;

    cfg.has_loraModule = true;
    cfg.loraModule.entries_count = 2;
    cfg.loraModule.entries[0].modeId = 1;
    cfg.loraModule.entries[0].period = 10;
    cfg.loraModule.entries[1].modeId = 2;
    cfg.loraModule.entries[1].period = 20;

    cfg.has_iridiumModule = true;
    strcpy(cfg.iridiumModule.imei, "111122223333444");
    cfg.iridiumModule.entries_count = 2;
    cfg.iridiumModule.entries[0].modeId = 1;
    cfg.iridiumModule.entries[0].period = 30;
    cfg.iridiumModule.entries[1].modeId = 2;
    cfg.iridiumModule.entries[1].period = 60;

    cfg.has_gsmMqttModule = true;
    strcpy(cfg.gsmMqttModule.clientId, "CLIENT_MAIN");
    strcpy(cfg.gsmMqttModule.broker, "mqtt.example.org");
    cfg.gsmMqttModule.port = 1883;
    cfg.gsmMqttModule.entries_count = 2;
    cfg.gsmMqttModule.entries[0].modeId = 1;
    cfg.gsmMqttModule.entries[0].period = 25;
    cfg.gsmMqttModule.entries[1].modeId = 2;
    cfg.gsmMqttModule.entries[1].period = 50;

    EXPECT_NO_THROW(repo.printNodeConfiguration(cfg));
}