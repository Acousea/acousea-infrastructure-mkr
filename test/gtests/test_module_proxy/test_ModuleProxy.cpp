#include <gtest/gtest.h>
#include "ModuleProxy/ModuleProxy.hpp"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>
#include <Ports/Iridium/MockIridiumPort.h>
#include <Ports/LoRa/MockLoRaPort.h>
#include <Ports/Serial/MockSerialPort.h>


#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#ifdef UNIT_TESTING
#define DBG(msg) std::cerr << "[DBG] " << msg << std::endl
#else
#define DBG(msg)
#endif


class ModuleProxyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Evitar aborts durante los tests
        ErrorHandler::setHandler([](const std::string& msg)
        {
            fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg.c_str());
        });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};



// =====================================================
// Tests de la caché
// =====================================================

TEST_F(ModuleProxyTest, CacheStoresAndRetrieves)
{
    MockSerialPort serialPort;
    MockLoRaPort loRaPort;
    MockIridiumPort iridiumPort;
    Router router({&serialPort, &loRaPort, &iridiumPort});
    ModuleProxy proxy(reinterpret_cast<Router&>(router));

    // Crear un wrapper simulado (ICListenHF)
    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;
    strcpy(wrapper.module.icListenHF.serialNumber, "TEST123");

    // Guardar y recuperar
    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_HF, wrapper);

    auto retrieved = proxy.getCache().get(acousea_ModuleCode_ICLISTEN_HF);
    EXPECT_TRUE(retrieved.valid());
    EXPECT_TRUE(retrieved.fresh());
    EXPECT_STREQ(retrieved.get().module.icListenHF.serialNumber, "TEST123");
}

TEST_F(ModuleProxyTest, InvalidateRemovesFreshness)
{
    MockSerialPort serialPort;
    MockLoRaPort loRaPort;
    MockIridiumPort iridiumPort;
    Router router({&serialPort, &loRaPort, &iridiumPort});
    ModuleProxy proxy(reinterpret_cast<Router&>(router));

    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;

    proxy.getCache().store(acousea_ModuleCode_ICLISTEN_HF, wrapper);
    proxy.getCache().invalidate(acousea_ModuleCode_ICLISTEN_HF);

    EXPECT_FALSE(proxy.getCache().fresh(acousea_ModuleCode_ICLISTEN_HF));
}

// =====================================================
// Test del envío (buildSetPacket + sendModule)
// =====================================================

TEST_F(ModuleProxyTest, SendModuleBuildsValidPacket)
{
    MockSerialPort serialPort;
    MockLoRaPort loRaPort;
    MockIridiumPort iridiumPort;
    Router router({&serialPort, &loRaPort, &iridiumPort});
    ModuleProxy proxy(reinterpret_cast<Router&>(router));

    // Crear un wrapper ICListenHF simulado
    acousea_ModuleWrapper wrapper = acousea_ModuleWrapper_init_default;
    wrapper.which_module = acousea_ModuleWrapper_icListenHF_tag;
    wrapper.module.icListenHF = acousea_ICListenHF_init_default;
    strcpy(wrapper.module.icListenHF.serialNumber, "SN123456");
    DBG("Created wrapper with serialNumber: " << wrapper.module.icListenHF.serialNumber);

    // Simular el envío
    bool result = proxy.sendModule(acousea_ModuleCode_ICLISTEN_HF, wrapper, ModuleProxy::DeviceAlias::ICListen);

    EXPECT_TRUE(result);

    // Verificar indirectamente que el wrapper fue empaquetado correctamente
    acousea_CommunicationPacket pkt = ModuleProxy::buildSetPacket(acousea_ModuleCode_ICLISTEN_HF, wrapper);

    DBG("Built packet for sending ICListenHF module");


    EXPECT_EQ(pkt.which_body, acousea_CommunicationPacket_command_tag);
    EXPECT_EQ(pkt.body.command.which_command, acousea_CommandBody_setConfiguration_tag);
    EXPECT_EQ(pkt.body.command.command.setConfiguration.modules_count, 1);
    EXPECT_TRUE(pkt.body.command.command.setConfiguration.modules[0].has_value);
    EXPECT_EQ(pkt.body.command.command.setConfiguration.modules[0].key, acousea_ModuleCode_ICLISTEN_HF);

    // Validar que el wrapper dentro del paquete es el mismo
    const acousea_ModuleWrapper& builtWrapper = pkt.body.command.command.setConfiguration.modules[0].value;

    DBG("Verifying built wrapper with serialNumber: " << builtWrapper.module.icListenHF.serialNumber);

    EXPECT_EQ(builtWrapper.which_module, acousea_ModuleWrapper_icListenHF_tag);
    EXPECT_STREQ(builtWrapper.module.icListenHF.serialNumber, "SN123456");
}
