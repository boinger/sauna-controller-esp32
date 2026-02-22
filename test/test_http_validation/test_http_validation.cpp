/**
 * Unit tests for http_validation.h — runs on the host via PlatformIO native env.
 *
 * Covers boundary conditions for REST API input validation.
 */

#include <unity.h>
#include "http_validation.h"

void setUp(void) {}
void tearDown(void) {}

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
// isTrailingClean
// =============================================================================

void test_trailing_clean_empty(void) {
    TEST_ASSERT_TRUE(isTrailingClean(""));
}

void test_trailing_clean_null(void) {
    TEST_ASSERT_TRUE(isTrailingClean(nullptr));
}

void test_trailing_clean_whitespace(void) {
    TEST_ASSERT_TRUE(isTrailingClean("  \t\n"));
}

void test_trailing_clean_closing_brace(void) {
    TEST_ASSERT_TRUE(isTrailingClean("}"));
}

void test_trailing_clean_closing_bracket(void) {
    TEST_ASSERT_TRUE(isTrailingClean("]"));
}

void test_trailing_clean_brace_with_whitespace(void) {
    TEST_ASSERT_TRUE(isTrailingClean(" } "));
}

void test_trailing_clean_rejects_alpha(void) {
    TEST_ASSERT_FALSE(isTrailingClean("abc"));
}

void test_trailing_clean_rejects_mixed(void) {
    TEST_ASSERT_FALSE(isTrailingClean("}x"));
}

// =============================================================================
// parseIntValue
// =============================================================================

void test_parse_int_valid_zero(void) {
    int out = -1;
    bool ok = parseIntValue(" 0}", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(0, out);
}

void test_parse_int_valid_one(void) {
    int out = -1;
    bool ok = parseIntValue(" 1}", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(1, out);
}

void test_parse_int_negative(void) {
    int out = 0;
    bool ok = parseIntValue("-5", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_INT(-5, out);
}

void test_parse_int_rejects_null(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue(nullptr, out));
}

void test_parse_int_rejects_empty(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue("", out));
}

void test_parse_int_rejects_garbage(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue("garbage", out));
}

void test_parse_int_rejects_trailing_alpha(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue("1abc", out));
}

void test_parse_int_rejects_decimal(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue("1.5", out));
}

void test_parse_int_whitespace_only(void) {
    int out = 0;
    TEST_ASSERT_FALSE(parseIntValue("   ", out));
}

// =============================================================================
// parseFloatValue
// =============================================================================

void test_parse_float_valid_integer(void) {
    float out = -1.0f;
    bool ok = parseFloatValue(" 70}", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, out);
}

void test_parse_float_valid_decimal(void) {
    float out = -1.0f;
    bool ok = parseFloatValue("85.5}", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_FLOAT(85.5f, out);
}

void test_parse_float_negative(void) {
    float out = 0.0f;
    bool ok = parseFloatValue("-3.14", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -3.14f, out);
}

void test_parse_float_trailing_whitespace(void) {
    float out = -1.0f;
    bool ok = parseFloatValue("42.0  ", out);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL_FLOAT(42.0f, out);
}

void test_parse_float_rejects_null(void) {
    float out = 0.0f;
    TEST_ASSERT_FALSE(parseFloatValue(nullptr, out));
}

void test_parse_float_rejects_empty(void) {
    float out = 0.0f;
    TEST_ASSERT_FALSE(parseFloatValue("", out));
}

void test_parse_float_rejects_garbage(void) {
    float out = 0.0f;
    TEST_ASSERT_FALSE(parseFloatValue("garbage", out));
}

void test_parse_float_rejects_trailing_alpha(void) {
    float out = 0.0f;
    TEST_ASSERT_FALSE(parseFloatValue("70.0abc", out));
}

void test_parse_float_whitespace_only(void) {
    float out = 0.0f;
    TEST_ASSERT_FALSE(parseFloatValue("   ", out));
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

    // isTrailingClean
    RUN_TEST(test_trailing_clean_empty);
    RUN_TEST(test_trailing_clean_null);
    RUN_TEST(test_trailing_clean_whitespace);
    RUN_TEST(test_trailing_clean_closing_brace);
    RUN_TEST(test_trailing_clean_closing_bracket);
    RUN_TEST(test_trailing_clean_brace_with_whitespace);
    RUN_TEST(test_trailing_clean_rejects_alpha);
    RUN_TEST(test_trailing_clean_rejects_mixed);

    // parseIntValue
    RUN_TEST(test_parse_int_valid_zero);
    RUN_TEST(test_parse_int_valid_one);
    RUN_TEST(test_parse_int_negative);
    RUN_TEST(test_parse_int_rejects_null);
    RUN_TEST(test_parse_int_rejects_empty);
    RUN_TEST(test_parse_int_rejects_garbage);
    RUN_TEST(test_parse_int_rejects_trailing_alpha);
    RUN_TEST(test_parse_int_rejects_decimal);
    RUN_TEST(test_parse_int_whitespace_only);

    // parseFloatValue
    RUN_TEST(test_parse_float_valid_integer);
    RUN_TEST(test_parse_float_valid_decimal);
    RUN_TEST(test_parse_float_negative);
    RUN_TEST(test_parse_float_trailing_whitespace);
    RUN_TEST(test_parse_float_rejects_null);
    RUN_TEST(test_parse_float_rejects_empty);
    RUN_TEST(test_parse_float_rejects_garbage);
    RUN_TEST(test_parse_float_rejects_trailing_alpha);
    RUN_TEST(test_parse_float_whitespace_only);

    return UNITY_END();
}
