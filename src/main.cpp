/*
   Smart Solartracker für ESP32
   Modus: Access Point (AP)
   --------------------------------
   - ESP32 erstellt eigenes WLAN
   - Handy/Tablet verbindet sich darauf
   - über HTTP-API kann Regen/Wetter gesetzt werden
   - 4 LDRs, 2 Servos (Yaw unten, Tilt oben)
   - Start-Einrichtung
   - Sleep bei Nacht/Wolken/Regen
   - einfacher Software-Watchdog
*/

#include "st_debug.h"
#include "config.h"
#include "time_rtc.h"
#include "st_config.h"
#include "st_helpers.h"
#include "st_calibration.h"
#include "st_init.h"
#include "st_webserver.h"

// =====================================================
// SERVO OBJECTS
// =====================================================
Servo servoYaw;
Servo servoTilt;

// =====================================================
// RUNTIME STATE - Current Positions
// =====================================================
int yawAngle = 90;
int tiltAngle = 90;

// =====================================================
// RUNTIME STATE - Timing
// =====================================================
unsigned long lastRead = 0;
unsigned long lastSerialLog = 0;
unsigned long lastAlive = 0;

// =====================================================
// RUNTIME STATE - Watchdog
// =====================================================
uint8_t watchdogHits = 0;
unsigned long lastWatchdogReset = 0;

// =====================================================
// RUNTIME STATE - Weather & Tracking
// =====================================================
bool weatherSaysStop = false;
TrackerState trackerState = STATE_INIT;
TrackerState lastLoggedState = STATE_INIT;

// =====================================================
// RUNTIME STATE - Transition Counters
// =====================================================
uint8_t darkCount = 0;
uint8_t lightCount = 0;

// =====================================================
// RUNTIME STATE - Position History
// =====================================================
int historyYaw[POS_HISTORY_SIZE];
int historyTilt[POS_HISTORY_SIZE];
int historyIndex = 0;
bool haveHistory = false;

// =====================================================
// FILESYSTEM STATE
// =====================================================
bool littlefsAvailable = false;
unsigned long lastHistoryWrite = 0;

