#include "config.h"

// =====================================================
// WLAN & API CONFIG (Definitions)
// =====================================================
const char *AP_SSID = "SolarTracker_AP";
const char *AP_PASS = "SolarTracker";
const char *API_KEY = "key-123";
char staSsid[64] = "Hofer";
char staPass[64] = "MTLia21.01.21g";

// WiFi connection parameters
const int WIFI_CONNECT_RETRIES = 20;            // Number of retry attempts for WiFi connection
const unsigned long WIFI_RETRY_DELAY_MS = 500;  // Delay between WiFi connection retries (ms)

// =====================================================
// GEO CONFIG (Definitions)
// =====================================================
const double DEFAULT_LATITUDE = 52.5200;
const double DEFAULT_LONGITUDE = 13.4050;
double currentLatitude = 52.5200;
double currentLongitude = 13.4050;

// Timezone configuration
bool useAutoDST = true;                // Automatic EU DST calculation enabled by default
int manualTimezoneOffset = 3600;       // Manual offset: UTC+1 (3600 sec) - used when useAutoDST is false
const unsigned long NTP_UPDATE_INTERVAL = 3600000;  // Interval for NTP time sync (1 hour)

// =====================================================
// STARTUP & TIMING (Definitions)
// =====================================================
const unsigned long START_INIT_DELAY_MS = 300;  // Initial delay during startup initialization (ms)

// =====================================================
// LDR PINS (Definitions)
// =====================================================
const int LDR_TL_PIN = 34;
const int LDR_TR_PIN = 35;
const int LDR_BL_PIN = 32;
const int LDR_BR_PIN = 33;

// =====================================================
// SERVO CONFIG (Definitions)
// =====================================================
const int SERVO_YAW_PIN = 25;
const int SERVO_TILT_PIN = 26;

const int SERVO_MIN = 0;
const int SERVO_MAX = 180;

const int YAW_MIN = 5;
const int YAW_MAX = 175;
const int TILT_MIN = 15;
const int TILT_MAX = 165;

const int YAW_DIR = 1;
const int TILT_DIR = 1;

// =====================================================
// TRACKING PARAMETER (Definitions)
// =====================================================
const float DEAD_BAND_YAW = 0.01;
const float DEAD_BAND_TILT = 0.01;
const int STEP_YAW = 1;
const int STEP_TILT = 1;
const int MIN_LIGHT_SUM = 100;

const unsigned long READ_INTERVAL = 200;
const unsigned long SERIAL_LOG_INTERVAL = 5000;

const int NIGHT_THRESHOLD = 200;
const int CLOUD_THRESHOLD = 600;

const unsigned long WATCHDOG_TIMEOUT = 5000;              // Max time without main loop activity (ms)
const unsigned long WATCHDOG_RESET_WINDOW_MS = 600000;  // Time window for counting watchdog hits (10 min)
const uint8_t WATCHDOG_MAX_HITS = 3;                    // Max consecutive watchdog hits before restart

const uint8_t STATE_CHANGE_THRESHOLD = 3;

// POS_HISTORY_SIZE is now a #define in config.h (needed for compile-time array sizing)
const unsigned long HISTORY_WRITE_MIN_MS = 10000;  // Min time between history writes to protect flash (10 sec)
