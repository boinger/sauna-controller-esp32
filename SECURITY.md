# Security Policy

## Important Safety Notice

This firmware controls a relay that switches high-voltage, high-current equipment (a sauna heater). **All electrical work must be done by a qualified electrician and must meet local building and electrical codes.** Improper installation can cause fire, electrocution, or death.

## Network Security Model

This project is designed for **local network use only**:

- The REST API (port 8080) uses plain HTTP with no authentication
- HomeKit communication (port 80) uses Apple's encrypted HAP protocol
- There is no cloud connectivity, no remote access, and no internet-facing service

The lack of authentication on the REST API is a deliberate design choice for a LAN-only appliance. If you expose your ESP32 to the internet, the REST API will be unauthenticated and unencrypted — **do not do this.**

## Reporting a Vulnerability

If you discover a security vulnerability, please report it responsibly:

- **GitHub Security Advisory**: Use the [Security Advisories](https://github.com/boinger/sauna-controller-esp32/security/advisories) tab to report privately
- **Email**: Contact the maintainer via their GitHub profile

Please do **not** open a public issue for security vulnerabilities.

## Scope

Security reports are welcome for:

- Bugs in the safety logic (temperature limits, session timeout, fail-safe behavior)
- Vulnerabilities in the HTTP request handling
- Issues with HomeSpan/HAP integration

Out of scope:

- The intentional lack of REST API authentication (see above)
- Physical access attacks (if someone has physical access to the ESP32, they have the relay)
