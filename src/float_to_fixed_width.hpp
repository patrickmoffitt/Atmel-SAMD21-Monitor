//
// Created by Patrick Moffitt on 2019-05-09.
//

#ifndef SOLAR_POWER_MONITOR_FLOAT_TO_FIXED_WIDTH_HPP
#define SOLAR_POWER_MONITOR_FLOAT_TO_FIXED_WIDTH_HPP
#include "stdio.h"
#include "float.h"
#include <string>

#define MAX_INT32_DIG 10
#define ZERO_FIELD_PAD 0

struct float_to_fixed_width {

    static char *to_fixed_width (float number,
                                 signed char field_width,
                                 unsigned char precision,
                                 char *output);

    static char *to_fixed_width (double number,
                                 signed char field_width,
                                 unsigned char precision,
                                 char *output);

    static char *to_fixed_width (long double number,
                                 signed char field_width,
                                 unsigned char precision,
                                 char *output);

    static char *to_fixed_width (int number,
                                 signed char field_width,
                                 char *output);

    std::string to_fixed_width (float number,
                                signed char field_width,
                                unsigned char precision);

    std::string to_fixed_width (double number,
                                signed char field_width,
                                unsigned char precision);

    std::string to_fixed_width (long double number,
                                signed char field_width,
                                unsigned char precision);

    std::string to_fixed_width (int number,
                                signed char field_width);
};


#endif //SOLAR_POWER_MONITOR_FLOAT_TO_FIXED_WIDTH_HPP
