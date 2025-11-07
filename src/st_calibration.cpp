#include "st_debug.h"
#include "st_calibration.h"
#include "config.h"

// External references from main.cpp
extern Servo servoYaw;
extern Servo servoTilt;
extern int yawAngle;
extern int tiltAngle;
extern bool littlefsAvailable;
extern WebServer server;
extern void pushHistory(int y, int t, bool forceWrite);

void saveCalibrationData(int yaw, int tilt)
{
    if (!littlefsAvailable)
        return;

    File f = LittleFS.open("/calibration.txt", "w");
    if (f)
    {
        f.print(yaw);
        f.print(",");
        f.println(tilt);
        f.close();
        DEBUG_PRINT("✓ Kalibrierung gespeichert: Yaw=");
        DEBUG_PRINT(yaw);
        DEBUG_PRINT("° Tilt=");
        DEBUG_PRINT(tilt);
        DEBUG_PRINTLN("°");
    }
}

void loadCalibrationData(int &yaw, int &tilt)
{
    yaw = 90;
    tilt = 90;

    if (!littlefsAvailable)
        return;

    if (LittleFS.exists("/calibration.txt"))
    {
        File f = LittleFS.open("/calibration.txt", "r");
        if (f)
        {
            String data = f.readString();
            f.close();

            int commaIndex = data.indexOf(',');
            if (commaIndex > 0)
            {
                yaw = data.substring(0, commaIndex).toInt();
                tilt = data.substring(commaIndex + 1).toInt();
                DEBUG_PRINT("✓ Kalibrierung geladen: Yaw=");
                DEBUG_PRINT(yaw);
                DEBUG_PRINT("° Tilt=");
                DEBUG_PRINT(tilt);
                DEBUG_PRINTLN("°");
            }
        }
    }
}

void startCalibration()
{
    DEBUG_PRINTLN("========================================");
    DEBUG_PRINTLN("Starte automatische Kalibrierung...");
    DEBUG_PRINTLN("========================================");

    int bestYaw = 90;
    int bestTilt = 90;
    int maxLightSum = 0;

    const int YAW_STEP = 10;
    const int TILT_STEP = 10;
    const int MEASUREMENT_DELAY = 300;

    for (int y = YAW_MIN; y <= YAW_MAX; y += YAW_STEP)
    {
        for (int t = TILT_MIN; t <= TILT_MAX; t += TILT_STEP)
        {
            servoYaw.write(y);
            servoTilt.write(t);
            delay(MEASUREMENT_DELAY);

            int tl = analogRead(LDR_TL_PIN);
            int tr = analogRead(LDR_TR_PIN);
            int bl = analogRead(LDR_BL_PIN);
            int br = analogRead(LDR_BR_PIN);
            int lightSum = tl + tr + bl + br;

            DEBUG_PRINT("Test Yaw=");
            DEBUG_PRINT(y);
            DEBUG_PRINT("° Tilt=");
            DEBUG_PRINT(t);
            DEBUG_PRINT("° -> LichtSum=");
            DEBUG_PRINTLN(lightSum);

            if (lightSum > maxLightSum)
            {
                maxLightSum = lightSum;
                bestYaw = y;
                bestTilt = t;
                DEBUG_PRINT("  ✓ Neues Maximum gefunden! Yaw=");
                DEBUG_PRINT(bestYaw);
                DEBUG_PRINT("° Tilt=");
                DEBUG_PRINT(bestTilt);
                DEBUG_PRINT("° Sum=");
                DEBUG_PRINTLN(maxLightSum);
            }

            server.handleClient();
        }
    }

    DEBUG_PRINTLN("========================================");
    DEBUG_PRINT("Kalibrierung abgeschlossen! Optimale Position: Yaw=");
    DEBUG_PRINT(bestYaw);
    DEBUG_PRINT("° Tilt=");
    DEBUG_PRINT(bestTilt);
    DEBUG_PRINT("° (Max Light=");
    DEBUG_PRINT(maxLightSum);
    DEBUG_PRINTLN(")");
    DEBUG_PRINTLN("========================================");

    saveCalibrationData(bestYaw, bestTilt);

    yawAngle = bestYaw;
    tiltAngle = bestTilt;
    servoYaw.write(yawAngle);
    servoTilt.write(tiltAngle);

    pushHistory(yawAngle, tiltAngle, true);
}
