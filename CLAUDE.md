# Sauna Controller - ESP32 Firmware

## Project Overview
ESP32-based firmware for controlling a 7kW barrel sauna heater via relay/contactor. Exposes HomeKit accessory (port 80) and REST API (port 8080) for iOS integration.

## Tech Stack
- **Platform**: ESP32 (using PlatformIO + Arduino framework)
- **HomeKit**: HomeSpan library for HAP implementation
- **Temperature**: DS18B20 sensor via OneWire/DallasTemperature

## Building & Flashing
```bash
# Build (ESP32 firmware)
pio run -e esp32

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor

# Static analysis (cppcheck)
pio check -e esp32

# Run unit tests (host, no hardware needed)
pio test -e native
```

## Development Guidelines
- This is an open-source project — keep code clean and well-documented
- Safety is paramount: always fail-safe (heater OFF on any error condition)
- Before commits: `pio run -e esp32` (zero warnings), `pio check -e esp32` (zero defects), `pio test -e native` (all pass)
- Test HomeKit pairing after any changes to accessory definitions
- Safety-critical logic lives in `include/sauna_logic.h` — pure functions, no hardware deps, tested natively
- HTTP input validation lives in `include/http_validation.h` — also pure functions, tested natively

## REST API (port 8080)
HomeSpan occupies port 80 for HAP, so the REST API runs on port 8080.

| Endpoint | Method | Body | Success | Errors |
|-----------|--------|------|---------|--------|
| `/status` | GET | — | 200 + JSON | — |
| `/heater` | POST | `{"state": 0\|1}` | 200 `{"ok":true}` | 400 (invalid), 503 (sensor fault) |
| `/target` | POST | `{"temperature": 40.0–100.0}` | 200 `{"ok":true}` | 400 (out of range) |

**Status response**: `{"current_temp":XX.X,"target_temp":70.0,"heating":false,"firmware":"1.0.0"}`

**Safety**: `POST /heater` with `state=1` sets target state to HEAT — the relay is only engaged by `loop()` through the full safety pipeline (sensor check, over-temp, session timeout). It never directly activates the heater.

## Interface Contract
See `SPEC.md` (project root) for the authoritative system specification. Do not modify the REST API endpoints, JSON schemas, or HomeKit characteristics without updating SPEC.md first.

## Hardware Connections
- GPIO 26: Relay output (controls contactor)
- GPIO 27: DS18B20 temperature sensor data
- GPIO 2: Status LED (onboard)

## Safety Considerations
- Max temperature hard limit: 110°C
- Session hard limit: 60 minutes
- Sensor disconnect = heater OFF
- Any error condition = heater OFF
