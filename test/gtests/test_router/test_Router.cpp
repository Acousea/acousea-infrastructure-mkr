#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include <pb_encode.h>
#include <pb_decode.h>

#include "Logger/Logger.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>

#include "Router.h"
#include "bindings/nodeDevice.pb.h"

// ======================================================================
// Mock simple de puerto para pruebas
// ======================================================================
class TestPort : public IPort
{
public:
    explicit TestPort(PortType t) : IPort(t) {}

    void init() override {}
    bool send(const std::vector<uint8_t> &data) override
    {
        sentPackets.push_back(data);
        return sendReturn;
    }
    bool available() override { return !inQueue.empty(); }
    std::vector<std::vector<uint8_t>> read() override
    {
        auto tmp = inQueue;
        inQueue.clear();
        return tmp;
    }

    void enqueueRaw(const std::vector<uint8_t> &raw) { inQueue.push_back(raw); }
    void setSendReturn(bool val) { sendReturn = val; }

    std::vector<std::vector<uint8_t>> sentPackets;

private:
    std::vector<std::vector<uint8_t>> inQueue;
    bool sendReturn{true};
};

// ======================================================================
// Helpers nanopb
// ======================================================================
static std::vector<uint8_t> encodePacketTest(const acousea_CommunicationPacket &pkt)
{
    pb_ostream_t sizing = PB_OSTREAM_SIZING;
    if (!pb_encode(&sizing, acousea_CommunicationPacket_fields, &pkt))
        throw std::runtime_error(std::string("encodePacketTest sizing failed: ") + PB_GET_ERROR(&sizing));

    std::vector<uint8_t> buf(sizing.bytes_written);
    pb_ostream_t os = pb_ostream_from_buffer(buf.data(), buf.size());
    if (!pb_encode(&os, acousea_CommunicationPacket_fields, &pkt))
        throw std::runtime_error(std::string("encodePacketTest write failed: ") + PB_GET_ERROR(&os));

    return buf;
}

static acousea_CommunicationPacket decodePacketTest(const std::vector<uint8_t> &raw)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pb_istream_t is = pb_istream_from_buffer(raw.data(), raw.size());
    if (!pb_decode(&is, acousea_CommunicationPacket_fields, &pkt))
        throw std::runtime_error(std::string("decodePacketTest failed: ") + PB_GET_ERROR(&is));
    return pkt;
}

static acousea_CommunicationPacket makeRoutedPacket(uint8_t sender, uint8_t receiver)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing = acousea_RoutingChunk_init_default;
    pkt.routing.sender = sender;
    pkt.routing.receiver = receiver;

    // Asignamos un body "command.setConfiguration" vacío válido
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    return pkt;
}

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
    TestPort serial(IPort::PortType::SerialPort);
    TestPort lora(IPort::PortType::LoraPort);
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

    auto sent = decodePacketTest(lora.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, sender);
}

TEST_F(RouterTest, BroadcastUsesBroadcastSender)
{
    TestPort serial(IPort::PortType::SerialPort);
    Router router({&serial});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    bool ok = router.broadcast().through(IPort::PortType::SerialPort).send(pkt);
    ASSERT_TRUE(ok);
    ASSERT_EQ(serial.sentPackets.size(), 1u);

    auto sent = decodePacketTest(serial.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, Router::broadcastAddress);
}

TEST_F(RouterTest, ReadPortsGroupsByPortTypeAndFiltersReceiver)
{
    TestPort serial(IPort::PortType::SerialPort);
    TestPort lora(IPort::PortType::LoraPort);
    Router router({&serial, &lora});

    auto pktLocal = makeRoutedPacket(10, 7);
    serial.enqueueRaw(encodePacketTest(pktLocal));

    auto pktOther = makeRoutedPacket(11, 99);
    lora.enqueueRaw(encodePacketTest(pktOther));

    auto pktBcast = makeRoutedPacket(12, Router::broadcastAddress);
    lora.enqueueRaw(encodePacketTest(pktBcast));

    auto result = router.readPorts(7);

    ASSERT_EQ(result.count(IPort::PortType::SerialPort), 1u);
    ASSERT_EQ(result.count(IPort::PortType::LoraPort), 1u);

    EXPECT_EQ(result[IPort::PortType::SerialPort].size(), 1u);
    EXPECT_EQ(result[IPort::PortType::LoraPort].size(), 1u);

    const auto &sPkt = result[IPort::PortType::SerialPort].front();
    EXPECT_EQ(sPkt.routing.receiver, 7);

    const auto &lPkt = result[IPort::PortType::LoraPort].front();
    EXPECT_EQ(lPkt.routing.receiver, Router::broadcastAddress);
}

TEST_F(RouterTest, ReadPortsSkipsPacketsWithoutRouting)
{
    TestPort serial(IPort::PortType::SerialPort);
    Router router({&serial});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    serial.enqueueRaw(encodePacketTest(pkt));

    auto result = router.readPorts(5);
    EXPECT_TRUE(result.empty());
}

TEST_F(RouterTest, ReadPortsSkipsPacketsThatFailToDecode)
{
    TestPort serial(IPort::PortType::SerialPort);
    Router router({&serial});
    serial.enqueueRaw(std::vector<uint8_t>{0xFF, 0x00, 0xAA});
    auto result = router.readPorts(1);
    EXPECT_TRUE(result.empty());
}

TEST_F(RouterTest, SendFailsWhenPortTypeNotPresent)
{
    TestPort serial(IPort::PortType::SerialPort);
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
    TestPort sbd(IPort::PortType::SBDPort);
    Router router({&sbd});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command = acousea_CommandBody_init_default;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;
    pkt.body.command.command.setConfiguration = acousea_SetNodeConfigurationPayload_init_default;

    ASSERT_TRUE(router.from(3).through(IPort::PortType::SBDPort).send(pkt));
    ASSERT_EQ(sbd.sentPackets.size(), 1u);

    auto sent = decodePacketTest(sbd.sentPackets[0]);
    EXPECT_TRUE(sent.has_routing);
    EXPECT_EQ(sent.routing.sender, 3);
}
