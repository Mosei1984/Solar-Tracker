#include "st_debug.h"
#include "config.h"

// External references
extern bool littlefsAvailable;

// Global debug state (default enabled)
bool debugEnabled = true;

void loadDebugConfig()
{
    debugEnabled = true; // Default

    if (!littlefsAvailable)
        return;

    if (LittleFS.exists("/debug.txt"))
    {
        File f = LittleFS.open("/debug.txt", "r");
        if (f)
        {
            String data = f.readString();
            f.close();
            data.trim();
            debugEnabled = (data == "1" || data.equalsIgnoreCase("true"));
            Serial.print("✓ Debug-Config geladen: ");
            Serial.println(debugEnabled ? "EIN" : "AUS");
        }
    }
}

void saveDebugConfig()
{
    if (!littlefsAvailable)
        return;

    File f = LittleFS.open("/debug.txt", "w");
    if (f)
    {
        f.println(debugEnabled ? "1" : "0");
        f.close();
        Serial.print("✓ Debug-Config gespeichert: ");
        Serial.println(debugEnabled ? "EIN" : "AUS");
    }
}

void setDebugEnabled(bool enabled)
{
    debugEnabled = enabled;
    saveDebugConfig();
    Serial.print("Debug-Modus: ");
    Serial.println(debugEnabled ? "AKTIVIERT" : "DEAKTIVIERT");
}

bool isDebugEnabled()
{
    return debugEnabled;
}
