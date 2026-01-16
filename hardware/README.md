# Sauna Controller Hardware

KiCad schematic and wiring documentation for the ESP32 sauna controller.

## Components

| Reference | Part | Description |
|-----------|------|-------------|
| U1 | ESP32-DevKitC | Main controller module |
| U2 | DS18B20 | Waterproof temperature probe |
| R1 | 4.7kΩ | Pull-up resistor for DS18B20 data line |
| K1 | SSR-25DA | Solid state relay (3-32VDC → 24-380VAC) |
| K2 | C25BNB230T | Existing contactor in heater (240VAC coil) |
| S1 | — | Existing timer switch in heater |

## Wiring Summary

### Low Voltage (3.3V DC) - ESP32 Side

```
ESP32 GPIO26 ────────────────────► SSR DC+ (pin 3)
ESP32 GND ───────────────────────► SSR DC- (pin 4)

ESP32 3.3V ──┬───────────────────► DS18B20 VDD
             │
             └──── R1 (4.7kΩ) ───┬► DS18B20 DQ ◄── ESP32 GPIO27
                                 │
ESP32 GND ───────────────────────► DS18B20 GND
```

### High Voltage (240V AC) - Heater Side

```
AC Hot (L) ──┬─► SSR AC1 (pin 1)
             │        │
             │        ▼
             │   SSR AC2 (pin 2) ──┬──► Contactor Coil A1
             │                     │
             └─► Timer Switch ─────┘         │
                                             ▼
AC Neutral (N) ◄───────────────── Contactor Coil A2
```

The SSR and existing timer are wired **in parallel** — either can energize the contactor coil.

## Pin Assignments

| ESP32 Pin | Function | Connected To |
|-----------|----------|--------------|
| GPIO 26 | SSR Control | SSR-25DA DC+ (pin 3) |
| GPIO 27 | Temp Sensor | DS18B20 DQ (data) |
| 3.3V | Power | DS18B20 VDD, R1 |
| GND | Ground | DS18B20 GND, SSR DC- |

## Safety Notes

⚠️ **WARNING: This project involves 240VAC mains voltage.**

- All high-voltage work should be performed by a qualified electrician
- Ensure power is disconnected before working on wiring
- Use appropriate wire gauges for current ratings
- The ESP32 and low-voltage components must be in an enclosure away from heat/moisture
- The DS18B20 probe is rated to 125°C and can be placed inside the sauna
- The heater's built-in thermal cutoff provides hardware over-temperature protection

## Enclosure Recommendations

- ESP32 + SSR: IP65/IP67 enclosure, mounted outside sauna (ambient temp < 50°C)
- Route DS18B20 cable through grommet/cable gland
- Ensure adequate ventilation if SSR is switching frequently (though at <1A coil load, heat is minimal)

## Files

- `sauna-controller.kicad_pro` - KiCad project file
- `sauna-controller.kicad_sch` - Schematic (open with KiCad 8+)
