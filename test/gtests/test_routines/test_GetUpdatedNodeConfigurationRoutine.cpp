#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "routines/GetUpdatedNodeConfigurationRoutine/GetUpdatedNodeConfigurationRoutine.hpp"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ErrorHandler/ErrorHandler.h"
#include "MockGPS/MockGPS.h"
#include "MockBatteryController/MockBatteryController.h"
#include "MockRTCController/MockRTCController.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "common_test_resources.hpp"

// =====================================================================
// Mock Router & Proxy
// =====================================================================
class DummyRouter {};

class MockModuleProxy : public ModuleProxy {
public:
    explicit MockModuleProxy() : ModuleProxy(reinterpret_cast<Router&>(dummyRouter)) {}

private:
    DummyRouter dummyRouter;
};

// =====================================================================
// Fixture común con Logger y ErrorHandler seguro
// =====================================================================
class GetUpdatedNodeConfigurationRoutineTest : public ::testing::Test {
protected:
    void SetUp() override {
        ErrorHandler::setHandler([](const std::string& msg) {
            fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg.c_str());
        });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};

// =====================================================================
// TESTS
// =====================================================================

// Caso 1: Devuelve con éxito la configuración actual codificada en UpdatedNodeConfigurationPayload
TEST_F(GetUpdatedNodeConfigurationRoutineTest, ExecuteReturnsUpdatedConfigurationPacket) {
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    MockModuleProxy proxy;
    MockGPS gps(10.0f, 20.0f, 0.5f);
    MockBatteryController battery(80, acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING);
    MockRTCController rtc;

    // Crear un paquete de solicitud válido
    acousea_CommunicationPacket req = acousea_CommunicationPacket_init_default;
    req.has_routing = true;
    req.routing.sender = 1;
    req.routing.receiver = 255;
    req.which_body = acousea_CommunicationPacket_command_tag;
    req.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;
    req.body.command.command.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    req.body.command.command.requestedConfiguration.requestedModules_count = 1;
    req.body.command.command.requestedConfiguration.requestedModules[0] = acousea_ModuleCode_BATTERY_MODULE;

    GetUpdatedNodeConfigurationRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(req);

    ASSERT_TRUE(result.isSuccess()) << "Expected success but got: " << result.getError();
    const auto& pkt = result.getValueConst();
    EXPECT_TRUE(pkt.has_routing);
    EXPECT_EQ(pkt.which_body, acousea_CommunicationPacket_response_tag);

    const auto& resp = pkt.body.response;
    EXPECT_EQ(resp.which_response, acousea_ResponseBody_updatedConfiguration_tag) <<
        "Expected updatedConfiguration response type = " <<  acousea_ResponseBody_updatedConfiguration_tag << ", but got " << resp.which_response;
    const auto& updated = resp.response.updatedConfiguration;

    EXPECT_GT(updated.modules_count, 0) << "Expected at least one module in updated configuration";
    EXPECT_EQ(updated.modules[0].key, acousea_ModuleCode_BATTERY_MODULE) <<
        "Expected first module key to be BATTERY_MODULE, but got " << updated.modules[0].key;
}

// Caso 2: Devuelve pending si se solicita un módulo no fresco
TEST_F(GetUpdatedNodeConfigurationRoutineTest, ReturnsPendingIfICListenNotFresh) {
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    // Paquete que solicita ICListenHF (no fresco)
    acousea_CommunicationPacket req = acousea_CommunicationPacket_init_default;
    req.has_routing = true;
    req.which_body = acousea_CommunicationPacket_command_tag;
    req.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;
    req.body.command.command.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    req.body.command.command.requestedConfiguration.requestedModules_count = 1;
    req.body.command.command.requestedConfiguration.requestedModules[0] = acousea_ModuleCode_ICLISTEN_HF;

    GetUpdatedNodeConfigurationRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(req);

    EXPECT_TRUE(result.isPending());
    EXPECT_NE(result.getError().find("ICListen"), std::string::npos);
}

// Caso 3: Falla si el paquete no tiene el body esperado
TEST_F(GetUpdatedNodeConfigurationRoutineTest, FailsIfPacketNotCommandType) {
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    acousea_CommunicationPacket invalidPkt = acousea_CommunicationPacket_init_default;
    invalidPkt.which_body = acousea_CommunicationPacket_report_tag; // no es command

    GetUpdatedNodeConfigurationRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(invalidPkt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(result.getError().find("not of type command"), std::string::npos);
}

// Caso 4: Falla si no se pasa ningún paquete
TEST_F(GetUpdatedNodeConfigurationRoutineTest, FailsIfNoPacketProvided) {
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.4f);
    MockBatteryController battery;
    MockRTCController rtc;

    GetUpdatedNodeConfigurationRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(result.getError().find("No packet provided"), std::string::npos);
}

// Caso 5: Devuelve correctamente la hora del RTC
TEST_F(GetUpdatedNodeConfigurationRoutineTest, ReturnsRTCModuleCorrectly) {
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.3f);
    MockBatteryController battery;
    MockRTCController rtc;
    rtc.setEpoch(1700000000);

    // Solicitar RTC
    acousea_CommunicationPacket req = acousea_CommunicationPacket_init_default;
    req.has_routing = true;
    req.which_body = acousea_CommunicationPacket_command_tag;
    req.body.command.which_command = acousea_CommandBody_requestedConfiguration_tag;
    req.body.command.command.requestedConfiguration = acousea_GetUpdatedNodeConfigurationPayload_init_default;
    req.body.command.command.requestedConfiguration.requestedModules_count = 1;
    req.body.command.command.requestedConfiguration.requestedModules[0] = acousea_ModuleCode_RTC_MODULE;

    GetUpdatedNodeConfigurationRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(req);

    ASSERT_TRUE(result.isSuccess());
    const auto& pkt = result.getValueConst();
    const auto& updated = pkt.body.response.response.updatedConfiguration;
    ASSERT_GE(updated.modules_count, 1);
    EXPECT_EQ(updated.modules[0].key, acousea_ModuleCode_RTC_MODULE);
}
