#ifndef ST_DEBUG_H
#define ST_DEBUG_H

#include <Arduino.h>

// Runtime debug control - can be changed via API
extern bool debugEnabled;

// Debug functions
void loadDebugConfig();
void saveDebugConfig();
void setDebugEnabled(bool enabled);
bool isDebugEnabled();

// Debug macros - check runtime flag
#define DEBUG_INIT(baud) Serial.begin(baud)
#define DEBUG_PRINT(...) do { if (debugEnabled) Serial.print(__VA_ARGS__); } while(0)
#define DEBUG_PRINTLN(...) do { if (debugEnabled) Serial.println(__VA_ARGS__); } while(0)
#define DEBUG_PRINTF(fmt, ...) do { if (debugEnabled) Serial.printf(fmt, __VA_ARGS__); } while(0)

#endif
