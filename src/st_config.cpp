#include "st_debug.h"
#include "config.h"
#include "st_config.h"

// External references from main.cpp
extern bool littlefsAvailable;

// External references from config.cpp
extern double currentLatitude;
extern double currentLongitude;
extern bool useAutoDST;
extern int manualTimezoneOffset;
extern char staSsid[];
extern char staPass[];

void loadGeoConfig()
{
    if (!littlefsAvailable)
        return;

    if (LittleFS.exists("/geo.txt"))
    {
        File f = LittleFS.open("/geo.txt", "r");
        if (f)
        {
            String data = f.readString();
            f.close();

            int firstComma = data.indexOf(',');
            if (firstComma > 0)
            {
                currentLatitude = data.substring(0, firstComma).toDouble();
                int secondComma = data.indexOf(',', firstComma + 1);
                if (secondComma > 0)
                {
                    currentLongitude = data.substring(firstComma + 1, secondComma).toDouble();
                    int thirdComma = data.indexOf(',', secondComma + 1);
                    if (thirdComma > 0)
                    {
                        useAutoDST = (data.substring(secondComma + 1, thirdComma).toInt() == 1);
                        manualTimezoneOffset = data.substring(thirdComma + 1).toInt();
                    }
                }
                else
                {
                    currentLongitude = data.substring(firstComma + 1).toDouble();
                }
                DEBUG_PRINT("✓ Geo-Config geladen: ");
                DEBUG_PRINT(currentLatitude, 6);
                DEBUG_PRINT(", ");
                DEBUG_PRINT(currentLongitude, 6);
                DEBUG_PRINT(", AutoDST: ");
                DEBUG_PRINT(useAutoDST ? "AN" : "AUS");
                DEBUG_PRINT(", TZ: ");
                DEBUG_PRINTLN(manualTimezoneOffset / 3600.0);
            }
        }
    }
    else
    {
        DEBUG_PRINTLN("Nutze Default-Position");
    }
}

void saveGeoConfig()
{
    if (!littlefsAvailable)
        return;

    File f = LittleFS.open("/geo.txt", "w");
    if (f)
    {
        f.print(currentLatitude, 6);
        f.print(",");
        f.print(currentLongitude, 6);
        f.print(",");
        f.print(useAutoDST ? "1" : "0");
        f.print(",");
        f.println(manualTimezoneOffset);
        f.close();
        DEBUG_PRINTLN("✓ Geo-Config gespeichert");
    }
}

void loadWifiConfig()
{
    if (!littlefsAvailable)
        return;

    if (LittleFS.exists("/wifi.txt"))
    {
        File f = LittleFS.open("/wifi.txt", "r");
        if (f)
        {
            String data = f.readString();
            f.close();

            int firstComma = data.indexOf(',');
            if (firstComma > 0)
            {
                String ssid = data.substring(0, firstComma);
                String pass = data.substring(firstComma + 1);

                ssid.trim();
                pass.trim();

                ssid.toCharArray(staSsid, 64);
                pass.toCharArray(staPass, 64);

                DEBUG_PRINT("✓ WiFi-Config geladen: SSID=");
                DEBUG_PRINTLN(staSsid);
            }
        }
    }
    else
    {
        DEBUG_PRINTLN("Nutze Default-WiFi-Config");
    }
}

void saveWifiConfig()
{
    if (!littlefsAvailable)
        return;

    File f = LittleFS.open("/wifi.txt", "w");
    if (f)
    {
        f.print(staSsid);
        f.print(",");
        f.println(staPass);
        f.close();
        DEBUG_PRINTLN("✓ WiFi-Config gespeichert");
    }
}
