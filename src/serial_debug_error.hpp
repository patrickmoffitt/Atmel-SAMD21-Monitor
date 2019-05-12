//
// Created by Patrick Moffitt on 2019-05-11.
//

// Uncomment/comment to turn on/off debug output messages.
// #define SERIAL_DEBUG
// Uncomment/comment to turn on/off error output messages.
// #define SERIAL_ERROR

// Set where debug messages will be printed.
#define DEBUG_PRINTER Serial
// If using something like Zero or Due, change the above to SerialUSB

// Define actual debug output functions when necessary.
#ifdef SERIAL_DEBUG
    #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
    #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
    #define DEBUG_PRINTBUFFER(buffer, len) { printBuffer(buffer, len); }
#else
    #define DEBUG_PRINT(...) {}
    #define DEBUG_PRINTLN(...) {}
    #define DEBUG_PRINTBUFFER(buffer, len) {}
#endif

#ifdef SERIAL_ERROR
    #define ERROR_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
    #define ERROR_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
    #define ERROR_PRINTBUFFER(buffer, len) { printBuffer(buffer, len); }
#else
    #define ERROR_PRINT(...) {}
    #define ERROR_PRINTLN(...) {}
    #define ERROR_PRINTBUFFER(buffer, len) {}
#endif
