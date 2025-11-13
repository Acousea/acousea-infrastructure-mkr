#include <Arduino.h>
#include "unity.h"
#include "Result.h"

void test_success_case()
{
    auto result = Result<int>::success(42);
    TEST_ASSERT_TRUE(result.isSuccess());
    TEST_ASSERT_FALSE(result.isError());
    TEST_ASSERT_EQUAL(42, result.getValue());
    TEST_ASSERT_NOT_EQUAL(24, result.getValue());
}

void test_failure_case()
{
    auto result = Result<int>::failure("Error occurred");
    TEST_ASSERT_FALSE(result.isSuccess());
    TEST_ASSERT_TRUE(result.isError());
    TEST_ASSERT_EQUAL_STRING("Error occurred", result.getError());
}

int runUnityTests(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_success_case);
    RUN_TEST(test_failure_case);
    UNITY_END();
}

void setup()
{
    delay(2000);

    runUnityTests();
}

void loop()
{
}
