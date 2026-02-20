/**
 * Unit tests for http_validation.h — runs on the host via PlatformIO native env.
 *
 * Covers boundary conditions for REST API input validation.
 */

#include <unity.h>
#include "http_validation.h"

// =============================================================================
// Heater State Validation
// =============================================================================

void test_heater_state_zero_is_valid(void) {
    TEST_ASSERT_TRUE(isValidHeaterState(0));
}

void test_heater_state_one_is_valid(void) {
    TEST_ASSERT_TRUE(isValidHeaterState(1));
}

void test_heater_state_two_is_invalid(void) {
    TEST_ASSERT_FALSE(isValidHeaterState(2));
}

void test_heater_state_negative_is_invalid(void) {
    TEST_ASSERT_FALSE(isValidHeaterState(-1));
}

void test_heater_state_large_value_is_invalid(void) {
    TEST_ASSERT_FALSE(isValidHeaterState(99));
}

// =============================================================================
// Target Temperature Validation
// =============================================================================

void test_target_temp_at_minimum(void) {
    TEST_ASSERT_TRUE(isValidTargetTemp(40.0f));
}

void test_target_temp_at_maximum(void) {
    TEST_ASSERT_TRUE(isValidTargetTemp(100.0f));
}

void test_target_temp_mid_range(void) {
    TEST_ASSERT_TRUE(isValidTargetTemp(70.0f));
}

void test_target_temp_below_minimum(void) {
    TEST_ASSERT_FALSE(isValidTargetTemp(39.9f));
}

void test_target_temp_above_maximum(void) {
    TEST_ASSERT_FALSE(isValidTargetTemp(100.1f));
}

// =============================================================================
// Test Runner
// =============================================================================

int main(void) {
    UNITY_BEGIN();

    // Heater state validation
    RUN_TEST(test_heater_state_zero_is_valid);
    RUN_TEST(test_heater_state_one_is_valid);
    RUN_TEST(test_heater_state_two_is_invalid);
    RUN_TEST(test_heater_state_negative_is_invalid);
    RUN_TEST(test_heater_state_large_value_is_invalid);

    // Target temperature validation
    RUN_TEST(test_target_temp_at_minimum);
    RUN_TEST(test_target_temp_at_maximum);
    RUN_TEST(test_target_temp_mid_range);
    RUN_TEST(test_target_temp_below_minimum);
    RUN_TEST(test_target_temp_above_maximum);

    return UNITY_END();
}
