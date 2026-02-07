/**
 * Unit tests for sauna_logic.h — runs on the host via PlatformIO native env.
 *
 * Covers all safety-critical decision functions with boundary conditions.
 */

#include <unity.h>
#include "sauna_logic.h"

// =============================================================================
// Sensor Fault Detection
// =============================================================================

void test_sensor_fault_at_disconnect_value(void) {
    TEST_ASSERT_TRUE(isSensorFault(-127.0f));
}

void test_sensor_fault_below_disconnect(void) {
    TEST_ASSERT_TRUE(isSensorFault(-128.0f));
}

void test_sensor_fault_zero_is_valid(void) {
    TEST_ASSERT_FALSE(isSensorFault(0.0f));
}

void test_sensor_fault_normal_temp(void) {
    TEST_ASSERT_FALSE(isSensorFault(75.0f));
}

void test_sensor_fault_negative_but_valid(void) {
    // -10°C is cold but a legitimate reading
    TEST_ASSERT_FALSE(isSensorFault(-10.0f));
}

// =============================================================================
// Over-Temperature Detection
// =============================================================================

void test_overtemp_at_limit(void) {
    TEST_ASSERT_TRUE(isOverTemperature(110.0f));
}

void test_overtemp_above_limit(void) {
    TEST_ASSERT_TRUE(isOverTemperature(115.0f));
}

void test_overtemp_just_below_limit(void) {
    TEST_ASSERT_FALSE(isOverTemperature(109.9f));
}

void test_overtemp_normal(void) {
    TEST_ASSERT_FALSE(isOverTemperature(80.0f));
}

// =============================================================================
// Session Timeout
// =============================================================================

void test_session_not_expired_at_start(void) {
    TEST_ASSERT_FALSE(isSessionExpired(0, 0));
}

void test_session_not_expired_midway(void) {
    // 30 minutes in
    TEST_ASSERT_FALSE(isSessionExpired(0, 30 * 60000UL));
}

void test_session_expired_at_limit(void) {
    TEST_ASSERT_TRUE(isSessionExpired(0, SESSION_MAX_MS));
}

void test_session_expired_past_limit(void) {
    TEST_ASSERT_TRUE(isSessionExpired(0, SESSION_MAX_MS + 1000));
}

void test_session_not_expired_offset_start(void) {
    // Started at 10s, now at 10s + 30min — not expired
    uint32_t start = 10000;
    uint32_t now = start + 30 * 60000UL;
    TEST_ASSERT_FALSE(isSessionExpired(start, now));
}

void test_session_expired_offset_start(void) {
    // Started at 10s, now at 10s + 60min — expired
    uint32_t start = 10000;
    uint32_t now = start + SESSION_MAX_MS;
    TEST_ASSERT_TRUE(isSessionExpired(start, now));
}

void test_session_millis_wraparound(void) {
    // millis() wrapped: start near max, now past zero
    // Elapsed = 0x00000010 - 0xFFFFFFF0 = 0x20 = 32ms (not expired)
    uint32_t start = 0xFFFFFFF0;
    uint32_t now = 0x00000010;
    TEST_ASSERT_FALSE(isSessionExpired(start, now));
}

// =============================================================================
// Thermostat Hysteresis
// =============================================================================

void test_hysteresis_engage_below_deadband(void) {
    // Target 80, hysteresis 2 → engage below 78
    TEST_ASSERT_TRUE(shouldHeaterEngage(77.0f, 80.0f, false));
}

void test_hysteresis_stay_off_in_deadband(void) {
    // 79°C is between (80-2)=78 and 80 — heater should stay OFF
    TEST_ASSERT_FALSE(shouldHeaterEngage(79.0f, 80.0f, false));
}

void test_hysteresis_stay_off_at_deadband_edge(void) {
    // Exactly at the lower threshold (78°C) — should NOT engage (< not <=)
    TEST_ASSERT_FALSE(shouldHeaterEngage(78.0f, 80.0f, false));
}

void test_hysteresis_disengage_at_target(void) {
    // Active heater reaches target → disengage
    TEST_ASSERT_FALSE(shouldHeaterEngage(80.0f, 80.0f, true));
}

void test_hysteresis_stay_on_in_deadband(void) {
    // Active heater at 79°C (below target, in deadband) → stay ON
    TEST_ASSERT_TRUE(shouldHeaterEngage(79.0f, 80.0f, true));
}

void test_hysteresis_cold_start(void) {
    // Room-temp sauna, heater off — should engage
    TEST_ASSERT_TRUE(shouldHeaterEngage(20.0f, 80.0f, false));
}

// =============================================================================
// HEAT Command Acceptance
// =============================================================================

void test_heat_command_accepted_no_fault(void) {
    TEST_ASSERT_TRUE(canAcceptHeatCommand(false));
}

void test_heat_command_blocked_on_fault(void) {
    TEST_ASSERT_FALSE(canAcceptHeatCommand(true));
}

// =============================================================================
// Test Runner
// =============================================================================

int main(void) {
    UNITY_BEGIN();

    // Sensor fault detection
    RUN_TEST(test_sensor_fault_at_disconnect_value);
    RUN_TEST(test_sensor_fault_below_disconnect);
    RUN_TEST(test_sensor_fault_zero_is_valid);
    RUN_TEST(test_sensor_fault_normal_temp);
    RUN_TEST(test_sensor_fault_negative_but_valid);

    // Over-temperature detection
    RUN_TEST(test_overtemp_at_limit);
    RUN_TEST(test_overtemp_above_limit);
    RUN_TEST(test_overtemp_just_below_limit);
    RUN_TEST(test_overtemp_normal);

    // Session timeout
    RUN_TEST(test_session_not_expired_at_start);
    RUN_TEST(test_session_not_expired_midway);
    RUN_TEST(test_session_expired_at_limit);
    RUN_TEST(test_session_expired_past_limit);
    RUN_TEST(test_session_not_expired_offset_start);
    RUN_TEST(test_session_expired_offset_start);
    RUN_TEST(test_session_millis_wraparound);

    // Thermostat hysteresis
    RUN_TEST(test_hysteresis_engage_below_deadband);
    RUN_TEST(test_hysteresis_stay_off_in_deadband);
    RUN_TEST(test_hysteresis_stay_off_at_deadband_edge);
    RUN_TEST(test_hysteresis_disengage_at_target);
    RUN_TEST(test_hysteresis_stay_on_in_deadband);
    RUN_TEST(test_hysteresis_cold_start);

    // HEAT command acceptance
    RUN_TEST(test_heat_command_accepted_no_fault);
    RUN_TEST(test_heat_command_blocked_on_fault);

    return UNITY_END();
}
