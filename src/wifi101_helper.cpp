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

#include "wifi101_helper.hpp"

String ipv4_int_to_str(const uint32_t ip) {
    std::stringstream str_buf;
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    str_buf << (int) bytes[0] << '.';
    bytes[1] = (ip >> 8) & 0xFF;
    str_buf << (int) bytes[1] << ".";
    bytes[2] = (ip >> 16) & 0xFF;
    str_buf << (int) bytes[2] << ".";
    bytes[3] = (ip >> 24) & 0xFF;
    str_buf << (int) bytes[3] << std::ends;
    return String(str_buf.str().c_str());
}

String macv4_int_to_str(const uint8_t *mac) {
    std::stringstream str_buf;
    int i{0};
    for(i = 5; i >= 0; i--) {
        str_buf << std::hex << std::setw(2) << std::setfill('0') << (int) mac[i];
        if (i > 0)
            str_buf << ':';
    }
    str_buf << std::ends;
    return String(str_buf.str().c_str());
}

const char *wifi_state(const uint8_t key) {
    std::map<uint8_t, const char*> wifi_status = {
            {255, "WL_NO_SHIELD"},
            {0, "WL_IDLE_STATUS"},
            {1, "WL_NO_SSID_AVAIL"},
            {2, "WL_SCAN_COMPLETED"},
            {3, "WL_CONNECTED"},
            {4, "WL_CONNECT_FAILED"},
            {5, "WL_CONNECTION_LOST"},
            {6, "WL_DISCONNECTED"},
            {7, "WL_AP_LISTENING"},
            {8, "WL_AP_CONNECTED"},
            {9, "WL_AP_FAILED"},
            {10, "WL_PROVISIONING"},
            {11, "WL_PROVISIONING_FAILED"}
    };
    return wifi_status[key];
}
