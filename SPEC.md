# Sauna Controller System Specification

Version 1.0 — February 2026

---

## 1. System Overview

The Sauna Controller is a two-component system for controlling an electric barrel sauna heater:

1. **ESP32 Firmware** (`ESP32/`) — Runs on an ESP32-WROOM-32. Controls the heater relay, reads the temperature sensor, and exposes both a HomeKit accessory (port 80) and a REST API (port 8080).
2. **iOS App** (`App/`) — Native SwiftUI app that communicates with the ESP32 over local WiFi via the REST API. Provides temperature monitoring, heater control, and target temperature setting.

Both interfaces (HomeKit and REST API) control the same thermostat state. Changes from one are immediately visible to the other.

```
┌─────────────┐     REST API (8080)     ┌──────────────────────┐
│   iOS App   │◄───────────────────────►│                      │
│  (SwiftUI)  │     GET/POST JSON       │                      │
└─────────────┘                         │    ESP32 Firmware     │
                                        │                      │
┌─────────────┐     HomeKit HAP (80)    │  ┌────────────────┐  │     ┌───────┐
│  Apple Home │◄───────────────────────►│  │ SaunaThermostat │──┼────►│ Relay │──► Contactor ──► Heater
│   / Siri    │                         │  └───────┬────────┘  │     └───────┘
└─────────────┘                         │          │           │
                                        │    ┌─────▼─────┐    │
                                        │    │  DS18B20   │    │
                                        │    │  Sensor    │    │
                                        │    └───────────┘    │
                                        └──────────────────────┘
```

## 2. Hardware

### Pin Assignments

| GPIO | Function | Notes |
|------|----------|-------|
| 26 | Relay output | Controls contactor coil |
| 27 | DS18B20 data | Requires 4.7k&#8486; pullup to 3.3V |
| 2 | Status LED | Onboard, mirrors heater state |

### Components

- **ESP32-WROOM-32** development board
- **DS18B20** waterproof temperature sensor (12-bit, 750ms conversion)
- **Relay module** (5V coil, appropriate for contactor)
- **Contactor** rated for heater load (7kW @ 240V = ~30A)

See `ESP32/hardware/README.md` and KiCad schematic for wiring details.

### Safety Hierarchy

```
Layer 0 (Hardware)  ── Thermal cutoff on heater (independent of firmware)
Layer 1 (Firmware)  ── sauna_logic.h safety functions (sensor fault, over-temp, session timeout)
Layer 2 (Software)  ── Input validation, UI disabling, error alerts
```

Layer 0 operates independently of all software. Layers 1 and 2 are defense-in-depth; Layer 1 is the authoritative safety boundary in firmware.

## 3. Safety Model

All safety logic lives in `ESP32/include/sauna_logic.h` as pure, hardware-independent functions. This file must not be modified without thorough review and testing.

### Invariants

| Invariant | Enforcement | Constant |
|-----------|-------------|----------|
| Temperature must never exceed safety limit | `isOverTemperature()` — heater OFF + targetState=0 | `TEMP_MAX_CELSIUS = 110.0`&#176;C |
| Sensor fault = immediate heater OFF | `isSensorFault()` — heater OFF + targetState=0 | `SENSOR_DISCONNECTED_C = -127.0`&#176;C |
| Sessions have a hard time limit | `isSessionExpired()` — heater OFF + targetState=0 | `SESSION_MAX_MS = 3,600,000` (60 min) |
| HEAT commands blocked during sensor fault | `canAcceptHeatCommand()` returns false | — |
| Heater is OFF on boot | `PIN_RELAY` set LOW in `setup()` before any logic runs | — |
| Thermostat uses hysteresis to prevent rapid cycling | `shouldHeaterEngage()` — deadband between engage/disengage thresholds | `TEMP_HYSTERESIS = 2.0`&#176;C |

### Critical Safety Rule

**No command path (HomeKit `update()` or REST handler) may directly call `setHeaterState(true)`.** The HEAT command sets `targetState` to 1; the `loop()` function engages the relay only after passing through the full safety pipeline:

1. Session timeout check
2. Sensor fault check
3. Over-temperature check
4. Hysteresis calculation (`shouldHeaterEngage()`)

Turning OFF (`state=0`) is always immediate and unconditional — `setHeaterState(false)` is called directly.

## 4. Communication Protocol

### 4.1 REST API (Port 8080)

Base URL: `http://<ESP32-IP>:8080`

HomeSpan occupies port 80 for the HomeKit Accessory Protocol (HAP). The iOS app hardcodes port 8080 in the base URL. Users enter only the IP address (e.g., `192.168.1.100`).

#### GET /status

Returns current thermostat state.

**Response** (200):
```json
{
  "current_temp": 72.5,
  "target_temp": 80.0,
  "heating": true,
  "firmware": "1.0.0"
}
```

