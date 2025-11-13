#include <Arduino.h>
#include <unity.h>

#include "StorageManager/SDStorageManager/SDStorageManager.h"
#include "StorageManager/SDStorageManager/SDPath/SDPath.hpp"
#include "PacketQueue/PacketQueue.hpp"
#include "ProtoUtils/ProtoUtils.hpp"
#include "bindings/nodeDevice.pb.h"
#include "MockRTCController/MockRTCController.h"

// ---------------------------------------------------------
// GLOBALS
// ---------------------------------------------------------
static SDStorageManager sdStorage(SDCARD_SS_PIN);
static MockRTCController mockRTC;              // Usar el MockRTCController
static PacketQueue queue(sdStorage, mockRTC); // Pasa el MockRTCController al PacketQueue

// ========== PACKET HELPERS ==========

// 1) Basic command packet
static acousea_CommunicationPacket makePacket_basic(uint32_t id)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;

    pkt.packetId = id;
    pkt.has_routing = true;
    pkt.routing.sender = 1;
    pkt.routing.receiver = 2;
    pkt.routing.ttl = 10;

    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;

    pkt.body.command.command.setConfiguration.modules_count = 1;
    pkt.body.command.command.setConfiguration.modules[0].key = acousea_ModuleCode_AMBIENT_MODULE;
    pkt.body.command.command.setConfiguration.modules[0].has_value = true;

    pkt.body.command.command.setConfiguration.modules[0].value.which_module =
        acousea_ModuleWrapper_ambient_tag;
    pkt.body.command.command.setConfiguration.modules[0].value.module.ambient.temperature = 123;
    pkt.body.command.command.setConfiguration.modules[0].value.module.ambient.humidity = 77;

    return pkt;
}

// 2) Status Report Packet
static acousea_CommunicationPacket makePacket_statusReport(uint32_t id)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;

    pkt.packetId = id;
    pkt.has_routing = true;
    pkt.routing.sender = 5;
    pkt.routing.receiver = 8;
    pkt.routing.ttl = 2;

    pkt.which_body = acousea_CommunicationPacket_report_tag;
    pkt.body.report.which_report = acousea_ReportBody_statusPayload_tag;

    auto &st = pkt.body.report.report.statusPayload;
    st.reportTypeId = 99;
    st.modules_count = 1;

    st.modules[0].key = acousea_ModuleCode_BATTERY_MODULE;
    st.modules[0].has_value = true;

    auto &bm = st.modules[0].value;
    bm.which_module = acousea_ModuleWrapper_battery_tag;
    bm.module.battery.batteryPercentage = 82;
    bm.module.battery.batteryStatus = acousea_BatteryStatus_BATTERY_STATUS_DISCHARGING;

    return pkt;
}

// 3) Error Packet
static acousea_CommunicationPacket makePacket_error(uint32_t id, const char *message)
{
    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;

    pkt.packetId = id;
    pkt.has_routing = true;
    pkt.routing.sender = 10;
    pkt.routing.receiver = 33;
    pkt.routing.ttl = 1;

    pkt.which_body = acousea_CommunicationPacket_error_tag;
    strncpy(pkt.body.error.errorMessage, message, sizeof(pkt.body.error.errorMessage) - 1);

    return pkt;
}

// ========== ENCODE / DECODE HELPERS ==========
// Helper: serializa un packet en un buffer
static size_t encodePacket(const acousea_CommunicationPacket &pkt, uint8_t *out, size_t maxSize)
{
    auto res = ProtoUtils::CommunicationPacket::encodeInto(pkt, out, maxSize);
    TEST_ASSERT_TRUE_MESSAGE(res.isSuccess(), "Packet encoding failed");
    return res.getValueConst();
}

static acousea_CommunicationPacket decodePacket(const uint8_t *data, size_t length)
{
    acousea_CommunicationPacket pkt;
    auto res = ProtoUtils::CommunicationPacket::decodeInto(data, length, &pkt);
    TEST_ASSERT_TRUE_MESSAGE(res.isSuccess(), "Packet decoding failed");
    return pkt;
}

// ---------------------------------------------------------
// TESTS
// ---------------------------------------------------------

void test_begin_and_clear()
{
    // delay(1000);
    TEST_ASSERT_TRUE(queue.begin());
    TEST_ASSERT_TRUE(queue.clear(1));
    TEST_ASSERT_TRUE(queue.isPortEmpty(1));
}

void test_push_and_popAny_single()
{
    // delay(1000);
    queue.clear(1);
    TEST_ASSERT_TRUE(queue.isPortEmpty(1));

    uint8_t buf[256];

    // Nuevo type: basic command packet
    acousea_CommunicationPacket pkt = makePacket_basic(42);
    size_t encoded = encodePacket(pkt, buf, sizeof(buf));

    TEST_ASSERT_TRUE(queue.push(0x01, buf, encoded));
    TEST_ASSERT_FALSE(queue.isPortEmpty(1));

    uint8_t out[256];
    uint16_t n = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(encoded, n);

    // Decode y verificar
    acousea_CommunicationPacket decoded = decodePacket(out, n);
    TEST_ASSERT_EQUAL_UINT32(42, decoded.packetId);

    TEST_ASSERT_TRUE(queue.isPortEmpty(1));
}

