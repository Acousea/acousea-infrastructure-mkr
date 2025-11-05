#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>


#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "Router.h"

#include "../common_test_resources/DummyPort.hpp"
#include "../common_test_resources/PacketUtils.hpp"


// ======================================================================
// Fixture con logger
// ======================================================================
class RouterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        Logger::initialize(&display, nullptr, nullptr, "LOG.TXT", Logger::Mode::SerialOnly);
    }

    ConsoleDisplay display;
};

// ======================================================================
// TESTS
// ======================================================================
TEST_F(RouterTest, SendThroughSpecificPortWithSenderSetsRoutingSender)
{
    DummyPort serial(IPort::PortType::SerialPort);
    DummyPort lora(IPort::PortType::LoraPort);
    Router router({&serial, &lora});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    const uint8_t sender = 42;
    bool ok = router.from(sender).through(IPort::PortType::LoraPort).send(pkt);
    ASSERT_TRUE(ok);
    ASSERT_EQ(lora.sentPackets.size(), 1u);
    ASSERT_TRUE(serial.sentPackets.empty());

    auto sent = PacketUtils::decodePacketTest(lora.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, sender);
}

TEST_F(RouterTest, BroadcastUsesBroadcastSender)
{
    DummyPort serial(IPort::PortType::SerialPort);
    Router router({&serial});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    bool ok = router.broadcast().through(IPort::PortType::SerialPort).send(pkt);
    ASSERT_TRUE(ok);
    ASSERT_EQ(serial.sentPackets.size(), 1u);

    auto sent = PacketUtils::decodePacketTest(serial.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, Router::broadcastAddress);
}

TEST_F(RouterTest, ReadPortsGroupsByPortTypeAndFiltersReceiver)
{
    DummyPort serial(IPort::PortType::SerialPort);
    DummyPort lora(IPort::PortType::LoraPort);
    Router router({&serial, &lora});

    auto pktLocal = PacketUtils::makeRoutedPacket(10, 7);
    serial.enqueueRaw(PacketUtils::encodePacketTest(pktLocal));

    auto pktOther = PacketUtils::makeRoutedPacket(11, 99);
    lora.enqueueRaw(PacketUtils::encodePacketTest(pktOther));

    auto pktBcast = PacketUtils::makeRoutedPacket(12, Router::broadcastAddress);
    lora.enqueueRaw(PacketUtils::encodePacketTest(pktBcast));

    auto result = router.readPorts(7);

    ASSERT_EQ(result.count(IPort::PortType::SerialPort), 1u);
    ASSERT_EQ(result.count(IPort::PortType::LoraPort), 1u);

    EXPECT_EQ(result[IPort::PortType::SerialPort].size(), 1u);
    EXPECT_EQ(result[IPort::PortType::LoraPort].size(), 1u);

    const auto& sPkt = result[IPort::PortType::SerialPort].front();
    EXPECT_EQ(sPkt.routing.receiver, 7);

    const auto& lPkt = result[IPort::PortType::LoraPort].front();
    EXPECT_EQ(lPkt.routing.receiver, Router::broadcastAddress);
}

TEST_F(RouterTest, ReadPortsSkipsPacketsWithoutRouting)
{
    DummyPort serial(IPort::PortType::SerialPort);
    Router router({&serial});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    serial.enqueueRaw(PacketUtils::encodePacketTest(pkt));

    auto result = router.readPorts(5);
    EXPECT_TRUE(result.empty());
}

TEST_F(RouterTest, ReadPortsSkipsPacketsThatFailToDecode)
{
    DummyPort serial(IPort::PortType::SerialPort);
    Router router({&serial});
    serial.enqueueRaw(std::vector<uint8_t>{0xFF, 0x00, 0xAA});
    auto result = router.readPorts(1);
    EXPECT_TRUE(result.empty());
}

TEST_F(RouterTest, SendFailsWhenPortTypeNotPresent)
{
    DummyPort serial(IPort::PortType::SerialPort);
    Router router({&serial});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    bool ok = router.from(9).through(IPort::PortType::LoraPort).send(pkt);
    EXPECT_FALSE(ok);
    EXPECT_TRUE(serial.sentPackets.empty());
}

TEST_F(RouterTest, SendAlwaysSetsHasRoutingTrue)
{
    DummyPort sbd(IPort::PortType::SBDPort);
    Router router({&sbd});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    ASSERT_TRUE(router.from(3).through(IPort::PortType::SBDPort).send(pkt));
    ASSERT_EQ(sbd.sentPackets.size(), 1u);

    auto sent = PacketUtils::decodePacketTest(sbd.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, 3);
}