// =====================================================
// SETUP
// =====================================================
void setup()
{
    DEBUG_INIT(115200);
    delay(500);
    DEBUG_PRINTLN("Smart Solartracker (AP-Modus) startet...");

    littlefsAvailable = LittleFS.begin(true);
    if (!littlefsAvailable)
    {
        DEBUG_PRINTLN("⚠ LittleFS Mount fehlgeschlagen!");
    }
    else
    {
        DEBUG_PRINTLN("✓ LittleFS erfolgreich gemountet");
        loadDebugConfig();
        loadGeoConfig();
        loadWifiConfig();
        if (!LittleFS.exists("/history.csv"))
        {
            File file = LittleFS.open("/history.csv", "w");
            if (file)
            {
                file.println("Timestamp,Yaw,Tilt,SunAzimuth,SunAltitude");
                file.close();
                DEBUG_PRINTLN("history.csv erstellt.");
            }
        }
    }

    initRTC();

    servoYaw.attach(SERVO_YAW_PIN);
    servoTilt.attach(SERVO_TILT_PIN);

    DEBUG_PRINTLN("Starte WLAN im AP+Station Modus...");
    WiFi.mode(WIFI_AP_STA);

    DEBUG_PRINT("Starte AP: ");
    DEBUG_PRINTLN(AP_SSID);
    bool apOk = WiFi.softAP(AP_SSID, AP_PASS);
    if (apOk)
    {
        DEBUG_PRINT("AP gestartet. IP: ");
        DEBUG_PRINTLN(WiFi.softAPIP());
    }
    else
    {
        DEBUG_PRINTLN("AP konnte nicht gestartet werden!");
    }

    DEBUG_PRINT("Verbinde mit WLAN: ");
    DEBUG_PRINTLN(staSsid);
    WiFi.begin(staSsid, staPass);

    int wifiTimeout = 0;
    while (WiFi.status() != WL_CONNECTED && wifiTimeout < 20)
    {
        delay(500);
        DEBUG_PRINT(".");
        wifiTimeout++;
    }
    DEBUG_PRINTLN();

    if (WiFi.status() == WL_CONNECTED)
    {
        DEBUG_PRINT("WLAN verbunden! IP: ");
        DEBUG_PRINTLN(WiFi.localIP());

        // Prefer internet time when available
        DEBUG_PRINTLN("Synchronisiere RTC mit NTP...");
        if (!syncRTCFromNTP())
        {
            DEBUG_PRINTLN("⚠ NTP-Sync fehlgeschlagen, nutze RTC falls verfügbar.");
        }
    }
    else
    {
        DEBUG_PRINTLN("WLAN-Verbindung fehlgeschlagen!");
        if (rtcAvailable)
        {
            DEBUG_PRINTLN("Tracker laeuft mit RTC-Zeit.");
        }
        else
        {
            DEBUG_PRINTLN("Tracker laeuft OHNE Zeit/Smart-Tracking.");
        }
    }

    server.on("/", handleRoot);
    server.on("/status", handleStatus);
    server.on("/weather", handleWeather);
    server.on("/history", handleHistory);
    server.on("/geo", handleGeoConfig);
    server.on("/geo/save", HTTP_POST, handleGeoSave);
    server.on("/wifi", handleWifiConfig);
    server.on("/wifi/save", HTTP_POST, handleWifiSave);
    server.on("/calibrate", handleCalibrate);
    server.on("/calibrate/start", handleCalibrateStart);
    server.on("/debug", handleDebug);
    server.on("/debug/set", handleDebugSet);
    server.onNotFound(handleNotFound);
    server.begin();
    DEBUG_PRINTLN("HTTP-Server laeuft.");

    trackerState = STATE_INIT;
    lastAlive = millis();
}

