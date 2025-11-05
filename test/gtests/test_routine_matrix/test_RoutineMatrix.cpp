#ifndef UNIT_TESTING
#define UNIT_TESTING
#endif

#include <gtest/gtest.h>
#include "NodeOperationRunner/RoutineMatrix/Group.hpp"
#include "NodeOperationRunner/RoutineMatrix/RoutineMatrix.hpp"
#include "NodeOperationRunner/RoutineMatrix/Utils.hpp"
#include "Result.h"
#include "IRoutine.h"
#include <optional>
#include "bindings/nodeDevice.pb.h"

// ======================================================
// DummyRoutine: mínima implementación de IRoutine
// ======================================================
class DummyRoutine : public IRoutine<acousea_CommunicationPacket>
{
public:
    explicit DummyRoutine(const std::string &name)
        : IRoutine<acousea_CommunicationPacket>(name)
    {
    }

    Result<acousea_CommunicationPacket> execute(const std::optional<acousea_CommunicationPacket> &input) override
    {
        (void)input;
        acousea_CommunicationPacket pkt = acousea_CommunicationPacket_init_default;
        return Result<acousea_CommunicationPacket>::success(pkt);
    }
};

// ======================================================
// FIXTURE
// ======================================================
class GroupAndMatrixTest : public ::testing::Test
{
protected:
    DummyRoutine r1{"RoutineA"};
    DummyRoutine r2{"RoutineB"};
    DummyRoutine r3{"RoutineC"};
    DummyRoutine r4{"RoutineD"};
};

// ======================================================
// TESTS PARA GROUP
// ======================================================

TEST_F(GroupAndMatrixTest, GroupStoresAndReturnsCorrectRoutines)
{
    constexpr uint8_t tags[] = {10, 11, 12};
    IRoutine<acousea_CommunicationPacket>* list[] = {&r1, nullptr, &r2};

    Group<IRoutine<acousea_CommunicationPacket>> group(tags, list, arr_count(tags));

    EXPECT_EQ(group.minTag(), 10);
    EXPECT_EQ(group.maxTag(), 12);
    EXPECT_EQ(group.capacity(), 3u);

    auto *routineA = group.get(10);
    ASSERT_NE(routineA, nullptr);
    EXPECT_EQ(routineA->routineName, "RoutineA");

    EXPECT_EQ(group.get(11), nullptr);

    auto *routineB = group.get(12);
    ASSERT_NE(routineB, nullptr);
    EXPECT_EQ(routineB->routineName, "RoutineB");

    EXPECT_EQ(group.get(13), nullptr);
    EXPECT_EQ(group.get(0), nullptr);
}

TEST_F(GroupAndMatrixTest, GroupContainsWorksAndValidatesBounds)
{
    constexpr uint8_t tags[] = {1, 2, 3};
    IRoutine<acousea_CommunicationPacket>* list[] = {&r1, &r2, &r3};
    Group<IRoutine<acousea_CommunicationPacket>> group(tags, list, arr_count(tags));

    EXPECT_TRUE(group.contains(1));
    EXPECT_TRUE(group.contains(2));
    EXPECT_TRUE(group.contains(3));
    EXPECT_FALSE(group.contains(0));
    EXPECT_FALSE(group.contains(4));
}

TEST_F(GroupAndMatrixTest, GroupOutOfRangeReturnsNullptr)
{
    constexpr uint8_t tags[] = {5, 6};
    IRoutine<acousea_CommunicationPacket>* list[] = {&r1, &r2};
    Group<IRoutine<acousea_CommunicationPacket>> group(tags, list, arr_count(tags));

    EXPECT_EQ(group.get(4), nullptr);
    EXPECT_EQ(group.get(7), nullptr);
}

// ======================================================
// TEST NUEVO: clear()
// ======================================================

TEST_F(GroupAndMatrixTest, GroupClearResetsAllEntries)
{
    constexpr uint8_t tags[] = {1, 2};
    IRoutine<acousea_CommunicationPacket>* list[] = {&r1, &r2};
    Group<IRoutine<acousea_CommunicationPacket>> group(tags, list, arr_count(tags));

    EXPECT_TRUE(group.contains(1));
    EXPECT_TRUE(group.contains(2));

    group.clear();

    EXPECT_FALSE(group.contains(1));
    EXPECT_FALSE(group.contains(2));
    EXPECT_EQ(group.get(1), nullptr);
}

// ======================================================
// TESTS PARA ROUTINE MATRIX (Group<Group<...>>)
// ======================================================

