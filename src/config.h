#ifndef ST_CONFIG_H
#define ST_CONFIG_H

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <LittleFS.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <time.h>
#include <SolarCalculator.h>
#include <RTClib.h>
#include <Wire.h>

#include "tracker_state.h"

// =====================================================
// WLAN AP KONFIG
// =====================================================
extern const char *AP_SSID;
extern const char *AP_PASS;
extern const char *API_KEY;

// WLAN Station Konfig (für Internet/NTP)
extern char staSsid[64];
extern char staPass[64];

// WiFi connection parameters
extern const int WIFI_CONNECT_RETRIES;         // Number of retry attempts for WiFi connection
extern const unsigned long WIFI_RETRY_DELAY_MS; // Delay between WiFi connection retries (ms)

// =====================================================
// ZEIT & GEO KONFIG
// =====================================================
extern const double DEFAULT_LATITUDE;
extern const double DEFAULT_LONGITUDE;
extern double currentLatitude;
extern double currentLongitude;

// Timezone configuration
extern bool useAutoDST;                          // Automatic DST calculation (EU rules)
extern int manualTimezoneOffset;                 // Manual timezone offset in seconds (UTC+1 = 3600)
extern const unsigned long NTP_UPDATE_INTERVAL; // Interval for NTP time synchronization (ms)

// =====================================================
// STARTUP & TIMING
// =====================================================
extern const unsigned long START_INIT_DELAY_MS; // Initial delay during startup initialization (ms)

// =====================================================
// LDR PINS
// =====================================================
extern const int LDR_TL_PIN;
extern const int LDR_TR_PIN;
extern const int LDR_BL_PIN;
extern const int LDR_BR_PIN;

// =====================================================
// SERVO PINS
// =====================================================
extern const int SERVO_YAW_PIN;
extern const int SERVO_TILT_PIN;

extern const int SERVO_MIN;
extern const int SERVO_MAX;

// Mechanische Limits (zum Schutz der Hardware)
extern const int YAW_MIN;
extern const int YAW_MAX;
extern const int TILT_MIN;
extern const int TILT_MAX;

// Richtungs-Flags für Kalibrierung (bei Bedarf auf -1 setzen)
extern const int YAW_DIR;
extern const int TILT_DIR;

// =====================================================
// TRACKING PARAMETER
// =====================================================
extern const float DEAD_BAND_YAW;
extern const float DEAD_BAND_TILT;
extern const int STEP_YAW;
extern const int STEP_TILT;
extern const int MIN_LIGHT_SUM;

extern const unsigned long READ_INTERVAL;
extern const unsigned long SERIAL_LOG_INTERVAL;

// Licht-/Wetterlogik
extern const int NIGHT_THRESHOLD;
extern const int CLOUD_THRESHOLD;

// Watchdog
extern const unsigned long WATCHDOG_TIMEOUT;         // Max time without main loop activity (ms)
extern const unsigned long WATCHDOG_RESET_WINDOW_MS; // Time window for counting watchdog hits (ms)
extern const uint8_t WATCHDOG_MAX_HITS;              // Max consecutive watchdog hits before restart

// Hysterese für Zustandswechsel
extern const uint8_t STATE_CHANGE_THRESHOLD;

// History
#define POS_HISTORY_SIZE 10  // Must be #define for compile-time array sizing
extern const unsigned long HISTORY_WRITE_MIN_MS; // Minimum time between history writes to protect flash (ms)

// Module function declarations
void loadGeoConfig(void);
void saveGeoConfig(void);
void loadWifiConfig(void);
void saveWifiConfig(void);

#endif
