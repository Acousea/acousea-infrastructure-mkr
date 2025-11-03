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
