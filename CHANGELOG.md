# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [1.0.0] - 2026-02-20

### Added

- ESP32 firmware with HomeKit native integration via HomeSpan
- REST API on port 8080 (`/status`, `/heater`, `/target`) for companion iOS app
- DS18B20 temperature sensor monitoring
- Safety system: 110°C hard limit, 60-minute session timeout, sensor fault detection, fail-safe defaults
- Unit tests for safety logic and HTTP validation (host-native, no hardware required)
- Static analysis via cppcheck
- KiCad schematic in `hardware/`