| Field | Type | Description |
|-------|------|-------------|
| `current_temp` | float | Current temperature reading (&#176;C), 1 decimal |
| `target_temp` | float | Target temperature (&#176;C), 1 decimal |
| `heating` | boolean | Whether the heater relay is currently active |
| `firmware` | string | Firmware version |

#### POST /heater

Sets heater mode (OFF or HEAT).

**Request body**:
```json
{"state": 0}
```

| Field | Type | Valid Values | Description |
|-------|------|-------------|-------------|
| `state` | integer | 0, 1 | 0 = OFF, 1 = HEAT |

**Responses**:

| Status | Body | Condition |
|--------|------|-----------|
| 200 | `{"ok":true}` | Command accepted |
| 400 | `{"error":"invalid state, must be 0 or 1"}` | state not 0 or 1 |
| 400 | `{"error":"missing 'state' field"}` | No state in body |
| 503 | `{"error":"sensor fault active, cannot enable heater"}` | Sensor fault, state=1 rejected |

#### POST /target

Sets the target temperature.

**Request body**:
```json
{"temperature": 85.0}
```

| Field | Type | Valid Range | Description |
|-------|------|------------|-------------|
| `temperature` | float | 40.0–100.0 | Target temperature in &#176;C |

**Responses**:

| Status | Body | Condition |
|--------|------|-----------|
| 200 | `{"ok":true}` | Temperature set |
| 400 | `{"error":"temperature must be between 40 and 100"}` | Out of range |
| 400 | `{"error":"missing 'temperature' field"}` | No temperature in body |

### 4.2 HomeKit (Port 80)

The ESP32 exposes a **Thermostat** service via HomeSpan (HAP over port 80).

#### Characteristics

| Characteristic | Type | Range / Valid Values |
|----------------|------|---------------------|
| CurrentTemperature | float | 0–120&#176;C |
| TargetTemperature | float | 40–100&#176;C |
| CurrentHeatingCoolingState | int | 0 (OFF), 1 (HEAT) |
| TargetHeatingCoolingState | int | 0 (OFF), 1 (HEAT) — cooling disabled |
| TemperatureDisplayUnits | int | 0 (Celsius) |
| FirmwareRevision | string | Matches `FIRMWARE_VERSION` |

### 4.3 State Model

The `SaunaThermostat` struct is the single source of truth for all thermostat state.

#### Canonical State Variables

| Variable | Type | Owned By | Read By |
|----------|------|----------|---------|
| `currentTemp` | SpanCharacteristic (float) | Firmware loop (sensor reads) | HomeKit, REST `/status` |
| `targetTemp` | SpanCharacteristic (float) | HomeKit writes, REST `/target` | Firmware loop, REST `/status` |
| `currentState` | SpanCharacteristic (int) | Firmware loop | HomeKit |
| `targetState` | SpanCharacteristic (int) | HomeKit writes, REST `/heater` | Firmware loop |
| `heaterActive` | bool | `setHeaterState()` | REST `/status` (`heating` field) |
| `sensorFault` | bool | Firmware loop | REST `/heater` (503 guard) |
| `sessionStartTime` | uint32_t | `setHeaterState(true)` | Firmware loop (timeout check) |

#### Data Flow

```
HomeKit write ──► targetState/targetTemp ──► loop() reads ──► safety checks ──► setHeaterState()
REST POST    ──► targetState/targetTemp ──► loop() reads ──► safety checks ──► setHeaterState()
                                                                                      │
REST GET     ◄── heaterActive, currentTemp, targetTemp, firmware ◄────────────────────┘
HomeKit read ◄── currentState, currentTemp ◄──────────────────────────────────────────┘
```

Both command paths converge at the same SpanCharacteristic values. The `loop()` function is the sole authority for engaging the heater relay.

## 5. Firmware Architecture

### Boot Sequence (`setup()`)

1. Serial init (115200 baud)
2. Pin init — relay LOW (heater OFF), LED LOW
3. Temperature sensor init — halt if no sensor found (LED blink loop)
4. Set non-blocking conversion mode
5. HomeSpan init — thermostat service with characteristics
6. HTTP server init — register routes, begin on port 8080
7. Watchdog timer init (30s timeout)

### Main Loop (`loop()`)

Runs continuously, called by Arduino framework:

1. Reset watchdog timer
2. `homeSpan.poll()` — handles HomeKit communication
3. `httpServer.handleClient()` — handles REST API requests
4. `SaunaThermostat::loop()` (called by HomeSpan):
   a. Session timeout check — disable heater if expired
   b. Temperature read state machine (async, non-blocking):
      - Phase 1: Request conversion every 2s
      - Phase 2: Read result after 750ms conversion time
   c. On valid reading: over-temp check, then hysteresis-based heater control
   d. On sensor fault: immediate heater disable

### Temperature Read State Machine

```
                    ┌──────────────────────────┐
                    │   conversionRequested=F   │◄──── (initial state)
                    │   Wait for interval (2s)  │
                    └─────────┬────────────────┘
                              │ interval elapsed
                              ▼
                    ┌──────────────────────────┐
                    │  conversionRequested=T    │
                    │  requestTemperatures()    │
                    │  Wait for conversion      │
                    └─────────┬────────────────┘
                              │ 750ms elapsed
                              ▼
                    ┌──────────────────────────┐
                    │  getTempCByIndex(0)       │
                    │  Process reading          │──── loop back
                    └──────────────────────────┘
```

## 6. iOS App Architecture

### Pattern: MVVM with ObservableObject

- **SaunaManager** (`@MainActor ObservableObject`) — single source of truth, injected via `@EnvironmentObject`
- **Views** — SwiftUI, lightweight, delegate logic to SaunaManager
- **SaunaStatus** — `Codable` model for parsing `/status` response

### Network Layer

- `NetworkSession` protocol abstracts `URLSession` for testability
- Timeout: 5s request / 10s resource
- Base URL constructed from `controllerAddress` (user-entered IP) with port 8080 appended

### Polling

- Connected: 2s interval
- Disconnected: exponential backoff, 2s → 4s → 8s → ... → 30s max
- Polling task cancelled on address change or `deinit`

### Debounce

Both use Task-cancellation pattern (cancel previous → sleep → guard `!isCancelled` → execute):

| Input | Delay | Location |
|-------|-------|----------|
| Target temperature | 300ms | ContentView `onChanged` callback |
| IP address | 500ms | SettingsView |

### Error Handling

| Source | Behavior |
|--------|----------|
| Command errors (heater/target) | Set `lastError` → alert shown to user |
| Non-200 HTTP on commands | Set `lastError` with status code |
| Polling errors | Set `isConnected = false` only (no alert) |
| URL construction failure | Set `lastError` |

### Race Prevention

- `isCommandInFlight` flag disables power button during network requests
- `TargetTemperatureView` uses local `@State` + `onChanged` callback (not `@Binding`) to prevent poll-triggered round-trips

## 7. Shared Constants

These values must stay in sync between firmware and app. The firmware is authoritative.

| Constant | Firmware | iOS App | Value |
|----------|----------|---------|-------|
| Target temp minimum | `TARGET_TEMP_MIN` / `targetTemp->setRange` | Slider min | 40&#176;C |
| Target temp maximum | `TARGET_TEMP_MAX` / `targetTemp->setRange` | Slider max | 100&#176;C |
| Safety max temp | `TEMP_MAX_CELSIUS` | — (firmware-only) | 110&#176;C |
| Hysteresis | `TEMP_HYSTERESIS` | — (firmware-only) | 2&#176;C |
| Session timeout | `SESSION_MAX_MS` | — (firmware-only) | 60 min |
| Polling interval | — | `startPolling()` | 2s connected |
| Backoff max | — | `startPolling()` | 30s |
| Sensor disconnected | `SENSOR_DISCONNECTED_C` | — (firmware-only) | -127&#176;C |

## 8. Known Gaps and Decisions Needed

### 8.1 Session Timeout Disconnect

The firmware hard-codes a 60-minute session timeout (`SESSION_MAX_MINUTES`). The iOS `SaunaSession` model exists and is tested but is not populated — the app has no visibility into when the firmware will cut the heater. A future endpoint or `/status` field could expose remaining session time.

### 8.2 Sensor Fault Not in /status

The `/status` response does not include `sensor_fault`. If the sensor fails, the app only sees `heating: false` — it cannot distinguish between "heater was turned off" and "sensor fault forced emergency shutdown." Adding a `sensor_fault` boolean to `/status` would let the app show an appropriate warning.

### 8.3 Port Split

HomeKit occupies port 80, REST API runs on port 8080. The iOS app hardcodes port 8080; users enter only the IP address. mDNS auto-discovery could eliminate the remaining manual IP entry.

### 8.4 Error Response Parsing

The iOS app checks HTTP status codes but does not parse error response bodies. The firmware returns structured error messages (e.g., `{"error": "sensor fault active, cannot enable heater"}`) that could be shown to the user instead of generic "HTTP 503" messages.

### 8.5 SaunaSession Model Unused

`SaunaSession` (SwiftData `@Model`) is defined and tested but never populated. Wiring it up requires detecting session start/end from polling state transitions.

### 8.6 Celsius Only

The system is Celsius throughout. The HomeKit `TemperatureDisplayUnits` characteristic is set to 0 (Celsius). A Fahrenheit toggle would need conversion in the app UI layer only — the firmware and API always use Celsius.
