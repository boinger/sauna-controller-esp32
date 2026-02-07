# Sauna Controller - ESP32 Firmware

## Project Overview
ESP32-based firmware for controlling a 7kW barrel sauna heater via relay/contactor. Exposes HomeKit accessory for native iOS Home app integration.

## Tech Stack
- **Platform**: ESP32 (using PlatformIO + Arduino framework)
- **HomeKit**: HomeSpan library for HAP implementation
- **Temperature**: DS18B20 sensor via OneWire/DallasTemperature

## Building & Flashing
```bash
# Build
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor

# Static analysis (cppcheck)
pio check

# Run unit tests (host, no hardware needed)
pio test -e native
```

## Development Guidelines
- This is an open-source project — keep code clean and well-documented
- Safety is paramount: always fail-safe (heater OFF on any error condition)
- Before commits: `pio run` (zero warnings), `pio check` (zero defects), `pio test -e native` (all pass)
- Test HomeKit pairing after any changes to accessory definitions
- Safety-critical logic lives in `include/sauna_logic.h` — pure functions, no hardware deps, tested natively

## Hardware Connections
- GPIO 26: Relay output (controls contactor)
- GPIO 27: DS18B20 temperature sensor data
- GPIO 2: Status LED (onboard)

## Safety Considerations
- Max temperature hard limit: 110°C
- Session hard limit: 60 minutes
- Sensor disconnect = heater OFF
- Any error condition = heater OFF
