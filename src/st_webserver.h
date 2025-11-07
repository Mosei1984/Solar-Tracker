#ifndef ST_WEBSERVER_H
#define ST_WEBSERVER_H

#include "config.h"

// Webserver Objekt
extern WebServer server;

// HTML Content
extern const char INDEX_HTML[] PROGMEM;

// HTTP Handler Funktionen
void handleRoot();
void handleStatus();
void handleWeather();
void handleHistory();
void handleGeoConfig();
void handleGeoSave();
void handleWifiConfig();
void handleWifiSave();
void handleCalibrate();
void handleCalibrateStart();
void handleDebug();
void handleDebugSet();
void handleNotFound();

#endif
