# Contributing to Sauna Controller ESP32

Thanks for your interest in contributing! This project controls high-voltage equipment, so code quality and safety are paramount.

## Getting Started

1. Install [PlatformIO](https://platformio.org/) (Core or IDE plugin)
2. Clone the repo and open in your editor
3. Build: `pio run -e esp32`

## Pre-Commit Gates

All of these must pass before submitting a PR:

- **`pio run -e esp32`** — Compiles with zero warnings
- **`pio check -e esp32`** — Static analysis (cppcheck) reports zero defects
- **`pio test -e native`** — All unit tests pass

## Safety Rules

This firmware controls a relay that switches high-voltage, high-current equipment. These rules are non-negotiable:

- **No direct heater activation.** No code path may call `setHeaterState(true)` directly. The relay is only engaged by `loop()` through the full safety pipeline (sensor check, over-temp limit, session timeout).
- **Safety-critical logic lives in `include/sauna_logic.h`.** This is a pure-function header with no hardware dependencies, specifically so it can be tested natively. Changes to safety logic require corresponding test updates.
- **Fail-safe defaults.** On boot, on error, or on any ambiguous state, the heater must be OFF.

## Specification Contract

The REST API contract, safety model, and architecture are defined in [SPEC.md](SPEC.md). API changes (endpoints, JSON schemas, status codes) require updating SPEC.md **first**, then implementing the change.

## Workflow

1. **Open an issue** describing the change you'd like to make
2. **Discuss** the approach before writing code
3. **Fork and branch** from `main`
4. **Implement** with passing gates (see above)
5. **Open a PR** referencing the issue

## Code Style

- C++ with Arduino framework conventions
- Header-only pure functions for testable logic (`include/`)
- Hardware interaction in `src/main.cpp`
- Comments for non-obvious behavior, especially safety-related logic
