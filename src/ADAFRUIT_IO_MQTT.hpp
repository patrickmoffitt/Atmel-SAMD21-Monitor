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

#ifndef MONITOR_ADAFRUIT_IO_MQTT_HPP
#define MONITOR_ADAFRUIT_IO_MQTT_HPP

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
static const char MQTT_SERVER[]   = AIO_SERVER;
static const char MQTT_CLIENTID[] = AIO_KEY __DATE__ __TIME__;
static const char MQTT_USERNAME[] = AIO_USERNAME;
static const char MQTT_PASSWORD[] = AIO_KEY;

// Define Feeds
static const char BATTERY_VDC[]     = AIO_USERNAME "/feeds/" AIO_GROUP_KEY ".battery-vdc";
static const char CURRENT_MA[]      = AIO_USERNAME "/feeds/" AIO_GROUP_KEY ".current-ma";
static const char HUMIDITY_RH[]     = AIO_USERNAME "/feeds/" AIO_GROUP_KEY ".humidity-rh";
static const char TEMPERATURE_F[]   = AIO_USERNAME "/feeds/" AIO_GROUP_KEY ".temperature-f";
static const char UNIX_EPOCH_TIME[] = AIO_USERNAME "/feeds/" AIO_GROUP_KEY ".unix-epoch-eastern";

#endif //MONITOR_ADAFRUIT_IO_MQTT_HPP
