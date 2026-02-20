/**
 * http_validation.h — Input validation for REST API endpoints.
 *
 * Pure functions with no hardware dependencies — testable on any host.
 * Keeps sauna_logic.h (safety-critical) untouched.
 */

#ifndef HTTP_VALIDATION_H
#define HTTP_VALIDATION_H

// =============================================================================
// Target Temperature Range
// =============================================================================
constexpr float TARGET_TEMP_MIN = 40.0f;   // Minimum settable target (°C)
constexpr float TARGET_TEMP_MAX = 100.0f;  // Maximum settable target (°C)

// =============================================================================
// Validation Functions
// =============================================================================

/** Returns true if the heater state value is valid (0 = OFF, 1 = HEAT). */
inline bool isValidHeaterState(int state) {
    return state == 0 || state == 1;
}

/** Returns true if the target temperature is within the allowed range. */
inline bool isValidTargetTemp(float temp) {
    return temp >= TARGET_TEMP_MIN && temp <= TARGET_TEMP_MAX;
}

#endif // HTTP_VALIDATION_H