void test_push_multiple_ports_and_popForPort()
{
    // delay(1000);
    queue.clear(1);
    uint8_t buf[256];

    // Packet A: basic (port 0x01)
    size_t len1 = encodePacket(makePacket_basic(100), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x01, buf, len1));

    // Packet B: status report (port 0x02)
    size_t len2 = encodePacket(makePacket_statusReport(200), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x02, buf, len2));

    // Packet C: error (port 0x01)
    size_t len3 = encodePacket(makePacket_error(300, "E_FAIL"), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x01, buf, len3));

    TEST_ASSERT_FALSE(queue.isPortEmpty(0x01));
    TEST_ASSERT_FALSE(queue.isPortEmpty(0x02));

    uint8_t out[256];

    // pop first 0x01 → packetId 100
    uint16_t r1 = queue.popNext(0x01, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(len1, r1);

    auto p1 = decodePacket(out, r1);
    TEST_ASSERT_EQUAL_UINT32(100, p1.packetId);

    // Now queue contains: [200 @0x02] [300 @0x01]
    uint16_t r2 = queue.popNext(0x01, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(len3, r2);

    auto p2 = decodePacket(out, r2);
    TEST_ASSERT_EQUAL_UINT32(300, p2.packetId);
    TEST_ASSERT_EQUAL_STRING("E_FAIL", p2.body.error.errorMessage);

    // Now only packet left is 200 @ 0x02
    TEST_ASSERT_TRUE(queue.isPortEmpty(0x01));

    uint16_t r3 = queue.popNext(0x02, out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(len2, r3);

    auto p3 = decodePacket(out, r3);
    TEST_ASSERT_EQUAL_UINT32(200, p3.packetId);

    TEST_ASSERT_TRUE(queue.isEmpty());
}

void test_popAny_keeps_order_and_compacts()
{
    // delay(1000);
    queue.clear(1);
    uint8_t buf[256];

    size_t l1 = encodePacket(makePacket_basic(1), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x01, buf, l1));

    size_t l2 = encodePacket(makePacket_statusReport(2), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x02, buf, l2));

    size_t l3 = encodePacket(makePacket_error(3, "Critical"), buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x03, buf, l3));

    uint8_t out[256];

    // Packet 1
    uint16_t r1 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l1, r1);

    auto p1 = decodePacket(out, r1);
    TEST_ASSERT_EQUAL_UINT32(1, p1.packetId);

    // Packet 2
    uint16_t r2 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l2, r2);

    auto p2 = decodePacket(out, r2);
    TEST_ASSERT_EQUAL_UINT32(2, p2.packetId);

    // Packet 3
    uint16_t r3 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l3, r3);

    auto p3 = decodePacket(out, r3);
    TEST_ASSERT_EQUAL_UINT32(3, p3.packetId);
    TEST_ASSERT_EQUAL_STRING("Critical", p3.body.error.errorMessage);

    TEST_ASSERT_TRUE(queue.isEmpty());
}

void test_multiple_packet_types()
{
    // delay(1000);
    queue.clear(1);
    uint8_t buf[512];

    // Make packets of different types
    auto p1 = makePacket_basic(101);
    auto p2 = makePacket_statusReport(202);
    auto p3 = makePacket_error(303, "Sensor failure");

    size_t l1 = encodePacket(p1, buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x01, buf, l1));

    size_t l2 = encodePacket(p2, buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x02, buf, l2));

    size_t l3 = encodePacket(p3, buf, sizeof(buf));
    TEST_ASSERT_TRUE(queue.push(0x03, buf, l3));

    // Now pop in order: popAny
    uint8_t out[512];

    uint16_t n1 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l1, n1);
    auto r1 = decodePacket(out, n1);
    TEST_ASSERT_EQUAL_UINT32(101, r1.packetId);

    uint16_t n2 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l2, n2);
    auto r2 = decodePacket(out, n2);
    TEST_ASSERT_EQUAL_UINT32(202, r2.packetId);

    uint16_t n3 = queue.popAny(out, sizeof(out));
    TEST_ASSERT_EQUAL_UINT16(l3, n3);
    auto r3 = decodePacket(out, n3);
    TEST_ASSERT_EQUAL_UINT32(303, r3.packetId);

    TEST_ASSERT_TRUE(queue.isEmpty());
}

// ---------------------------------------------------------
// Arduino entry
// ---------------------------------------------------------

void setup()
{
    delay(2000); // Allow serial to attach

    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }

    const bool storageOk = sdStorage.begin();
    if (!storageOk)
    {
        Serial.println("SDStorageManager begin() failed!");
        while (true)
        {
            delay(1000);
        }
    }

    mockRTC.setEpoch(1622505600); // June 1, 2021

    UNITY_BEGIN();

    RUN_TEST(test_begin_and_clear);
    RUN_TEST(test_push_and_popAny_single);
    RUN_TEST(test_push_multiple_ports_and_popForPort);
    RUN_TEST(test_popAny_keeps_order_and_compacts);
    RUN_TEST(test_multiple_packet_types);

    UNITY_END();
}

void loop() {}
