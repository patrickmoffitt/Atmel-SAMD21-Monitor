//
// Created by Patrick Moffitt on 2019-05-09.
//

#include "float_to_fixed_width.hpp"

char *float_to_fixed_width::to_fixed_width (float number,
                                            signed char field_width,
                                            unsigned char precision,
                                            char *output) {
    if (precision > FLT_DIG)
        precision = FLT_DIG;
    char format[9];
    sprintf(format, "%%%d.%df", field_width, precision);
    sprintf(output, format, number);
    return output;
}

char *float_to_fixed_width::to_fixed_width (double number,
                                            signed char field_width,
                                            unsigned char precision,
                                            char *output) {
    if (precision > DBL_DIG)
        precision = DBL_DIG;
    char format[9];
    sprintf(format, "%%%d.%df", field_width, precision);
    sprintf(output, format, number);
    return output;
}


char *float_to_fixed_width::to_fixed_width (long double number,
                                            signed char field_width,
                                            unsigned char precision,
                                            char *output) {
    if (precision > LDBL_DIG)
        precision = LDBL_DIG;
    char format[10];
    sprintf(format, "%%%d.%dLf", field_width, precision);
    sprintf(output, format, number);
    return output;
}

char *float_to_fixed_width::to_fixed_width (int number,
                                            signed char field_width,
                                            char *output)  {
    sprintf(output, "%*d", field_width, number);
    return output;
}

std::string float_to_fixed_width::to_fixed_width (float number,
                                                  signed char field_width,
                                                  unsigned char precision) {
    unsigned char f_max_width = FLT_RADIX + FLT_DIG + 2;
    char f[f_max_width];
    to_fixed_width(number, field_width, precision, f);
    return std::string(f);
}

std::string float_to_fixed_width::to_fixed_width (double number,
                                                  signed char field_width,
                                                  unsigned char precision) {
    unsigned char d_max_width = FLT_RADIX + DBL_DIG + 2;
    char d[d_max_width];
    to_fixed_width(number, field_width, precision, d);
    return std::string(d);
}

std::string float_to_fixed_width::to_fixed_width (long double number,
                                                  signed char field_width,
                                                  unsigned char precision) {
    unsigned char ld_max_width = FLT_RADIX + LDBL_DIG + 2;
    char ld[ld_max_width];
    to_fixed_width(number, field_width, precision, ld);
    return std::string(ld);
}

std::string float_to_fixed_width::to_fixed_width (int number,
                                                  signed char field_width) {
    unsigned char int_max_width = MAX_INT32_DIG + 2;
    char s[int_max_width];
    to_fixed_width(number, field_width, s);
    return std::string(s);
}