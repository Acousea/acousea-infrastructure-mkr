#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "NodeOperationRunner/NodeOperationRunner.h"
#include "NodeConfigurationRepository/NodeConfigurationRepository.h"
#include "Logger/Logger.h"
#include "ErrorHandler/ErrorHandler.h"
#include <ConsoleDisplay/ConsoleDisplay.hpp>
#include "common_test_resources.hpp"
#include "Result.h"
#include "../common_test_resources.hpp"


// =====================================================================
// Mock Router para pruebas
// =====================================================================

class DummyRouter : public Router {
public:
    struct SentPacket {
        IPort::PortType portType;
        acousea_CommunicationPacket packet;
    };

    std::vector<SentPacket> sentPackets;
    std::map<IPort::PortType, std::vector<acousea_CommunicationPacket>> queuedPackets;

    DummyRouter() : Router(std::vector<IPort*>{}) {}

    struct DummyThrough {
        DummyRouter& parent;
        IPort::PortType portType;
        bool send(const acousea_CommunicationPacket& packet) const {
            parent.sentPackets.push_back({portType, packet});
            return true; // simula envío correcto
        }
    };

    struct DummyFrom {
        DummyRouter& parent;
        uint8_t address;
        DummyThrough through(IPort::PortType port) const { return DummyThrough{parent, port}; }
    };

    DummyFrom from(uint8_t addr) { return DummyFrom{*this, addr}; }

    // Permite inyectar paquetes "recibidos"
    void queueIncoming(IPort::PortType port, const acousea_CommunicationPacket& pkt) {
        queuedPackets[port].push_back(pkt);
    }

    std::map<IPort::PortType, std::vector<acousea_CommunicationPacket>> readPorts(uint8_t) {
        auto result = queuedPackets;
        queuedPackets.clear(); // se consumen al leer
        return result;
    }
};


// =====================================================================
// Mock Routine (simula ejecución de comandos o reportes)
// =====================================================================

class DummyRoutine : public IRoutine<acousea_CommunicationPacket>
{
public:
    mutable int executeCount = 0;
    Result<acousea_CommunicationPacket> resultToReturn;

    explicit DummyRoutine(const std::string &name)
        : IRoutine<acousea_CommunicationPacket>(name), resultToReturn(RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default))
    {
    }

    DummyRoutine(const std::string &name, Result<acousea_CommunicationPacket> result)
        : IRoutine<acousea_CommunicationPacket>(name), resultToReturn(std::move(result))
    {
    }

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket> &input) override
    {
        executeCount++;
        (void)input;
        return resultToReturn;
    }
};


// =====================================================================
// Fixture
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

    DummyRouter router;
    MockStorageManager storageManager;
    TestableNodeConfigurationRepository repo{storageManager};
    ConsoleDisplay display;
};

// =====================================================================
// TESTS adaptados con DummyRouter real y TestableNodeConfigurationRepository
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
    DummyRoutine dummy(
        "RoutineA", RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default));
    std::map<uint8_t, std::map<uint8_t, IRoutine<acousea_CommunicationPacket>*>> routines = {
        {
            acousea_CommunicationPacket_command_tag,
            {{acousea_CommandBody_setConfiguration_tag, &dummy}}
        }
    };

    NodeOperationRunner runner(router, repo, routines);

    auto found = runner.findRoutine(acousea_CommunicationPacket_command_tag,
                                    acousea_CommandBody_setConfiguration_tag);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found.value(), &dummy);
}

TEST_F(NodeOperationRunnerTest, ExecuteRoutineReturnsSuccessPacket)
{
    acousea_CommunicationPacket expected = acousea_CommunicationPacket_init_default;
    Result<acousea_CommunicationPacket> ok = RESULT_SUCCESS(acousea_CommunicationPacket, expected);
    DummyRoutine dummy("SuccessRoutine", ok);

    NodeOperationRunner runner(router, repo, {});

    IRoutine<acousea_CommunicationPacket>* routinePtr = &dummy;
    auto out = runner.executeRoutine(routinePtr , std::nullopt, IPort::PortType::LoraPort, 0, false);
    ASSERT_TRUE(out.has_value());
}

TEST_F(NodeOperationRunnerTest, ExecuteRoutineRequeuesWhenPending)
{
    Result<acousea_CommunicationPacket> pending = RESULT_PENDINGF(acousea_CommunicationPacket, "waiting");
    DummyRoutine dummy("PendingRoutine", pending);

    NodeOperationRunner runner(router, repo, {});

    IRoutine<acousea_CommunicationPacket>* routinePtr = &dummy;
    // IRoutine<acousea_CommunicationPacket>*& routineRef = routinePtr;
    runner.executeRoutine(routinePtr, std::nullopt, IPort::PortType::LoraPort, 2, true);
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
    repo.setTestConfig(cfg);

    NodeOperationRunner runner(router, repo, {});
    runner.init();

    auto result = runner.getReportingEntryForCurrentOperationMode(
        cfg.operationModesModule.modes[0].id, IPort::PortType::LoraPort);
    ASSERT_TRUE(result.isSuccess());
    EXPECT_EQ(result.getValueConst().period, 60);
}

TEST_F(NodeOperationRunnerTest, RunPendingRoutinesProcessesAll)
{
    Result<acousea_CommunicationPacket> ok =
        RESULT_SUCCESS(acousea_CommunicationPacket, acousea_CommunicationPacket_init_default);
    DummyRoutine dummy("Pending", ok);

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
        {
            acousea_CommunicationPacket_command_tag,
            {{acousea_CommandBody_setConfiguration_tag, &dummy}}
        }
    };

    NodeOperationRunner runner(router, repo, routines);
    runner.init();

    acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
    pkt.has_routing = true;
    pkt.routing.sender = 77;
    pkt.which_body = acousea_CommunicationPacket_command_tag;
    pkt.body.command.which_command = acousea_CommandBody_setConfiguration_tag;

    router.queueIncoming(IPort::PortType::LoraPort, pkt);

    EXPECT_NO_THROW(runner.processIncomingPackets(repo.getNodeConfiguration().localAddress));

    ASSERT_FALSE(router.sentPackets.empty());
    EXPECT_EQ(router.sentPackets.front().portType, IPort::PortType::LoraPort);
}
