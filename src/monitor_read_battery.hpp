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

#ifndef MONITOR_MONITOR_READ_BATTERY_HPP
#define MONITOR_MONITOR_READ_BATTERY_HPP
#define ADC_PIN A0


#include <Arduino.h>
#undef min  // @ToDo remove when library? bug fixed.
#undef max  // @ToDo remove when library? bug fixed.
#include <array>
#include <algorithm>
#include "serial_debug_error.hpp"


// Analog read level is 10 bit 0-1023 (0V-1V).
// our 10MΩ & 2.2MΩ voltage divider takes the max
// lipo value of 4.49V and drops it to 811.
// this means our min analog read value should be 593 (3.29V).
// These are real, calibration, values as measured on the device.

#define MIN_BATTERY_VDC 3.29f

#define MAX_BATTERY_VDC 4.52f

#define ADC_10_BIT_CORRECTION 0.9990234375f  // 1 - (1/1024)
#define VDC_DIVISOR 180

float get_battery_vdc();
int get_battery_percent();
int map(float x, float in_min, float in_max, float out_min, float out_max);

#endif //MONITOR_MONITOR_READ_BATTERY_HPP
