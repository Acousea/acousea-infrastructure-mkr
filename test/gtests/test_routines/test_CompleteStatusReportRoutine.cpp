#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "common_test_resources.hpp"

#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "../common_test_resources.hpp"
#include "routines/CompleteStatusReportRoutine/CompleteStatusReportRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "MockGPS/MockGPS.h"
#include "MockBatteryController/MockBatteryController.h"
#include "MockRTCController/MockRTCController.h"
#include "ErrorHandler/ErrorHandler.h"

// =====================================================================
// Mock ModuleProxy para controlar el cache de módulos
// =====================================================================


class DummyRouter
{
}; // Router no usado en tests

class MockModuleProxy : public ModuleProxy
{
public:
    explicit MockModuleProxy() : ModuleProxy(reinterpret_cast<Router&>(dummyRouter))
    {
    }

    void injectHFModuleFresh(const acousea_ModuleWrapper& wrapper) {
        getCache().store(acousea_ModuleCode_ICLISTEN_HF, wrapper);    
    }

private:
    
    DummyRouter dummyRouter;
};


// =====================================================================
// Fixture
// =====================================================================
class CompleteStatusReportRoutineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
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

// --- Caso 1: Ejecución exitosa (todos los módulos frescos) ---
TEST_F(CompleteStatusReportRoutineTest, Execute_SuccessfulReportPacket)
{
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeValidNodeConfig());

    MockModuleProxy proxy;
    MockGPS gps(10.0f, 20.0f, 0.5f);
    MockBatteryController battery(80, acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING);
    MockRTCController rtc;

    // Añadir un módulo ICListenHF fresco
    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;
    strcpy(wrapper.module.icListenHF.serialNumber, "HF1234");

    proxy.injectHFModuleFresh(wrapper);
    EXPECT_TRUE(proxy.getCache().getIfFresh(acousea_ModuleCode_ICLISTEN_HF).has_value()) <<
        "ICListenHF module should be fresh in cache for this test.";

    proxy.injectHFModuleFresh(wrapper);
    CompleteStatusReportRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    ASSERT_TRUE(result.isSuccess()) << "Routine should produce success result. Error: " << result.getError();
    const auto& pkt = result.getValueConst();
    EXPECT_TRUE(pkt.has_routing);
    EXPECT_EQ(pkt.which_body, acousea_CommunicationPacket_report_tag);
    EXPECT_EQ(pkt.body.report.which_report, acousea_ReportBody_statusPayload_tag);

    const auto& payload = pkt.body.report.report.statusPayload;
    EXPECT_GE(payload.modules_count, 3);

    bool foundBattery = false, foundLocation = false, foundHF = false;
    for (size_t i = 0; i < payload.modules_count; ++i)
    {
        switch (payload.modules[i].key)
        {
        case acousea_ModuleCode_BATTERY_MODULE: foundBattery = true;
            break;
        case acousea_ModuleCode_LOCATION_MODULE: foundLocation = true;
            break;
        case acousea_ModuleCode_ICLISTEN_HF: foundHF = true;
            break;
        default: break;
        }
    }
    EXPECT_TRUE(foundBattery && foundLocation && foundHF);
}

// --- Caso 2: Pending si ICListenHF no está fresco ---
TEST_F(CompleteStatusReportRoutineTest, Execute_ReturnsPendingIfICListenNotFresh)
{
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeValidNodeConfig());

    MockModuleProxy proxy; // no setea stored (vacío)
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    CompleteStatusReportRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isPending());
    EXPECT_NE(std::string(result.getError()).find("ICListenHF"), std::string::npos);
}

// --- Caso 3: Falla por ausencia de operationModesModule ---
TEST_F(CompleteStatusReportRoutineTest, Execute_FailsWithoutOperationModesModule)
{
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);

    acousea_NodeConfiguration cfg = TestableNodeConfigurationRepository::makeDefault();
    cfg.has_operationModesModule = false;
    repo.saveConfiguration(cfg);

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    CompleteStatusReportRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("operation modes"), std::string::npos);
}

// --- Caso 4: Falla por ausencia de reportTypesModule ---
TEST_F(CompleteStatusReportRoutineTest, Execute_FailsWithoutReportTypesModule)
{
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);

    acousea_NodeConfiguration cfg = TestableNodeConfigurationRepository::makeDefault();
    cfg.has_reportTypesModule = false;
    repo.saveConfiguration(cfg);

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    CompleteStatusReportRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("report types"), std::string::npos);
}

// --- Caso 5: ReportType no contiene módulos soportados ---
TEST_F(CompleteStatusReportRoutineTest, Execute_IgnoresUnsupportedModulesGracefully)
{
    MockStorageManager storage;
    NodeConfigurationRepository repo(storage);

    // configuración válida pero con un módulo no soportado (STORAGE_MODULE)
    acousea_NodeConfiguration cfg = TestableNodeConfigurationRepository::makeValidNodeConfig();
    cfg.reportTypesModule.reportTypes[0].includedModules_count = 1;
    cfg.reportTypesModule.reportTypes[0].includedModules[0] = acousea_ModuleCode_STORAGE_MODULE;
    repo.saveConfiguration(cfg);

    MockModuleProxy proxy;
    MockGPS gps(0.0f, 0.0f, 0.1f);
    MockBatteryController battery;
    MockRTCController rtc;

    CompleteStatusReportRoutine routine(repo, proxy, &gps, &battery, &rtc);
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isSuccess());
    const auto& pkt = result.getValueConst();
    EXPECT_EQ(pkt.body.report.report.statusPayload.modules_count, 0);
}
