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

#include <numeric>
#include "monitor_read_battery.hpp"

float get_battery_vdc() {
    // Read the battery level from the Feather M0 analog in pin.
    const size_t readings_len{30};
    std::array<int, readings_len> readings;
    std::fill_n(begin(readings), readings_len, 0);
    analogReadResolution(10);
    analogReference(AR_INTERNAL1V0);
    for (int i=0; i < (int) readings_len; i++) {
        readings[i] = analogRead(ADC_PIN);
        delay(33);
    }
    int sum = std::accumulate(begin(readings), end(readings), 0, std::plus<int>());
    int level = (sum / readings_len);
    DEBUG_PRINT("Raw ADC value: ");
    DEBUG_PRINTLN(level);
    return ((float) level * ADC_10_BIT_CORRECTION) / VDC_DIVISOR;
}

int get_battery_percent() {
    float vdc = get_battery_vdc();
    DEBUG_PRINT("Battery: "); DEBUG_PRINT(vdc); DEBUG_PRINTLN(" VDC");
    int level;
    // Convert battery level to percent.
    level = map(vdc, MIN_BATTERY_VDC, MAX_BATTERY_VDC, 0.0, 100.0);
    DEBUG_PRINT("Battery level: ");
    DEBUG_PRINT(level);
    DEBUG_PRINTLN("%");
    return level;
}

int map(float x, float in_min, float in_max, float out_min, float out_max) {
    float divisor = (in_max - in_min);
    if(divisor == 0){
        return -1; //AVR returns -1, SAM returns 0
    }
    return (x - in_min) * (out_max - out_min) / divisor + out_min;
}