// =====================================================
// LOOP
// =====================================================
void loop()
{
    server.handleClient();
    lastAlive = millis();

    checkWatchdog();

    unsigned long now = millis();

    switch (trackerState)
    {

    case STATE_INIT:
        doStartInit();
        break;

    case STATE_TRACKING:
    {
        updateTime();
        if (timeInitialized)
        {
            calculateSunPosition();
        }

        if (now - lastRead >= READ_INTERVAL)
        {
            lastRead = now;

            int tl = analogRead(LDR_TL_PIN);
            int tr = analogRead(LDR_TR_PIN);
            int bl = analogRead(LDR_BL_PIN);
            int br = analogRead(LDR_BR_PIN);

            bool dark = isDark(tl, tr, bl, br);
            bool cloudy = isCloudy(tl, tr, bl, br);

            if (dark || cloudy || weatherSaysStop)
            {
                darkCount++;
                lightCount = 0;
                if (darkCount >= STATE_CHANGE_THRESHOLD)
                {
                    DEBUG_PRINTLN("-> Zu dunkel / Wolken / Web-Stop -> SLEEP");
                    yawAngle = 90;
                    tiltAngle = 70;
                    servoYaw.write(yawAngle);
                    servoTilt.write(tiltAngle);
                    pushHistory(yawAngle, tiltAngle, true);
                    trackerState = STATE_SLEEP;
                    lastLoggedState = STATE_SLEEP;
                    darkCount = 0;
                    break;
                }
            }
            else
            {
                darkCount = 0;
            }

            int targetYaw = yawAngle;
            int targetTilt = tiltAngle;

            if (timeInitialized && currentSunAltitude > 0)
            {
                targetYaw = targetYawFromSun;
                targetTilt = targetTiltFromSun;
            }

            int topSum = tl + tr;
            int bottomSum = bl + br;
            int leftSum = tl + bl;
            int rightSum = tr + br;

            int diffYaw = leftSum - rightSum;
            int diffTilt = topSum - bottomSum;
            int totalSum = topSum + bottomSum;

            if (totalSum < MIN_LIGHT_SUM)
            {
                DEBUG_PRINTLN("TRACK: Zu wenig Licht für LDR-Tracking, verwende nur Sonnenposition.");
                if (timeInitialized && currentSunAltitude > 0)
                {
                    yawAngle = targetYaw;
                    tiltAngle = targetTilt;
                    servoYaw.write(yawAngle);
                    servoTilt.write(tiltAngle);
                }
                break;
            }

            float errorYaw = (float)diffYaw / totalSum;
            float errorTilt = (float)diffTilt / totalSum;

            int prevYaw = yawAngle;
            int prevTilt = tiltAngle;

            if (timeInitialized && currentSunAltitude > 0)
            {
                if (yawAngle < targetYaw)
                    yawAngle++;
                else if (yawAngle > targetYaw)
                    yawAngle--;

                if (tiltAngle < targetTilt)
                    tiltAngle++;
                else if (tiltAngle > targetTilt)
                    tiltAngle--;
            }

            if (abs(errorYaw) > DEAD_BAND_YAW)
            {
                yawAngle += (errorYaw > 0 ? 1 : -1) * YAW_DIR * STEP_YAW;
            }

            if (abs(errorTilt) > DEAD_BAND_TILT)
            {
                tiltAngle += (errorTilt > 0 ? 1 : -1) * TILT_DIR * STEP_TILT;
            }

            yawAngle = constrain(yawAngle, YAW_MIN, YAW_MAX);
            tiltAngle = constrain(tiltAngle, TILT_MIN, TILT_MAX);

            if (prevYaw != yawAngle)
            {
                servoYaw.write(yawAngle);
            }
            if (prevTilt != tiltAngle)
            {
                servoTilt.write(tiltAngle);
            }

            pushHistory(yawAngle, tiltAngle);
            lastAlive = millis();

            if (now - lastSerialLog >= SERIAL_LOG_INTERVAL)
            {
                lastSerialLog = now;
                DEBUG_PRINT("TRACK Yaw: ");
                DEBUG_PRINT(yawAngle);
                DEBUG_PRINT("  Tilt: ");
                DEBUG_PRINT(tiltAngle);
                DEBUG_PRINT("  Target: ");
                DEBUG_PRINT(targetYaw);
                DEBUG_PRINT("/");
                DEBUG_PRINT(targetTilt);
                DEBUG_PRINT("  LuxSum: ");
                DEBUG_PRINTLN(topSum + bottomSum);
            }
        }
        break;
    }

    case STATE_SLEEP:
    {
        if (now - lastRead >= READ_INTERVAL)
        {
            lastRead = now;

            int tl = analogRead(LDR_TL_PIN);
            int tr = analogRead(LDR_TR_PIN);
            int bl = analogRead(LDR_BL_PIN);
            int br = analogRead(LDR_BR_PIN);

            bool dark = isDark(tl, tr, bl, br);
            bool cloudy = isCloudy(tl, tr, bl, br);

            if (!dark && !cloudy && !weatherSaysStop)
            {
                lightCount++;
                darkCount = 0;
                if (lightCount >= STATE_CHANGE_THRESHOLD)
                {
                    DEBUG_PRINTLN("-> Licht wieder ok, zurück zu TRACKING");
                    if (haveHistory)
                    {
                        int lastIdx = (historyIndex - 1 + POS_HISTORY_SIZE) % POS_HISTORY_SIZE;
                        yawAngle = historyYaw[lastIdx];
                        tiltAngle = historyTilt[lastIdx];
                        servoYaw.write(yawAngle);
                        servoTilt.write(tiltAngle);
                    }
                    pushHistory(yawAngle, tiltAngle, true);
                    trackerState = STATE_TRACKING;
                    lastLoggedState = STATE_TRACKING;
                    lightCount = 0;
                }
            }
            else
            {
                lightCount = 0;
            }
        }
        break;
    }
    }

    lastAlive = millis();
}
