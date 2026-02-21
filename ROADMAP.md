# Sauna Controller Roadmap

Prioritized feature roadmap for the Sauna Controller system (ESP32 firmware + iOS app).

---

## Tier 1 — Required for First Real Use

These must be done before the system is trustworthy for actual sauna sessions.

### Add `sensor_fault` to /status response
The iOS app cannot distinguish between "heater turned off" and "sensor fault forced emergency shutdown." Add a `sensor_fault` boolean to the `/status` JSON response and show a warning banner in the app when active.
- **Firmware**: Add `"sensor_fault":true|false` to `handleGetStatus()` JSON
- **iOS**: Add field to `SaunaStatus`, show warning in UI when true
- **SPEC.md**: Update §4.1 GET /status schema

### Session timeout visibility
The firmware enforces a 60-minute session timeout, but the app has no visibility into it. The user needs to know how much time remains before the heater auto-disables.
- **Firmware**: Add `session_remaining_sec` to `/status` (0 when heater is off)
- **iOS**: Display countdown timer when heating is active
- **SPEC.md**: Update §4.1 GET /status schema

### Wire up SaunaSession SwiftData model
`SaunaSession` is defined and tested but never populated. Detect session start/end from polling state transitions (`isHeating` false→true = start, true→false = end) and persist to SwiftData.
- **iOS only**: State machine in `SaunaManager` or a dedicated `SessionTracker`

### Parse error response bodies
The iOS app checks HTTP status codes but shows generic messages like "HTTP 503." The firmware returns structured error JSON (e.g., `{"error":"sensor fault active"}`). Parse and display the server's error message.
- **iOS only**: Decode error body in `setHeaterState()` and `setTargetTemperature()`

---

## Tier 2 — Quality of Life

Improve the day-to-day experience but not required for safe operation.

### mDNS auto-discovery
Replace manual IP entry with automatic discovery. The iOS app already hardcodes port 8080, so users only enter the IP address — mDNS would eliminate this remaining manual step. The ESP32 can advertise via mDNS (e.g., `sauna.local`), and the iOS app can browse for the service.
- **Firmware**: Add `ESPmDNS` service advertisement in `setup()`
- **iOS**: Use `NWBrowser` to discover `_http._tcp` services, pre-fill address

### Configurable session timeout
The firmware hard-codes 60 minutes. Add an endpoint to configure the timeout (within safe bounds, e.g., 15–120 minutes) and expose it in the iOS settings UI.
- **Firmware**: New `POST /session-timeout` endpoint + persistence (NVS or EEPROM)
- **iOS**: Stepper in SettingsView
- **SPEC.md**: New endpoint documentation

### OTA firmware updates
Enable over-the-air updates so the ESP32 doesn't need a USB connection for firmware upgrades. ArduinoOTA is already a dependency of HomeSpan.
- **Firmware**: Enable ArduinoOTA in `setup()`, add version check endpoint
- **iOS**: Show "update available" indicator when firmware version is behind

### Celsius/Fahrenheit toggle
The system is Celsius-only. Add a display units toggle in the iOS app — conversion happens in the UI layer only; the API and firmware always use Celsius.
- **iOS only**: Toggle in SettingsView, conversion in display views

---

## Tier 3 — Future Enhancements

Nice-to-have features for a more complete product.

- **Scheduling**: Set sauna start times (preheat so it's ready when you arrive)
- **Push notifications**: Alert when target temperature is reached
- **Temperature graphing**: Chart temperature over the session duration
- **WebSocket push**: Replace polling with server-push for instant updates
- **Apple Watch app / Widget**: Quick status and control from watch face or home screen
- **Home Assistant integration**: MQTT or REST integration for non-Apple smart home users
- **Multi-sensor support**: Additional DS18B20 sensors (e.g., bench level vs. ceiling)

---

## Excluded

Features investigated and intentionally rejected.

| Feature | Reason |
|---------|--------|
| Cloud connectivity | Violates local-only design principle. No accounts, no subscriptions. |
| Cooling mode | Saunas heat. The thermostat's cooling modes are disabled in HomeKit (`setValidValues(2, 0, 1)`). |
| Direct HomeKit control from iOS app | The app uses the REST API; HomeKit is a separate control path via Apple Home. Merging them would complicate state management with no UX benefit. |
| ArduinoJson library | Manual JSON parsing is sufficient for 3 simple endpoints. Avoids a dependency and keeps flash usage lower. |
