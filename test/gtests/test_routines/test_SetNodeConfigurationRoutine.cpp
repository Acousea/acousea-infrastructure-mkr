#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <vector>
#include <gtest/gtest.h>
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "routines/SetNodeConfigurationRoutine/SetNodeConfigurationRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "MockGPS/MockGPS.h"
#include "MockBatteryController/MockBatteryController.h"
#include "MockRTCController/MockRTCController.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "Router.h"

#include "bindings/nodeDevice.pb.h"

#include "../common_test_resources/InMemoryStorageManager.hpp"
#include "../common_test_resources/TestableNodeConfigurationRepository.hpp"
#include "../common_test_resources/PacketUtils.hpp"
#include "../common_test_resources/DummyPort.hpp"

// =====================================================================
// Fixture común para SetNodeConfigurationRoutine
// =====================================================================
class SetNodeConfigurationRoutineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ErrorHandler::setHandler([](const char *msg)
                                 { fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg); });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};

// =====================================================================
// TESTS
// =====================================================================

// Caso 1: Devuelve error si no se pasa ningún paquete
TEST_F(SetNodeConfigurationRoutineTest, FailsIfNoPacketProvided)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto result = routine.execute(std::nullopt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("No packet provided"), std::string::npos);
}

// Caso 2: Falla si el paquete no es de tipo command
TEST_F(SetNodeConfigurationRoutineTest, FailsIfPacketNotCommandType)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_response_tag; // no es command

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("not of type command"), std::string::npos);
}

// Caso 3: Falla si el comando no es setConfiguration
TEST_F(SetNodeConfigurationRoutineTest, FailsIfCommandNotSetConfiguration)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("setNodeConfiguration"), std::string::npos);
}

// Caso 4: Falla si contiene un módulo inválido
TEST_F(SetNodeConfigurationRoutineTest, FailsIfInvalidModuleCode)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count  = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = 255; // inválido
    entry.has_value = true;

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("Invalid TagType in SetNodeConfigurationPayload"), std::string::npos);
}

// Caso 5: Configura correctamente OperationModesModule
TEST_F(SetNodeConfigurationRoutineTest, SetsOperationModesSuccessfully)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_OPERATION_MODES_MODULE;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_operationModes_tag;
    entry.value.module.operationModes.modes_count = 1;
    strcpy(entry.value.module.operationModes.modes[0].name, "TESTMODE");

    auto result = routine.execute(pkt);

    ASSERT_TRUE(result.isSuccess());
    const auto &resp = result.getValueConst();
    EXPECT_EQ(resp.which_body, acousea_CommunicationPacket_response_tag);
    EXPECT_EQ(resp.body.response.which_response, acousea_ResponseBody_setConfiguration_tag);
}

// Caso 6: Retorna pending si ICListen config no está fresca
TEST_F(SetNodeConfigurationRoutineTest, ReturnsPendingIfICListenNotFresh)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    // Crear módulo ICListen
    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_ICLISTEN_HF;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;

    // La cache está vacía -> no fresca
    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isPending());
    EXPECT_NE(std::string(result.getError()).find("not fresh"), std::string::npos);
}

// Caso 7: Devuelve éxito si ICListen config está fresca
TEST_F(SetNodeConfigurationRoutineTest, SuccessIfICListenConfigFresh)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    // Simular cache fresca
    acousea_ModuleWrapper dummy = acousea_ModuleWrapper_init_default;
    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_LOGGING_CONFIG, dummy);
    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_STREAMING_CONFIG, dummy);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_ICLISTEN_HF;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_icListenHF_tag;

    auto result = routine.execute(pkt);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.getValueConst().which_body, acousea_CommunicationPacket_response_tag);
}

// Caso 8: Configura correctamente módulo de Reporting (LORA)
TEST_F(SetNodeConfigurationRoutineTest, SetsReportingModuleLoraSuccessfully)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_LORA_REPORTING_MODULE;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_loraReporting_tag;

    // Configurar correctamente el módulo LoRa según su definición
    entry.value.module.loraReporting.entries_count = 1;
    entry.value.module.loraReporting.entries[0].modeId = 1;
    entry.value.module.loraReporting.entries[0].period = 30;

    auto result = routine.execute(pkt);
    ASSERT_TRUE(result.isSuccess());
    const auto &resp = result.getValueConst();
    EXPECT_EQ(resp.which_body, acousea_CommunicationPacket_response_tag);
    EXPECT_EQ(resp.body.response.which_response, acousea_ResponseBody_setConfiguration_tag);
}


// Caso 9: Falla si un módulo no tiene valor
TEST_F(SetNodeConfigurationRoutineTest, FailsIfModuleHasNoValue)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort *> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    SetNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseSetConfigPacket();
    pkt.body.command.command.setConfiguration.modules_count  = 1;
    auto &entry = pkt.body.command.command.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_OPERATION_MODES_MODULE;
    entry.has_value = false; // sin valor

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("has no value"), std::string::npos);
}