#include "st_debug.h"
#include "time_rtc.h"

// RTC und NTP Objekte
RTC_DS3231 rtc;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);

// Zeit-Synchronisations-Flags
bool timeInitialized = false;
bool rtcAvailable = false;
unsigned long lastNTPUpdate = 0;

// Sonnenpositions-Variablen
double currentSunAzimuth = 0.0;
double currentSunAltitude = 0.0;
int targetYawFromSun = 90;
int targetTiltFromSun = 90;

void initRTC()
{
    Wire.begin();
    
    if (!rtc.begin())
    {
        Serial.println("DS3231 RTC nicht gefunden!");
        rtcAvailable = false;
        return;
    }
    
    rtcAvailable = true;
    Serial.println("DS3231 RTC gefunden!");
    
    if (rtc.lostPower())
    {
        Serial.println("RTC hat Stromversorgung verloren, Zeit wird ungültig sein.");
        Serial.println("Versuche NTP-Synchronisation...");
    }
    else
    {
        timeInitialized = true;
        DateTime now = rtc.now();
        Serial.print("RTC-Zeit: ");
        Serial.print(now.year());
        Serial.print("-");
        Serial.print(now.month());
        Serial.print("-");
        Serial.print(now.day());
        Serial.print(" ");
        Serial.print(now.hour());
        Serial.print(":");
        Serial.print(now.minute());
        Serial.print(":");
        Serial.println(now.second());
    }
}

bool syncRTCFromNTP()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Serial.println("NTP Sync fehlgeschlagen: WiFi nicht verbunden");
        return false;
    }
    
    static bool ntpBegun = false;
    if (!ntpBegun)
    {
        timeClient.begin();
        ntpBegun = true;
    }
    
    if (timeClient.update())
    {
        unsigned long epochTime = timeClient.getEpochTime();
        
        if (epochTime >= 1577836800UL)
        {
            int offset = getCurrentTimezoneOffset();
            epochTime += offset;
            
            if (rtcAvailable)
            {
                rtc.adjust(DateTime(epochTime));
                Serial.print("RTC mit NTP synchronisiert (");
            }
            else
            {
                Serial.print("Zeit via NTP synchronisiert (ohne RTC) (");
            }
            Serial.print(offset == 7200 ? "MESZ" : "MEZ");
            Serial.print("): ");
            timeInitialized = true;
            lastNTPUpdate = millis();
            DateTime localTime = DateTime(epochTime);
            char buffer[25];
            sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
                    localTime.year(), localTime.month(), localTime.day(),
                    localTime.hour(), localTime.minute(), localTime.second());
            Serial.println(buffer);
            return true;
        }
        else
        {
            Serial.println("NTP Sync fehlgeschlagen: Ungueltige Epochenzeit");
            return false;
        }
    }
    
    Serial.println("NTP Sync fehlgeschlagen: timeClient.update() fehlgeschlagen");
    return false;
}

DateTime getCurrentTime()
{
    if (rtcAvailable)
    {
        return rtc.now();
    }
    else if (timeInitialized && WiFi.status() == WL_CONNECTED)
    {
        timeClient.update();
        unsigned long epochTime = timeClient.getEpochTime();
        return DateTime((uint32_t)epochTime);
    }
    else
    {
        return DateTime(1970, 1, 1, 0, 0, 0);
    }
}

void updateTime()
{
    if (rtcAvailable && !timeInitialized)
    {
        if (!rtc.lostPower())
        {
            timeInitialized = true;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!timeInitialized || (millis() - lastNTPUpdate >= NTP_UPDATE_INTERVAL))
        {
            syncRTCFromNTP();
        }
    }
}

void calculateSunPosition()
{
    if (!timeInitialized)
        return;

    DateTime now = getCurrentTime();
    
    calcHorizontalCoordinates(now.year(), now.month(), now.day(),
                               now.hour(), now.minute(), now.second(),
                               currentLatitude, currentLongitude,
                               currentSunAzimuth, currentSunAltitude);
    
    sunPositionToServoAngles(currentSunAzimuth, currentSunAltitude);
}

void sunPositionToServoAngles(float azimuth, float altitude)
{
    targetYawFromSun = map((int)azimuth, 0, 360, 0, 180);
    targetYawFromSun = constrain(targetYawFromSun, YAW_MIN, YAW_MAX);
    
    targetTiltFromSun = map((int)altitude, 0, 90, TILT_MIN, TILT_MAX);
    targetTiltFromSun = constrain(targetTiltFromSun, TILT_MIN, TILT_MAX);
}

bool rtcLostPower()
{
    return rtcAvailable && rtc.lostPower();
}

bool isEUDaylightSavingTime(int year, int month, int day, int hour)
{
    if (month < 3 || month > 10) return false;
    if (month > 3 && month < 10) return true;
    
    int lastSunday = 31 - ((5 * year / 4 + 4) % 7);
    
    if (month == 3)
    {
        if (day < lastSunday) return false;
        if (day > lastSunday) return true;
        return hour >= 2;
    }
    else
    {
        if (day < lastSunday) return true;
        if (day > lastSunday) return false;
        return hour < 3;
    }
}

int getCurrentTimezoneOffset()
{
    if (useAutoDST && timeInitialized)
    {
        DateTime now = getCurrentTime();
        bool isDST = isEUDaylightSavingTime(now.year(), now.month(), now.day(), now.hour());
        return isDST ? 7200 : 3600;
    }
    else
    {
        return manualTimezoneOffset;
    }
}
