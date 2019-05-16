/*
    Copyright (c) 2018 Patrick Moffitt

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
 */

#include <bitset>
#include <cmath>
#include <locale>
#include <Arduino.h>
#include "../lib/WiFi101/src/WiFi101.h"
#include "monitor_data.hpp"
#include "ntp_time_utils.hpp"
#include "ADAFRUIT_IO_MQTT.hpp"
#include "monitor_oled_display.hpp"
#include "monitor_temp_rh_sensor.hpp"
#include "monitor_current_sensor.hpp"
#include "monitor_read_battery.hpp"
#include "wifi101_helper.hpp"
#include "float_to_fixed_width.hpp"
#include "serial_debug_error.hpp"
#include <RTCZero.h>

#define LOOP_LIMIT 4

#define SLEEP_INTERVAL_SECONDS 900

// Float format helper.
using format = float_to_fixed_width;

// struct to hold sensor measurements.
monitor_data sensor;

// struct for setting system time of day.
ntp_time_utils time_util;

RTCZero rtc;

// Create a WiFiClient class to connect to the MQTT server.
WiFiSSLClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client,  // @ToDo load adafruit.io TLS cert to MCU.
                          MQTT_SERVER,
                          AIO_SERVERPORT,
                          MQTT_CLIENTID,
                          MQTT_USERNAME,
                          MQTT_PASSWORD);

// OLED Display Utilities
monitor_display oled;

// Temperature and Humidity Utilities
DHT_Unified dht(DHTPIN, DHTTYPE);

// Current Sensor Utilities
Adafruit_INA219 ina219;

volatile bool display_data{false};  // Button A toggles the display
volatile bool degrees_c_f{false};   // Button C toggles the temperature scale.
volatile bool system_time_set{false};
volatile int loop_counter{0};
std::bitset<5> publish_status{0};

void alarm_handler();  // Advance declaration
volatile bool alarm_set{false};

void monitor_deep_sleep();  //  Advance declaration.

