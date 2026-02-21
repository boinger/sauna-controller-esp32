# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [1.0.1] - 2026-02-20

### Changed

- Updated espressif32 platform 6.3.2 → 6.12.0 (security/stability patches; still Arduino-ESP32 2.x)
- Updated DallasTemperature 3.11.0 → 4.0.6 (major version, backward-compatible API)

## [1.0.0] - 2026-02-20

### Added

- ESP32 firmware with HomeKit native integration via HomeSpan
- REST API on port 8080 (`/status`, `/heater`, `/target`) for companion iOS app
- DS18B20 temperature sensor monitoring
- Safety system: 110°C hard limit, 60-minute session timeout, sensor fault detection, fail-safe defaults
- Unit tests for safety logic and HTTP validation (host-native, no hardware required)
- Static analysis via cppcheck
- KiCad schematic in `hardware/`
