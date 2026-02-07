/**
 * sauna_logic.h — Pure safety and thermostat logic for the sauna controller.
 *
 * All functions are hardware-independent (no Arduino/HomeSpan deps) so they
 * can be compiled and tested on any host platform via the native test env.
 */

#ifndef SAUNA_LOGIC_H
#define SAUNA_LOGIC_H

#include <cstdint>

// =============================================================================
// Safety Constants
// =============================================================================
constexpr float TEMP_MAX_CELSIUS       = 110.0f;   // Absolute safety limit
constexpr uint32_t SESSION_MAX_MINUTES = 60;        // Hard session timeout
constexpr uint32_t SESSION_MAX_MS      = SESSION_MAX_MINUTES * 60000UL;
constexpr float TEMP_HYSTERESIS        = 2.0f;      // Deadband for thermostat cycling
constexpr float SENSOR_DISCONNECTED_C  = -127.0f;   // Must match DEVICE_DISCONNECTED_C

// =============================================================================
// Pure Logic Functions
// =============================================================================

/** Returns true when the temperature reading indicates a disconnected sensor. */
inline bool isSensorFault(float temp) {
    return temp <= SENSOR_DISCONNECTED_C;
}

/** Returns true when temperature has reached or exceeded the safety limit. */
inline bool isOverTemperature(float temp) {
    return temp >= TEMP_MAX_CELSIUS;
}

/**
 * Returns true when the session has exceeded the maximum duration.
 * Uses unsigned subtraction so it handles millis() wraparound correctly.
 */
inline bool isSessionExpired(uint32_t startMs, uint32_t nowMs) {
    return (nowMs - startMs) >= SESSION_MAX_MS;
}

/**
 * Thermostat hysteresis: decides whether the heater should be ON or OFF.
 *
 *   - When inactive: engage only when current < (target - TEMP_HYSTERESIS)
 *   - When active:   disengage when current >= target
 *   - In the deadband between those thresholds, maintain current state
 *
 * Returns the desired heater state (true = ON).
 */
inline bool shouldHeaterEngage(float current, float target, bool active) {
    if (!active && current < (target - TEMP_HYSTERESIS)) {
        return true;
    }
    if (active && current >= target) {
        return false;
    }
    return active;
}

/**
 * Returns true if a HEAT command should be accepted.
 * Blocks the command when the sensor is in a fault state.
 */
inline bool canAcceptHeatCommand(bool sensorFault) {
    return !sensorFault;
}

#endif // SAUNA_LOGIC_H
