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

#include "ntp_time_utils.hpp"

extern bool system_time_set;
extern RTCZero rtc;
// Declared in WiFi101.h
extern WiFiClass WiFi;

/*
 * DST starts in March and ends in November.
 * Dates per https://www.timeanddate.com/time/change/usa/washington-dc
 */
const std::map<int, std::pair<int, int>> ntp_time_utils::dst_dates {
        {2018, {11, 4}},
        {2019, {10, 3}},
        {2020, {8, 1}},
        {2021, {14, 7}},
        {2022, {13, 6}},
        {2023, {12, 5}},
        {2024, {10, 3}},
        {2025, {9, 2}},
        {2026, {8, 1}},
        {2027, {14, 7}},
        {2028, {12, 5}},
        {2029, {11, 4}}
};

/*
 * Set timezone abbreviation and daylight savings time offset.
 */
void ntp_time_utils::set_dst_usa(tm *time_o, time_t *time_stamp) {
    int year = time_o->tm_year + 1900;
    if (year > 2017 and year < 2030) {
        std::pair<int, int> dst = dst_dates.at(year);
        double dst_start_hour = ((31 + 28.25 + std::get<0>(dst) - 1) * 24) + 2;
        double dst_end_hour = ((31 + 28.25 + 31 + 30 + 31 + 30 + 31 + 31 +
                                30 + 31 + std::get<1>(dst) - 1) * 24) + 2;
        double hours_since_new_year = (*time_stamp - (31557600 * (year - 1970.0))) / 3600.0;
        if (dst_start_hour < hours_since_new_year and dst_end_hour > hours_since_new_year) {
            memcpy(&EASTERN_TIMEZONE_ABBREV, " EDT", sizeof(EASTERN_TIMEZONE_ABBREV));
            dst_offset_seconds = 3600;
        }
    }
}

/*
 * Set time using time from ATWINC1500 on the Feather M0.
 * Fallback to whatever time is on the RTC.
 */
bool ntp_time_utils::set_time_of_day() {
    uint32_t time{0};
    if (not system_time_set and WiFi.status() == WL_CONNECTED) {
        time = WiFi.getTime();
        if (time != 0) {
            unix_epoch_time_gmt = time;
            rtc.setEpoch(unix_epoch_time_gmt);
            system_time_set = true;
        }
    } else {
        unix_epoch_time_gmt = rtc.getEpoch();
    }
    set_sensor_time(unix_epoch_time_gmt);
    return system_time_set;
}

/*
 * Set the time string in sensor.unix_epoch_time
 */
void ntp_time_utils::set_sensor_time(time_t unix_epoch_time_gmt){
    extern monitor_data sensor;
    time_t now = unix_epoch_time_gmt;
    now += (GMT_OFFSET * 3600);
    tm time_info{};
    gmtime_r(&now, &time_info);
    set_dst_usa(&time_info, &now);
    now += dst_offset_seconds;
    std::strftime(
                  sensor.unix_epoch_time,
                  sizeof(sensor.unix_epoch_time),
                  "%c",
                  std::gmtime(&now)
    );
    strcat(&sensor.unix_epoch_time[sizeof(sensor.unix_epoch_time) -
                                   sizeof(EASTERN_TIMEZONE_ABBREV)],
           EASTERN_TIMEZONE_ABBREV);
}