void setup() {
#if defined(SERIAL_DEBUG) || defined(SERIAL_ERROR)
    Serial.begin(115200);
#endif
    rtc.begin();
    rtc.attachInterrupt(alarm_handler);
    pinMode(PIN_LED_13, LOW);
    dht.begin();     // Initialize the DHT sensor.
    ina219.begin();  // Initialize the INA219 sensor.
    oled.enable();   // Enable the SSD1306 OLED Display.
    pinMode(BUTTON_A, INPUT_PULLUP);
    attachInterrupt(
            digitalPinToInterrupt(BUTTON_A),
            [](){display_data = not display_data;},
            FALLING
            );
    pinMode(BUTTON_B, INPUT_PULLUP);
    attachInterrupt(
            digitalPinToInterrupt(BUTTON_B),
            [](){oled.page == 2 ? (oled.page = 0) : (oled.page++); loop_counter = 1;},
            FALLING
            );
    pinMode(BUTTON_C, INPUT_PULLUP);
    attachInterrupt(
            digitalPinToInterrupt(BUTTON_C),
            [](){degrees_c_f = not degrees_c_f;},
            FALLING
            );
    // Configure pins for Adafruit ATWINC1500 Feather.
    WiFi.setPins(8, 7, 4, 2);
    WiFi.maxLowPowerMode();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void loop() {
    loop_counter++;
    DEBUG_PRINT("Loop "); DEBUG_PRINT(loop_counter); DEBUG_PRINT(" of ");
    DEBUG_PRINTLN(LOOP_LIMIT);

    if (alarm_set == true) {
        alarm_set = false;
        oled.enable();
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        delay(500);
        return;
    }

    if (loop_counter == LOOP_LIMIT) {
        monitor_deep_sleep();
    }

    if (publish_status.any() and not display_data) {
        monitor_deep_sleep();
    }

    if (WiFi.status() != WL_CONNECTED) {
        uint8_t mac_address[6];
        WiFi.macAddress(mac_address);
        DEBUG_PRINT("WiFi is not connected. ");
        DEBUG_PRINT("MAC Address: "); DEBUG_PRINTLN(macv4_int_to_str(mac_address));
        delay(500 * loop_counter);
        return;
    }
    DEBUG_PRINT("WiFi Connected. IP: "); DEBUG_PRINTLN(ipv4_int_to_str(WiFi.localIP()));

    if (not mqtt.connected()) {
        mqtt.connect();
        delay(500 * loop_counter);
        if (not mqtt.connected()) {
            DEBUG_PRINTLN("Adafruit.io MQTT connection failure.");
            return;
        }
    }
    DEBUG_PRINT("MQTT Connection Status: "); DEBUG_PRINTLN(mqtt.connected());

    sensor.battery_vdc = get_battery_percent();
    sensor.current_ma = ina219.getCurrent_mA();
    if (std::isnan(sensor.current_ma)) {
        DEBUG_PRINTLN("Error reading current sensor!");
        return;
    }
    DEBUG_PRINT("Current : "); DEBUG_PRINT(sensor.current_ma); DEBUG_PRINTLN("mA");

    sensors_event_t event;
    dht.temperature().getEvent(&event);
    if (std::isnan(event.temperature)) {
        DEBUG_PRINTLN("Error reading temperature!");
        return;
    }
    sensor.temperature_f = event.temperature * 1.8 + 32;
    DEBUG_PRINT("Temperature: "); DEBUG_PRINT(sensor.temperature_f); DEBUG_PRINTLN(" ℉");

    dht.humidity().getEvent(&event);
    if (std::isnan(event.relative_humidity)) {
        DEBUG_PRINTLN("Error reading humidity!");
        return;
    }
    sensor.humidity_rh = event.relative_humidity;
    DEBUG_PRINT("Relative Humidity: "); DEBUG_PRINT(sensor.humidity_rh); DEBUG_PRINTLN(" ϕ");

    if (display_data) {
        oled.show_page(oled.page);
        delay(5000);
    }

    char payload_float[AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE];  // Float max 39 digits and minus sign.
    char zero{'\0'};

    if (mqtt.connected()) {
        // Publish Battery VDC Percent
        format::to_fixed_width(sensor.battery_vdc, ZERO_FIELD_PAD, payload_float);
        publish_status[0] = mqtt.publish(BATTERY_VDC, payload_float, 0);
        memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

        // Publish Current mA
        format::to_fixed_width(sensor.current_ma, ZERO_FIELD_PAD, AIO_FLOAT_PRECISION, payload_float);
        publish_status[1] = mqtt.publish(CURRENT_MA, payload_float, 0);
        memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

        // Publish Humidity ϕ
        format::to_fixed_width(sensor.humidity_rh, ZERO_FIELD_PAD, AIO_FLOAT_PRECISION, payload_float);
        publish_status[2] = mqtt.publish(HUMIDITY_RH, payload_float, 0);
        memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

        // Publish Temperature ℉
        format::to_fixed_width(sensor.temperature_f, ZERO_FIELD_PAD, AIO_FLOAT_PRECISION, payload_float);
        publish_status[3] = mqtt.publish(TEMPERATURE_F, payload_float, 0);
        memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

        // Publish Unix Epoch Time UTC
        publish_status[4] = mqtt.publish(UNIX_EPOCH_TIME, sensor.unix_epoch_time, 0);
        delay(100);
    }

    if (publish_status.any()) {  // At least one item was published.
        DEBUG_PRINT("Publish status battery: ");
        DEBUG_PRINTLN(publish_status[0]);
        DEBUG_PRINT("Publish status current: ");
        DEBUG_PRINTLN(publish_status[1]);
        DEBUG_PRINT("Publish status humidity: ");
        DEBUG_PRINTLN(publish_status[2]);
        DEBUG_PRINT("Publish status temperature: ");
        DEBUG_PRINTLN(publish_status[3]);
        DEBUG_PRINT("Publish status time: ");
        DEBUG_PRINTLN(publish_status[4]);
    }

}

void alarm_handler() {
    alarm_set = true;
}


void monitor_deep_sleep() {
    pinMode(PIN_LED_13, LOW);
    oled.disable();
    display_data = false;
    oled.page = 0;
    if (mqtt.connected()) {
        mqtt.disconnect();
    }
    client.stop();
    WiFi.end();

    auto epoch = rtc.getEpoch();
    auto alarm_time = epoch + SLEEP_INTERVAL_SECONDS;

    DEBUG_PRINTLN("Setting alarm and going to sleep.");
    DEBUG_PRINT("RTC Epoch: "); DEBUG_PRINTLN(epoch);
    DEBUG_PRINT("RTC Alarm: "); DEBUG_PRINTLN(alarm_time);

    rtc.setAlarmEpoch((uint32) alarm_time);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.standbyMode();
}