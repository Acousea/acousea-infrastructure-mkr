#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "routines/RelayPacketRoutine/RelayPacketRoutine.hpp"
#include "../common_test_resources/DummyPort.hpp"
#include "ErrorHandler/ErrorHandler.h"

// =====================================================================
// Fixture común con Logger y DummyPorts
// =====================================================================
class RelayPacketRoutineTest : public ::testing::Test
{
protected:
    DummyPort serialPort{IPort::PortType::SerialPort};
    DummyPort gsmPort{IPort::PortType::GsmMqttPort};
    DummyPort loraPort{IPort::PortType::LoraPort};
    Router router{std::vector<IPort *>{&serialPort, &gsmPort, &loraPort}};
    ConsoleDisplay display;

    void SetUp() override
    {
        ErrorHandler::setHandler([](const char *msg)
                                 { fprintf(stderr, "[TEST_ERROR_HANDLER] %s\n", msg); });

        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);

        serialPort.sentPackets.clear();
        gsmPort.sentPackets.clear();
        loraPort.sentPackets.clear();
    }
};

// =====================================================================
// TESTS
// =====================================================================

// Caso 1: Reenvía correctamente un paquete válido a los puertos configurados
TEST_F(RelayPacketRoutineTest, RelaysValidPacketThroughAllConfiguredPorts)
{
    // Paquete válido con routing completo
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing.sender = 42;
    pkt.routing.receiver = 0;
    pkt.which_body = acousea_CommunicationPacket_report_tag;

    RelayPacketRoutine routine(router, {
        IPort::PortType::SerialPort,
        IPort::PortType::GsmMqttPort,
        IPort::PortType::LoraPort
    });

    auto result = routine.execute(pkt);

    EXPECT_TRUE(result.isSuccess()) << "Expected success but got: " << result.getError();

    // Cada DummyPort debe haber enviado algo
    EXPECT_GT(serialPort.sentPackets.size(), 0) << "SerialPort should have sent packet";
    EXPECT_GT(gsmPort.sentPackets.size(), 0) << "GsmPort should have sent packet";
    EXPECT_GT(loraPort.sentPackets.size(), 0) << "LoraPort should have sent packet";
}

// Caso 2: No reenvía si el paquete no tiene routing
TEST_F(RelayPacketRoutineTest, FailsIfPacketHasNoRouting)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = false; // sin routing

    RelayPacketRoutine routine(router, { IPort::PortType::SerialPort });

    auto result = routine.execute(pkt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("routing"), std::string::npos)
        << "Expected routing error message";
    EXPECT_EQ(serialPort.sentPackets.size(), 0);
}

// Caso 3: No reenvía nada si los puertos configurados no existen en el Router
TEST_F(RelayPacketRoutineTest, FailsIfPortsNotFoundInRouter)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing.sender = 7;
    pkt.routing.receiver = 0;
    pkt.which_body = acousea_CommunicationPacket_report_tag;

    // Router no contiene PortType::SBDPort
    RelayPacketRoutine routine(router, { IPort::PortType::SBDPort });

    auto result = routine.execute(pkt);

    EXPECT_TRUE(result.isError()) << "Expected failure because SBDPort not present in router";
    EXPECT_EQ(serialPort.sentPackets.size(), 0);
    EXPECT_EQ(gsmPort.sentPackets.size(), 0);
    EXPECT_EQ(loraPort.sentPackets.size(), 0);
}

// Caso 4: Falla si no se pasa ningún paquete
TEST_F(RelayPacketRoutineTest, FailsIfNoPacketProvided)
{
    RelayPacketRoutine routine(router, { IPort::PortType::SerialPort });
    auto result = routine.execute(std::nullopt);

    EXPECT_TRUE(result.isError());
    EXPECT_NE(std::string(result.getError()).find("No packet"), std::string::npos);
}

// Caso 5: Reenvía correctamente a un único puerto configurado
TEST_F(RelayPacketRoutineTest, RelaysPacketToSinglePort)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing.sender = 1;
    pkt.routing.receiver = 0;
    pkt.which_body = acousea_CommunicationPacket_report_tag;

    RelayPacketRoutine routine(router, { IPort::PortType::GsmMqttPort });

    auto result = routine.execute(pkt);

    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(gsmPort.sentPackets.size(), 1);
    EXPECT_EQ(serialPort.sentPackets.size(), 0);
    EXPECT_EQ(loraPort.sentPackets.size(), 0);
}
