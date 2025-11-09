#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif


#include <gtest/gtest.h>
#include "ErrorHandler/ErrorHandler.h"
#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "routines/StoreNodeConfigurationRoutine/StoreNodeConfigurationRoutine.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "ModuleProxy/ModuleProxy.hpp"
#include "Router.h"

#include "bindings/nodeDevice.pb.h"

#include "../common_test_resources/InMemoryStorageManager.hpp"
#include "../common_test_resources/TestableNodeConfigurationRepository.hpp"
#include "../common_test_resources/PacketUtils.hpp"
#include "../common_test_resources/DummyPort.hpp"


// =====================================================================
// Fixture común
// =====================================================================
class StoreNodeConfigurationRoutineTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        ErrorHandler::setHandler([](const char* msg) {
            fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg);
        });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};




// =====================================================================
// TESTS
// =====================================================================

// Caso 1: Falla si no se pasa ningún paquete
TEST_F(StoreNodeConfigurationRoutineTest, FailsIfNoPacketProvided)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    auto result = routine.execute(std::nullopt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("No packet provided"), std::string::npos);
}


// Caso 2: Falla si el paquete no es de tipo response
TEST_F(StoreNodeConfigurationRoutineTest, FailsIfPacketNotResponse)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag; // incorrecto

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("not a response"), std::string::npos);
}


// Caso 3: Falla si el tipo de response no contiene módulos
TEST_F(StoreNodeConfigurationRoutineTest, FailsIfResponseTypeInvalid)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    // auto pkt = PacketUtils::makeBaseResponsePacket(acousea_ResponseBody_statusPayload_tag); // tipo no válido
    auto pkt = PacketUtils::makeBaseResponsePacket(255); // tipo no válido

    auto result = routine.execute(pkt);
    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("does not contain configuration modules"), std::string::npos);
}


// Caso 4: Almacena correctamente módulos desde setConfiguration
TEST_F(StoreNodeConfigurationRoutineTest, StoresModulesFromSetConfiguration)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseResponsePacket(acousea_ResponseBody_setConfiguration_tag);
    pkt.body.response.response.setConfiguration.modules_count = 1;
    auto& entry = pkt.body.response.response.setConfiguration.modules[0];
    entry.key = acousea_ModuleCode_BATTERY_MODULE;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_battery_tag;
    entry.value.module.battery.batteryPercentage = 77;

    auto result = routine.execute(pkt);
    ASSERT_TRUE(result.isSuccess());

    // Verificar que se almacenó en cache
    auto stored = proxy.getCache().get(acousea_ModuleCode_BATTERY_MODULE);
    EXPECT_TRUE(stored.valid());
    EXPECT_EQ(stored.get().which_module, acousea_ModuleWrapper_battery_tag);
    EXPECT_EQ(stored.get().module.battery.batteryPercentage, 77);
}


// Caso 5: Almacena correctamente módulos desde updatedConfiguration
TEST_F(StoreNodeConfigurationRoutineTest, StoresModulesFromUpdatedConfiguration)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);
    repo.saveConfiguration(TestableNodeConfigurationRepository::makeDefault());

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseResponsePacket(acousea_ResponseBody_updatedConfiguration_tag);
    pkt.body.response.response.updatedConfiguration.modules_count = 1;
    auto& entry = pkt.body.response.response.updatedConfiguration.modules[0];
    entry.key = acousea_ModuleCode_RTC_MODULE;
    entry.has_value = true;
    entry.value.which_module = acousea_ModuleWrapper_rtc_tag;
    entry.value.module.rtc.epochSeconds = 1700000000;

    auto result = routine.execute(pkt);
    ASSERT_TRUE(result.isSuccess());

    auto stored = proxy.getCache().get(acousea_ModuleCode_RTC_MODULE);
    EXPECT_TRUE(stored.valid());
    EXPECT_EQ(stored.get().which_module, acousea_ModuleWrapper_rtc_tag);
    EXPECT_EQ(stored.get().module.rtc.epochSeconds, 1700000000);
}


// Caso 6: Ignora módulos sin valor
TEST_F(StoreNodeConfigurationRoutineTest, FailsIfModuleWithoutValue)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    StoreNodeConfigurationRoutine routine(repo, proxy);

    auto pkt = PacketUtils::makeBaseResponsePacket(acousea_ResponseBody_updatedConfiguration_tag);
    pkt.body.response.response.updatedConfiguration.modules_count = 1;
    auto& entry = pkt.body.response.response.updatedConfiguration.modules[0];
    entry.key = acousea_ModuleCode_LOCATION_MODULE;
    entry.has_value = false; // sin valor

    auto result = routine.execute(pkt);
    ASSERT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("has no value"), std::string::npos);
    EXPECT_FALSE(proxy.getCache().get(acousea_ModuleCode_LOCATION_MODULE).valid());
}


// Caso 7: Si un módulo falla, la caché no se modifica (rollback implícito)
TEST_F(StoreNodeConfigurationRoutineTest, DiscardsChangesOnError)
{
    InMemoryStorageManager storage;
    NodeConfigurationRepository repo(storage);

    DummyPort port(IPort::PortType::SerialPort);
    std::vector<IPort*> ports{&port};
    Router router(ports);
    ModuleProxy proxy(router);

    // Simulamos que ya hay un módulo válido en caché
    acousea_ModuleWrapper initialWrapper = acousea_ModuleWrapper_init_default;
    initialWrapper.which_module = acousea_ModuleWrapper_battery_tag;
    initialWrapper.module.battery.batteryPercentage = 50;
    proxy.getCache().store(acousea_ModuleCode_BATTERY_MODULE, initialWrapper);

    // Creamos la rutina
    StoreNodeConfigurationRoutine routine(repo, proxy);

    // Paquete con dos módulos: el primero válido, el segundo sin valor -> provocará error
    auto pkt = PacketUtils::makeBaseResponsePacket(acousea_ResponseBody_setConfiguration_tag);
    pkt.body.response.response.setConfiguration.modules_count = 2;

    auto& validEntry = pkt.body.response.response.setConfiguration.modules[0];
    validEntry.key = acousea_ModuleCode_RTC_MODULE;
    validEntry.has_value = true;
    validEntry.value.which_module = acousea_ModuleWrapper_rtc_tag;
    validEntry.value.module.rtc.epochSeconds = 123456789;

    auto& invalidEntry = pkt.body.response.response.setConfiguration.modules[1];
    invalidEntry.key = acousea_ModuleCode_LOCATION_MODULE;
    invalidEntry.has_value = false; // fuerza error

    // Ejecutar
    auto result = routine.execute(pkt);

    // Debe fallar
    ASSERT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("has no value"), std::string::npos);

    // === Verificar que la caché no cambió ===
    auto stillStored = proxy.getCache().get(acousea_ModuleCode_BATTERY_MODULE);
    EXPECT_TRUE(stillStored.valid());
    EXPECT_EQ(stillStored.get().which_module, acousea_ModuleWrapper_battery_tag);
    EXPECT_EQ(stillStored.get().module.battery.batteryPercentage, 50);

    // Y que no se agregó el nuevo módulo fallido
    auto newStored = proxy.getCache().get(acousea_ModuleCode_RTC_MODULE);
    EXPECT_FALSE(newStored.valid());
}