TEST_F(GroupAndMatrixTest, RoutineMatrixAccessesSubgroupsCorrectly)
{
    // Subgrupos internos
    constexpr uint8_t tagsA[] = {1, 2};
    IRoutine<acousea_CommunicationPacket>* listA[] = {&r1, &r2};
    Group<IRoutine<acousea_CommunicationPacket>> groupA(tagsA, listA, arr_count(tagsA));

    constexpr uint8_t tagsB[] = {10, 11};
    IRoutine<acousea_CommunicationPacket>* listB[] = {&r3, &r4};
    Group<IRoutine<acousea_CommunicationPacket>> groupB(tagsB, listB, arr_count(tagsB));

    // Matriz externa
    constexpr uint8_t matrixTags[] = {3, 4};
    Group<IRoutine<acousea_CommunicationPacket>>* groups[] = {&groupA, &groupB};

    Group<Group<IRoutine<acousea_CommunicationPacket>>> matrix(matrixTags, groups, arr_count(matrixTags));

    EXPECT_EQ(matrix.minTag(), 3);
    EXPECT_EQ(matrix.maxTag(), 4);
    EXPECT_EQ(matrix.capacity(), 2u);

    auto *cmdGroup = matrix.get(3);
    ASSERT_NE(cmdGroup, nullptr);

    auto *rspGroup = matrix.get(4);
    ASSERT_NE(rspGroup, nullptr);

    auto *routineA = cmdGroup->get(1);
    auto *routineD = rspGroup->get(11);

    ASSERT_NE(routineA, nullptr);
    ASSERT_NE(routineD, nullptr);
    EXPECT_EQ(routineA->routineName, "RoutineA");
    EXPECT_EQ(routineD->routineName, "RoutineD");
}

TEST_F(GroupAndMatrixTest, RoutineMatrixHandlesNullSubgroupEntries)
{
    constexpr uint8_t tagsA[] = {1, 2};
    IRoutine<acousea_CommunicationPacket>* listA[] = {&r1, &r2};
    Group<IRoutine<acousea_CommunicationPacket>> groupA(tagsA, listA, arr_count(tagsA));

    constexpr uint8_t matrixTags[] = {3, 4};
    Group<IRoutine<acousea_CommunicationPacket>>* groups[] = {&groupA, nullptr};

    Group<Group<IRoutine<acousea_CommunicationPacket>>> matrix(matrixTags, groups, arr_count(matrixTags));

    auto *g0 = matrix.get(3);
    ASSERT_NE(g0, nullptr);
    EXPECT_EQ(g0->get(1)->routineName, "RoutineA");

    EXPECT_EQ(matrix.get(4), nullptr);
}

TEST_F(GroupAndMatrixTest, RoutineMatrixReturnsNullptrForOutOfRangeTag)
{
    constexpr uint8_t tagsA[] = {1, 2};
    IRoutine<acousea_CommunicationPacket>* listA[] = {&r1, &r2};
    Group<IRoutine<acousea_CommunicationPacket>> groupA(tagsA, listA, arr_count(tagsA));

    constexpr uint8_t tagsB[] = {10, 11};
    IRoutine<acousea_CommunicationPacket>* listB[] = {&r3, &r4};
    Group<IRoutine<acousea_CommunicationPacket>> groupB(tagsB, listB, arr_count(tagsB));

    constexpr uint8_t matrixTags[] = {3, 4};
    Group<IRoutine<acousea_CommunicationPacket>>* groups[] = {&groupA, &groupB};

    Group<Group<IRoutine<acousea_CommunicationPacket>>> matrix(matrixTags, groups, arr_count(matrixTags));

    EXPECT_EQ(matrix.get(5), nullptr);
    EXPECT_EQ(matrix.get(0), nullptr);
}

TEST_F(GroupAndMatrixTest, GroupHandlesNonContiguousAndUnorderedTags)
{
    constexpr uint8_t tags[] = {1, 5, 3, 0, 0};
    IRoutine<acousea_CommunicationPacket>* list[] = {&r1, &r2, &r3, nullptr, nullptr};

    Group<IRoutine<acousea_CommunicationPacket>> group(tags, list, arr_count(tags));

    EXPECT_EQ(group.minTag(), 0);
    EXPECT_EQ(group.maxTag(), 5);
    EXPECT_EQ(group.capacity(), 6u);

    auto *routineA = group.get(1);
    auto *routineC = group.get(3);
    auto *routineB = group.get(5);

    ASSERT_NE(routineA, nullptr);
    ASSERT_NE(routineC, nullptr);
    ASSERT_NE(routineB, nullptr);

    EXPECT_EQ(routineA->routineName, "RoutineA");
    EXPECT_EQ(routineC->routineName, "RoutineC");
    EXPECT_EQ(routineB->routineName, "RoutineB");

    EXPECT_EQ(group.get(2), nullptr);
    EXPECT_EQ(group.get(4), nullptr);

    EXPECT_EQ(group.get(0), nullptr);
    EXPECT_EQ(group.get(6), nullptr);
}
