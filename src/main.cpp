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

    SaunaThermostat() : Service::Thermostat() {
        currentTemp = new Characteristic::CurrentTemperature(20.0);
        currentTemp->setRange(0, 120);

        targetTemp = new Characteristic::TargetTemperature(70.0);
        targetTemp->setRange(40, 100);

        currentState = new Characteristic::CurrentHeatingCoolingState(0);
        targetState = new Characteristic::TargetHeatingCoolingState(0);

        // Sauna only heats, no cooling
        targetState->setValidValues(2, 0, 1);  // OFF and HEAT only
    }

    bool update() override {
        // Called when HomeKit sends a command
        if (targetState->updated()) {
            int state = targetState->getNewVal();
            setHeaterState(state == 1);  // 1 = HEAT
            LOG1("HomeKit: Heater set to %s\n", state == 1 ? "ON" : "OFF");
        }

        if (targetTemp->updated()) {
            float target = targetTemp->getNewVal<float>();
            LOG1("HomeKit: Target temp set to %.1f°C\n", target);
        }

        return true;
    }

    void loop() override {
        // Called every loop iteration - update current temperature
        static uint32_t lastTempRead = 0;

        if (millis() - lastTempRead >= TEMP_READ_INTERVAL_MS) {
            lastTempRead = millis();
            float temp = readTemperature();

            if (temp > -100) {  // Valid reading
                currentTemp->setVal(temp);

                // Update heating state based on actual conditions
                bool heaterOn = digitalRead(PIN_RELAY) == HIGH;
                currentState->setVal(heaterOn ? 1 : 0);

                // Safety check
                if (temp >= TEMP_MAX_CELSIUS) {
                    setHeaterState(false);
                    targetState->setVal(0);
                    LOG1("SAFETY: Max temp reached, heater disabled\n");
                }
            }
        }
    }

    void setHeaterState(bool on) {
        digitalWrite(PIN_RELAY, on ? HIGH : LOW);
        digitalWrite(PIN_STATUS_LED, on ? HIGH : LOW);
    }

    float readTemperature() {
        tempSensor.requestTemperatures();
        float tempC = tempSensor.getTempCByIndex(0);

        if (tempC == DEVICE_DISCONNECTED_C) {
            LOG1("ERROR: Temperature sensor disconnected\n");
            return -127.0f;
        }

        return tempC;
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

    // Initialize pins
    pinMode(PIN_RELAY, OUTPUT);
    pinMode(PIN_STATUS_LED, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);  // Start with heater OFF

    // Initialize temperature sensor
    tempSensor.begin();
    int sensorCount = tempSensor.getDeviceCount();
    Serial.printf("Found %d temperature sensor(s)\n", sensorCount);

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
}

void loop() {
    homeSpan.poll();
}
