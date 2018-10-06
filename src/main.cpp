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
#include <WiFi101.h>
#include <avr/dtostrf.h>
#include "monitor_data.hpp"
#include "ntp_time_utils.hpp"
#include "ADAFRUIT_IO_MQTT.hpp"
#include "monitor_oled_display.hpp"
#include "monitor_temp_rh_sensor.hpp"
#include "monitor_current_sensor.hpp"
#include "monitor_read_battery.hpp"
#include "wifi101_helper.hpp"
#include <RTCZero.h>


#define LOOP_LIMIT 6

#define SLEEP_INTERVAL_SECONDS 900

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
volatile int8_t mqtt_connect_status{-1};
volatile int loop_counter{1};

void alarm_handler();  // Advance declaration
volatile bool alarm_set{false};

void monitor_deep_sleep();  //  Advance declaration.

int wifi_connection_attempts{0};

void setup() {
    Serial.begin(115200);
    // Serial.setDebugOutput(true);
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
            [](){oled.page == 2 ? (oled.page = 0) : (oled.page++);},
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
    /*
    while (!Serial) {
        delay(33);  // Do not exit setup until Serial has success.
    }
     */
}

void loop() {
    if (alarm_set == true) {
        alarm_set = false;
        oled.enable();
        WiFi.begin(WIFI_SSID, WIFI_PASS);

    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println(ipv4_int_to_str(WiFi.localIP()));

        time_util.set_time_of_day();

        Serial.print("NTP: "); Serial.println(time_util.unix_epoch_time_gmt);
        Serial.print("Sensor Time: "); Serial.println(sensor.unix_epoch_time);

        if (mqtt_connect_status != 0) {
            mqtt_connect_status = mqtt.connect();
        }

        sensor.battery_vdc = get_battery_vdc();
        sensor.current_ma = ina219.getCurrent_mA();
        if (std::isnan(sensor.current_ma) or sensor.current_ma < 0) {
            Serial.println("Error reading current sensor!");
        } else {
            Serial.print("Current : ");
            Serial.print(sensor.current_ma);
            Serial.println("mA");
        }

        sensors_event_t event;
        dht.temperature().getEvent(&event);
        if (std::isnan(event.temperature)) {
            Serial.println("Error reading temperature!");
        } else {
            sensor.temperature_f = event.temperature * 1.8 + 32;
            Serial.print("Temperature: ");
            Serial.print(sensor.temperature_f);
            Serial.println(" ℉");
        };

        dht.humidity().getEvent(&event);
        if (std::isnan(event.relative_humidity)) {
            Serial.println("Error reading humidity!");
        } else {
            sensor.humidity_rh = event.relative_humidity;
            Serial.print("Relative Humidity: ");
            Serial.print(sensor.humidity_rh);
            Serial.println(" ϕ");
        }

        char payload_float[AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE];  // Float max 39 digits and minus sign.
        char zero{'\0'};
        std::bitset<5> publish_status{0};
        if (mqtt_connect_status == 0) {
            // Publish Battery VDC Percent
            dtostrf(sensor.battery_vdc, 0, AIO_FLOAT_PRECISION, payload_float);
            publish_status[0] = mqtt.publish(BATTERY_VDC, payload_float, 0);
            memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

            // Publish Current mA
            dtostrf(sensor.current_ma, 0, AIO_FLOAT_PRECISION, payload_float);
            publish_status[1] = mqtt.publish(CURRENT_MA, payload_float, 0);
            memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

            // Publish Humidity ϕ
            dtostrf(sensor.humidity_rh, 0, AIO_FLOAT_PRECISION, payload_float);
            publish_status[2] = mqtt.publish(HUMIDITY_RH, payload_float, 0);
            memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

            // Publish Temperature ℉
            dtostrf(sensor.temperature_f, 0, AIO_FLOAT_PRECISION, payload_float);
            publish_status[3] = mqtt.publish(TEMPERATURE_F, payload_float, 0);
            memcpy(&payload_float, &zero, AIO_MQTT_PAYLOAD_FLOAT_MAX_SIZE);

            // Publish Unix Epoch Time UTC
            publish_status[4] = mqtt.publish(UNIX_EPOCH_TIME, sensor.unix_epoch_time, 0);
        }

        Serial.print("Loop "); Serial.print(loop_counter); Serial.print(" of ");
        Serial.println(LOOP_LIMIT);
        if (display_data) {
            oled.show_page(oled.page);
            delay(5900);
            loop_counter++;
            if (loop_counter > LOOP_LIMIT) {
                Serial.println("Display work is exhausting. Tired now. Going to sleep. ZZZzzz...");
                monitor_deep_sleep();
            }
        } else if (publish_status != 0) {  // At least one item was published.
            delay(100);
            Serial.print("Publish status battery: ");
            Serial.println(publish_status[0]);
            Serial.print("Publish status current: ");
            Serial.println(publish_status[1]);
            Serial.print("Publish status humidity: ");
            Serial.println(publish_status[2]);
            Serial.print("Publish status temperature: ");
            Serial.println(publish_status[3]);
            Serial.print("Publish status time: ");
            Serial.println(publish_status[4]);
            Serial.println("No display work. Going back to sleep. ZZZzzz...");
            monitor_deep_sleep();
        }
    } else {
        uint8_t mac_address;
        WiFi.macAddress(&mac_address);
        Serial.println("WiFi is not connected.");
        Serial.print("MAC Address: ");
        Serial.println(mac_address);
        wifi_connection_attempts++;
        Serial.print("Connection attempt #");
        Serial.print(wifi_connection_attempts);
        Serial.println(" failed.");
        if (wifi_connection_attempts == 10) {
            monitor_deep_sleep();
        }
        delay(300);
    }
}

void alarm_handler() {
    alarm_set = true;
}


void monitor_deep_sleep() {
    pinMode(PIN_LED_13, LOW);
    oled.disable();
    mqtt.disconnect();
    mqtt_connect_status = -1;
    client.stop();
    WiFi.end();

    Serial.print("RTC Epoch: "); Serial.println(rtc.getEpoch());
    Serial.print("RTC Alarm: "); Serial.println(time_util.unix_epoch_time_gmt + SLEEP_INTERVAL_SECONDS);
    rtc.setAlarmEpoch((uint32)time_util.unix_epoch_time_gmt + SLEEP_INTERVAL_SECONDS);
    rtc.enableAlarm(rtc.MATCH_HHMMSS);
    rtc.standbyMode();
}