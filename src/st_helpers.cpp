#include "st_debug.h"
#include "config.h"
#include "st_helpers.h"
#include "time_rtc.h"

#define HISTORY_WRITE_MIN_MS (5 * 60 * 1000)

// External references from main.cpp
extern int historyYaw[];
extern int historyTilt[];
extern int historyIndex;
extern bool haveHistory;
extern bool littlefsAvailable;
extern unsigned long lastHistoryWrite;
extern Servo servoYaw;
extern Servo servoTilt;
extern int yawAngle;
extern int tiltAngle;
extern uint8_t watchdogHits;
extern unsigned long lastWatchdogReset;
extern unsigned long lastAlive;
extern unsigned long lastRead;
extern TrackerState trackerState;

void pushHistory(int y, int t, bool forceWrite)
{
    historyYaw[historyIndex] = y;
    historyTilt[historyIndex] = t;
    historyIndex++;
    if (historyIndex >= POS_HISTORY_SIZE)
        historyIndex = 0;
    haveHistory = true;

    if (!littlefsAvailable)
        return;

    unsigned long now = millis();
    if (!forceWrite && (now - lastHistoryWrite) < HISTORY_WRITE_MIN_MS)
        return;

    File file = LittleFS.open("/history.csv", "a");
    if (file)
    {
        String timestamp;
        if (timeInitialized)
        {
            DateTime now = getCurrentTime();
            char buffer[30];
            sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
                    now.year(), now.month(), now.day(),
                    now.hour(), now.minute(), now.second());
            timestamp = String(buffer);
        }
        else
        {
            timestamp = String(millis());
        }
        file.print(timestamp + "," + String(y) + "," + String(t));
        if (timeInitialized)
        {
            file.println("," + String(currentSunAzimuth, 2) + "," + String(currentSunAltitude, 2));
        }
        else
        {
            file.println(",,");
        }
        file.close();
        lastHistoryWrite = now;
    }
}

bool isDark(int tl, int tr, int bl, int br)
{
    return (tl < NIGHT_THRESHOLD &&
            tr < NIGHT_THRESHOLD &&
            bl < NIGHT_THRESHOLD &&
            br < NIGHT_THRESHOLD);
}

bool isCloudy(int tl, int tr, int bl, int br)
{
    int avg = (tl + tr + bl + br) / 4;
    return (avg < CLOUD_THRESHOLD);
}

void checkWatchdog()
{
    unsigned long now = millis();
    if (now - lastAlive > WATCHDOG_TIMEOUT)
    {
        watchdogHits++;

        if ((now - lastWatchdogReset) > WATCHDOG_RESET_WINDOW_MS)
        {
            watchdogHits = 0;
            lastWatchdogReset = now;
        }

        if (watchdogHits >= WATCHDOG_MAX_HITS)
        {
            DEBUG_PRINTLN("⚠ Watchdog: mehrfach ausgelöst -> System-Neustart");
            delay(100);
            ESP.restart();
        }

        DEBUG_PRINTLN("⚠ Watchdog: Loop hängt -> Grundstellung & Re-Init");
        yawAngle = 90;
        tiltAngle = 90;
        servoYaw.write(yawAngle);
        servoTilt.write(tiltAngle);
        trackerState = STATE_INIT;
        lastAlive = now;
        lastRead = now;
    }
}
