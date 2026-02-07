/**
 * Sauna Controller - ESP32 Firmware
 *
 * Controls a sauna heater via relay/contactor and monitors temperature
 * using DS18B20 sensor. Exposes HomeKit accessory for iOS integration.
 */

#include <Arduino.h>
#include <HomeSpan.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_task_wdt.h>

// =============================================================================
// Pin Definitions
// =============================================================================
constexpr uint8_t PIN_RELAY = 26;           // Relay output to contactor
constexpr uint8_t PIN_TEMP_SENSOR = 27;     // DS18B20 data pin
constexpr uint8_t PIN_STATUS_LED = 2;       // Onboard LED for status

// =============================================================================
// Configuration
// =============================================================================
constexpr float TEMP_MAX_CELSIUS = 110.0f;  // Safety limit
constexpr uint32_t SESSION_MAX_MINUTES = 60; // Hard limit fallback
constexpr uint32_t TEMP_READ_INTERVAL_MS = 2000;
constexpr float TEMP_HYSTERESIS = 2.0f;     // Deadband for thermostat cycling
constexpr uint32_t CONVERSION_WAIT_MS = 750; // DS18B20 12-bit conversion time

// =============================================================================
// Global Objects
// =============================================================================
OneWire oneWire(PIN_TEMP_SENSOR);
DallasTemperature tempSensor(&oneWire);

// =============================================================================
// HomeKit Accessory Definitions
// =============================================================================

// Thermostat accessory for sauna control
struct SaunaThermostat : Service::Thermostat {
    SpanCharacteristic *currentTemp;
    SpanCharacteristic *targetTemp;
    SpanCharacteristic *currentState;
    SpanCharacteristic *targetState;

    bool heaterActive = false;
    bool sensorFault = false;
    uint32_t sessionStartTime = 0;
    bool conversionRequested = false;
    uint32_t lastConversionRequest = 0;

    SaunaThermostat() : Service::Thermostat() {
        currentTemp = new Characteristic::CurrentTemperature(20.0);
        currentTemp->setRange(0, 120);

        targetTemp = new Characteristic::TargetTemperature(70.0);
        targetTemp->setRange(40, 100);

        currentState = new Characteristic::CurrentHeatingCoolingState(0);
        targetState = new Characteristic::TargetHeatingCoolingState(0);

        new Characteristic::TemperatureDisplayUnits(0);  // Celsius

        // Sauna only heats, no cooling
        targetState->setValidValues(2, 0, 1);  // OFF and HEAT only
    }

    bool update() override {
        if (targetState->updated()) {
            int state = targetState->getNewVal();

            if (state == 1 && sensorFault) {
                LOG1("SAFETY: HEAT command blocked — sensor fault active\n");
                return false;
            }

            if (state == 0) {
                setHeaterState(false);
            }
            // state == 1 (HEAT): don't turn heater on immediately.
            // loop() will engage it when temperature is below target.
            LOG1("HomeKit: Target state set to %s\n", state == 1 ? "HEAT" : "OFF");
        }

        if (targetTemp->updated()) {
            float target = targetTemp->getNewVal<float>();
            LOG1("HomeKit: Target temp set to %.1f°C\n", target);
        }

        return true;
    }

    void loop() override {
        uint32_t now = millis();

        // --- Session timeout safety check ---
        if (heaterActive && (now - sessionStartTime >= SESSION_MAX_MINUTES * 60000UL)) {
            setHeaterState(false);
            targetState->setVal(0);
            LOG1("SAFETY: Session time limit (%u min) reached, heater disabled\n",
                 SESSION_MAX_MINUTES);
        }

        // --- Async temperature read state machine ---
        if (!conversionRequested) {
            // Phase 1: Request a new conversion at the configured interval
            if (now - lastConversionRequest >= TEMP_READ_INTERVAL_MS) {
                tempSensor.requestTemperatures();
                conversionRequested = true;
                lastConversionRequest = now;
            }
        } else {
            // Phase 2: Wait for conversion to complete, then process
            if (now - lastConversionRequest >= CONVERSION_WAIT_MS) {
                conversionRequested = false;
                float temp = tempSensor.getTempCByIndex(0);

                if (temp <= DEVICE_DISCONNECTED_C) {
                    // Sensor fault — fail safe immediately
                    LOG1("SAFETY: Temperature sensor fault (%.1f), heater disabled\n", temp);
                    setHeaterState(false);
                    targetState->setVal(0);
                    sensorFault = true;
                    currentState->setVal(0);
                } else {
                    // Valid reading
                    sensorFault = false;
                    currentTemp->setVal(temp);

                    // Over-temperature safety cutoff
                    if (temp >= TEMP_MAX_CELSIUS) {
                        setHeaterState(false);
                        targetState->setVal(0);
                        LOG1("SAFETY: Max temp (%.0f°C) reached, heater disabled\n",
                             TEMP_MAX_CELSIUS);
                    }
                    // Thermostat hysteresis control
                    else if (targetState->getVal() == 1) {
                        float target = targetTemp->getVal<float>();
                        if (!heaterActive && temp < (target - TEMP_HYSTERESIS)) {
                            setHeaterState(true);
                        } else if (heaterActive && temp >= target) {
                            setHeaterState(false);
                        }
                    }

                    currentState->setVal(heaterActive ? 1 : 0);
                }
            }
        }
    }

    void setHeaterState(bool on) {
        heaterActive = on;
        digitalWrite(PIN_RELAY, on ? HIGH : LOW);
        digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW);
        if (on) {
            sessionStartTime = millis();
        }
    }
};

// =============================================================================
// Setup & Loop
// =============================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=================================");
    Serial.println("  Sauna Controller Starting...");
    Serial.println("=================================\n");

    // Initialize pins — heater OFF by default
    pinMode(PIN_RELAY, OUTPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);

    // Initialize temperature sensor
    tempSensor.begin();
    int sensorCount = tempSensor.getDeviceCount();
    Serial.printf("Found %d temperature sensor(s)\n", sensorCount);

    if (sensorCount == 0) {
        Serial.println("FATAL: No temperature sensor found — halting.");
        Serial.println("Check wiring on GPIO 27 and reset the device.");
        while (true) {
            digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
            delay(250);
        }
    }

    tempSensor.setWaitForConversion(false);  // Non-blocking reads

    // Initialize HomeSpan
    homeSpan.begin(Category::Thermostats, "Sauna Controller");

    // Create the accessory
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("Sauna");
            new Characteristic::Manufacturer("DIY");
            new Characteristic::Model("SaunaController-v1");
            new Characteristic::SerialNumber("001");
            new Characteristic::FirmwareRevision("1.0.0");
        new SaunaThermostat();

    Serial.println("\nHomeKit accessory ready.");
    Serial.println("Use the Home app to pair this device.\n");

    // Hardware watchdog — resets ESP32 if loop() stalls for 30s
    esp_task_wdt_init(30, true);
    esp_task_wdt_add(NULL);
}

void loop() {
    esp_task_wdt_reset();
    homeSpan.poll();
}
