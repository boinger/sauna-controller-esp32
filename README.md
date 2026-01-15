# Sauna Controller - ESP32 Firmware

Open-source ESP32 firmware for controlling an electric sauna heater with native Apple HomeKit integration.

## Features

- **HomeKit Native**: Appears as a Thermostat in Apple Home app — control via Siri or Home app
- **Temperature Monitoring**: Real-time temperature from DS18B20 sensor
- **Safety First**: Hard temperature limits, session timeouts, fail-safe defaults
- **Local Only**: No cloud, no accounts, no subscriptions — just your local WiFi

## Hardware Requirements

- ESP32 development board (ESP32-WROOM-32 or similar)
- DS18B20 temperature sensor (waterproof version recommended for sauna environment)
- Relay module (5V, appropriate for your contactor coil)
- Contactor rated for your heater (7kW @ 240V = ~30A)

### Wiring

```
ESP32 GPIO 26 ──► Relay IN ──► Contactor Coil
ESP32 GPIO 27 ──► DS18B20 Data (with 4.7kΩ pullup to 3.3V)
ESP32 3.3V    ──► DS18B20 VCC
ESP32 GND     ──► DS18B20 GND, Relay GND
```

⚠️ **WARNING**: This project controls high-voltage equipment. Ensure all electrical work is done by a qualified electrician and meets local codes.

## Building

Requires [PlatformIO](https://platformio.org/).

```bash
# Clone the repo
git clone https://github.com/YOUR_USERNAME/sauna-controller-esp32.git
cd sauna-controller-esp32

# Build
pio run

# Flash to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## HomeKit Setup

1. Power on the ESP32
2. Open the Home app on your iPhone
3. Tap "+" → "Add Accessory"
4. The ESP32 will broadcast as "Sauna Controller"
5. Follow pairing prompts (default setup code shown in serial monitor)

## Safety Features

- **Max Temperature**: Heater auto-disables at 110°C (configurable)
- **Session Limit**: 60-minute hard timeout (configurable)
- **Sensor Failure**: Heater disables if temperature sensor disconnects
- **Fail-Safe Default**: Heater is OFF on boot and on any error

## License

MIT License — see [LICENSE](LICENSE) for details.

## Contributing

Contributions welcome! Please open an issue first to discuss proposed changes.
