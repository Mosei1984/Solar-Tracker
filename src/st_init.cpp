#include "st_debug.h"
#include "st_init.h"
#include "config.h"
#include "time_rtc.h"
#include "st_calibration.h"
#include "st_helpers.h"

// External references from main.cpp
extern Servo servoYaw;
extern Servo servoTilt;
extern int yawAngle;
extern int tiltAngle;
extern TrackerState trackerState;
extern unsigned long lastAlive;

void doStartInit()
{
    DEBUG_PRINTLN("Starte Initialisierung...");

    int calibYaw = 90;
    int calibTilt = 90;
    loadCalibrationData(calibYaw, calibTilt);

    updateTime();

    if (timeInitialized)
    {
        calculateSunPosition();

        if (currentSunAltitude > 0)
        {
            yawAngle = targetYawFromSun;
            tiltAngle = targetTiltFromSun;
            DEBUG_PRINT("Smart Start -> Sonnenposition: Azimut=");
            DEBUG_PRINT(currentSunAzimuth);
            DEBUG_PRINT("° Altitude=");
            DEBUG_PRINT(currentSunAltitude);
            DEBUG_PRINT("° -> Yaw=");
            DEBUG_PRINT(yawAngle);
            DEBUG_PRINT("° Tilt=");
            DEBUG_PRINTLN(tiltAngle);
        }
        else
        {
            yawAngle = calibYaw;
            tiltAngle = calibTilt;
            DEBUG_PRINTLN("Sonne unter Horizont -> Verwende kalibrierte Position");
        }
    }
    else
    {
        yawAngle = calibYaw;
        tiltAngle = calibTilt;
        DEBUG_PRINTLN("Zeit nicht verfügbar -> Verwende kalibrierte Position");
    }

    servoYaw.write(yawAngle);
    servoTilt.write(tiltAngle);
    delay(300);

    int tl = analogRead(LDR_TL_PIN);
    int tr = analogRead(LDR_TR_PIN);
    int bl = analogRead(LDR_BL_PIN);
    int br = analogRead(LDR_BR_PIN);

    DEBUG_PRINT("Initial LDR: ");
    DEBUG_PRINT(tl);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(tr);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(bl);
    DEBUG_PRINT(" ");
    DEBUG_PRINTLN(br);

    pushHistory(yawAngle, tiltAngle);
    lastAlive = millis();

    trackerState = STATE_TRACKING;
    DEBUG_PRINTLN("Init fertig -> TRACKING.");
}
