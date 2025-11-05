#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "NodeOperationRunner/NodeOperationRunner.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>
#include "Result.h"

// Recursos comunes
#include "../common_test_resources/InMemoryStorageManager.hpp"
#include "../common_test_resources/TestableNodeConfigurationRepository.hpp"
#include "../common_test_resources/DummyRoutine.hpp"
#include "../common_test_resources/DummyPort.hpp"
#include "../common_test_resources/PacketUtils.hpp"


// =====================================================================
// Fixture para NodeOperationRunner
// =====================================================================
class NodeOperationRunnerTest : public ::testing::Test
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

    DummyPort lora{IPort::PortType::LoraPort};
    DummyPort iridium{IPort::PortType::SBDPort};
    std::vector<IPort*> ports{&lora, &iridium};
    Router router{ports};

    InMemoryStorageManager storageManager;
    TestableNodeConfigurationRepository repo{storageManager};
    ConsoleDisplay display;
};

// =====================================================================
// TESTS
// =====================================================================

TEST_F(NodeOperationRunnerTest, InitLoadsConfigurationCorrectly)
{
    auto cfg = TestableNodeConfigurationRepository::makeValidNodeConfig();
    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines;

    NodeOperationRunner runner(router, repo, routines);
    EXPECT_NO_THROW(runner.init());
}

TEST_F(NodeOperationRunnerTest, SearchForOperationModeReturnsCorrectMode)
{
    auto cfg = TestableNodeConfigurationRepository::makeValidNodeConfig();
    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines;

    NodeOperationRunner runner(router, repo, routines);
    runner.init();

    auto result = runner.searchForOperationMode(cfg.operationModesModule.activeModeId);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.getValueConst().id, cfg.operationModesModule.modes[0].id);
}

TEST_F(NodeOperationRunnerTest, SearchForOperationModeFailsWhenNotFound)
{
    NodeOperationRunner runner(router, repo, {});
    runner.init();

    auto result = runner.searchForOperationMode(255);
    EXPECT_TRUE(result.isError());
}

TEST_F(NodeOperationRunnerTest, FindRoutineReturnsCorrectPointer)
{
    DummyRoutine dummy("RoutineA",
        RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default));

    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines = {
        { acousea_CommunicationPacket_command_tag,
          {{acousea_CommandBody_setConfiguration_tag, &dummy}} }
    };

    NodeOperationRunner runner(router, repo, routines);
    auto found = runner.findRoutine(acousea_CommunicationPacket_command_tag,
                                    acousea_CommandBody_setConfiguration_tag);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found.value(), &dummy);
}

TEST_F(NodeOperationRunnerTest, ExecuteRoutineReturnsSuccessPacket)
{
    DummyRoutine dummy("SuccessRoutine",
        RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default));

    NodeOperationRunner runner(router, repo, {});
    IRoutine<acousea_CommunicationPacket>* routinePtr = &dummy;

    auto out = runner.executeRoutine(routinePtr, std::nullopt,
                                     IPort::PortType::LoraPort, 0, false);
    ASSERT_TRUE(out.has_value());
}

TEST_F(NodeOperationRunnerTest, ExecuteRoutineRequeuesWhenPending)
{
    DummyRoutine dummy("PendingRoutine",
        RESULT_PENDINGF(acousea_CommunicationPacket, "waiting"));

    NodeOperationRunner runner(router, repo, {});
    IRoutine<acousea_CommunicationPacket>* routinePtr = &dummy;

    runner.executeRoutine(routinePtr, std::nullopt,
                          IPort::PortType::LoraPort, 2, true);
    EXPECT_GT(dummy.executeCount, 0);
}

TEST_F(NodeOperationRunnerTest, ProcessPacketReturnsErrorPacketWhenNoRoutine)
{
    NodeOperationRunner runner(router, repo, {});

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing.sender = 55;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;

    auto out = runner.processPacket(IPort::PortType::LoraPort, pkt);
    ASSERT_TRUE(out.has_value());
    EXPECT_EQ(out->which_body, acousea_CommunicationPacket_error_tag);
}

TEST_F(NodeOperationRunnerTest, TryTransitionChangesModeWhenDurationReached)
{
    NodeOperationRunner runner(router, repo, {});
    runner.init();

    acousea_OperationMode mode = acousea_OperationMode_init_default;
    mode.id = 1;
    mode.has_transition = true;
    mode.transition.targetModeId = 1;
    mode.transition.duration = 0;

    runner.cache.currentOperationMode = mode;
    runner.tryTransitionOpMode();

    EXPECT_EQ(runner.cache.currentOperationMode.id, 1);
}

TEST_F(NodeOperationRunnerTest, GetReportingEntryReturnsCorrectEntry)
{
    auto cfg = TestableNodeConfigurationRepository::makeValidNodeConfig();
    cfg.has_loraModule = true;
    cfg.loraModule.entries_count = 1;
    cfg.loraModule.entries[0].modeId = cfg.operationModesModule.modes[0].id;
    cfg.loraModule.entries[0].period = 60;
    repo.saveConfiguration(cfg);

    NodeOperationRunner runner(router, repo, {});
    runner.init();

    auto result = runner.getReportingEntryForCurrentOperationMode(
        cfg.operationModesModule.modes[0].id, IPort::PortType::LoraPort);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.getValueConst().period, 60);
}

TEST_F(NodeOperationRunnerTest, RunPendingRoutinesProcessesAll)
{
    DummyRoutine dummy("Pending",
        RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default));

    NodeOperationRunner runner(router, repo, {});
    runner.pendingRoutines.add({&dummy, std::nullopt, 1, IPort::PortType::LoraPort});

    EXPECT_NO_THROW(runner.runPendingRoutines());
    EXPECT_GE(dummy.executeCount, 1);
}

TEST_F(NodeOperationRunnerTest, ProcessIncomingPacketsSendsResponsesThroughRouter)
{
    DummyRoutine dummy("ResponseRoutine",
        RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default));

    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines = {
        { acousea_CommunicationPacket_command_tag,
          {{acousea_CommandBody_setConfiguration_tag, &dummy}} }
    };

    NodeOperationRunner runner(router, repo, routines);
    runner.init();

    // Construimos un paquete en alto nivel y lo codificamos con nanopb
    const uint8_t localAddr = repo.getNodeConfiguration().localAddress;

    acousea_CommunicationPacket pkt = PacketUtils::makeRoutedPacket(/*sender=*/77, /*receiver=*/localAddr);

    // Explicit Command->SetConfiguration although it comes by default on makeRoutedPacket,
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;

    // Codificar a bytes (nanopb)
    std::vector<uint8_t> raw = PacketUtils::encodePacketTest(pkt);

    // Inyectar los bytes en el puerto LoRa
    lora.enqueueRaw(raw);

    // Procesar, esto hará que Router lea/decodifique, el runner ejecute la rutina
    // y envíe la respuesta por el mismo puerto
    runner.processIncomingPackets(localAddr);

    // Debe haberse enviado algo por el puerto LoRa
    EXPECT_FALSE(lora.sentPackets.empty());
}