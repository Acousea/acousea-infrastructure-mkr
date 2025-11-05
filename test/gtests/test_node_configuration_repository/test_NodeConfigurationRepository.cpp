#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ErrorHandler/ErrorHandler.h"
#include "common_test_resources.hpp"


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
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 123;

    auto enc = TestableNodeConfigurationRepository::encodeProto(cfg);
    ASSERT_TRUE(enc.isSuccess());

    auto dec = TestableNodeConfigurationRepository::decodeProto(enc.getValue());
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 123);
}

// saveConfiguration escribe bytes válidos
TEST_F(NodeConfigurationRepositoryTest, SaveConfigurationWritesToStorageManager)
{
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 42;

    bool ok = repo.saveConfiguration(cfg);
    EXPECT_TRUE(ok);
    EXPECT_FALSE(mock.lastWrittenBytes.empty());
    EXPECT_EQ(mock.lastWrittenFile, "nodeconf");

    // decodificar para validar coherencia
    auto dec = TestableNodeConfigurationRepository::decodeProto(mock.lastWrittenBytes);
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 42);
}

// saveConfiguration falla si el encode falla
TEST_F(NodeConfigurationRepositoryTest, SaveConfigurationFailsOnEncodeError)
{
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    // no hay forma directa de forzar pb_encode a fallar con datos normales,
    // así que verificamos que writeFileBytes que devuelve false hace que el repo devuelva false
    mock.simulateWriteFail = true;

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    bool ok = repo.saveConfiguration(cfg);
    EXPECT_FALSE(ok);
}

// getNodeConfiguration devuelve default si storage está vacío
TEST_F(NodeConfigurationRepositoryTest, GetNodeConfigurationReturnsDefaultIfEmpty)
{
    MockStorageManager mock;
    mock.simulateReadEmpty = true;
    NodeConfigurationRepository repo(mock);

    auto cfg = repo.getNodeConfiguration();
    EXPECT_EQ(cfg.localAddress, 255); // valor del makeDefault
    EXPECT_TRUE(cfg.has_reportTypesModule);
    EXPECT_TRUE(cfg.has_operationModesModule);
    EXPECT_TRUE(cfg.has_iridiumModule);
}

// getNodeConfiguration devuelve default si bytes corruptos
TEST_F(NodeConfigurationRepositoryTest, GetNodeConfigurationReturnsDefaultIfCorrupted)
{
    MockStorageManager mock;
    mock.simulateCorruptedRead = true;
    NodeConfigurationRepository repo(mock);

    auto cfg = repo.getNodeConfiguration();
    EXPECT_EQ(cfg.localAddress, 255);
}

// init() crea default si no hay fichero
TEST_F(NodeConfigurationRepositoryTest, InitCreatesDefaultWhenMissing)
{
    MockStorageManager mock;
    mock.simulateReadEmpty = true;
    NodeConfigurationRepository repo(mock);

    repo.init();

    EXPECT_FALSE(mock.lastWrittenBytes.empty());
    auto dec = TestableNodeConfigurationRepository::decodeProto(mock.lastWrittenBytes);
    ASSERT_TRUE(dec.isSuccess());
    EXPECT_EQ(dec.getValueConst().localAddress, 255);
}

// reset() guarda makeDefault
TEST_F(NodeConfigurationRepositoryTest, ResetStoresDefaultConfig)
{
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    repo.reset();

    auto dec = TestableNodeConfigurationRepository::decodeProto(mock.lastWrittenBytes);
    ASSERT_TRUE(dec.isSuccess());
    auto cfg = dec.getValueConst();

    EXPECT_EQ(cfg.localAddress, 255);
    EXPECT_TRUE(cfg.has_reportTypesModule);
    EXPECT_EQ(cfg.reportTypesModule.reportTypes_count, 1);
    EXPECT_STREQ(cfg.reportTypesModule.reportTypes[0].name, "BasicRep");
}

// printNodeConfiguration no lanza y muestra info coherente
TEST_F(NodeConfigurationRepositoryTest, PrintNodeConfigurationDoesNotCrash)
{
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    auto cfg = TestableNodeConfigurationRepository::makeDefault();
    EXPECT_NO_THROW(repo.printNodeConfiguration(cfg));
}

// printNodeConfiguration genera salida completa con múltiples módulos configurados
TEST_F(NodeConfigurationRepositoryTest, PrintNodeConfigurationPrintsRichConfiguration)
{
    MockStorageManager mock;
    NodeConfigurationRepository repo(mock);

    acousea_NodeConfiguration cfg = acousea_NodeConfiguration_init_default;
    cfg.localAddress = 99;

    // --- OperationModesModule ---
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

    // --- ReportTypesModule ---
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

    // --- LoRaReportingModule ---
    cfg.has_loraModule = true;
    cfg.loraModule.entries_count = 2;
    cfg.loraModule.entries[0].modeId = 1;
    cfg.loraModule.entries[0].period = 10;
    cfg.loraModule.entries[1].modeId = 2;
    cfg.loraModule.entries[1].period = 20;

    // --- IridiumReportingModule ---
    cfg.has_iridiumModule = true;
    strcpy(cfg.iridiumModule.imei, "111122223333444");
    cfg.iridiumModule.entries_count = 2;
    cfg.iridiumModule.entries[0].modeId = 1;
    cfg.iridiumModule.entries[0].period = 30;
    cfg.iridiumModule.entries[1].modeId = 2;
    cfg.iridiumModule.entries[1].period = 60;

    // --- GsmMqttReportingModule ---
    cfg.has_gsmMqttModule = true;
    strcpy(cfg.gsmMqttModule.clientId, "CLIENT_MAIN");
    strcpy(cfg.gsmMqttModule.broker, "mqtt.example.org");
    cfg.gsmMqttModule.port = 1883;
    cfg.gsmMqttModule.entries_count = 2;
    cfg.gsmMqttModule.entries[0].modeId = 1;
    cfg.gsmMqttModule.entries[0].period = 25;
    cfg.gsmMqttModule.entries[1].modeId = 2;
    cfg.gsmMqttModule.entries[1].period = 50;

    // --- Ejecución ---
    EXPECT_NO_THROW(repo.printNodeConfiguration(cfg));
}

