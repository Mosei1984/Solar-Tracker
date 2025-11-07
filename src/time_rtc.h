#ifndef ST_TIME_RTC_H
#define ST_TIME_RTC_H

#include "config.h"

// RTC und NTP Objekte
extern RTC_DS3231 rtc;
extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

// Zeit-Synchronisations-Flags
extern bool timeInitialized;
extern bool rtcAvailable;
extern unsigned long lastNTPUpdate;

// Sonnenpositions-Variablen
extern double currentSunAzimuth;
extern double currentSunAltitude;
extern int targetYawFromSun;
extern int targetTiltFromSun;

// Funktionen
void initRTC();
bool syncRTCFromNTP();
DateTime getCurrentTime();
void updateTime();
void calculateSunPosition();
void sunPositionToServoAngles(float azimuth, float altitude);
bool rtcLostPower();
bool isEUDaylightSavingTime(int year, int month, int day, int hour);
int getCurrentTimezoneOffset();

#endif
