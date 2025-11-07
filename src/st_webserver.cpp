#include "st_debug.h"
#include "config.h"
#include "time_rtc.h"
#include "st_config.h"
#include "st_calibration.h"
#include "st_webserver.h"

// External references from main.cpp
extern TrackerState trackerState;
extern int yawAngle;
extern int tiltAngle;
extern bool weatherSaysStop;

// External references from config.cpp
extern double currentLatitude;
extern double currentLongitude;

// Webserver Objekt
WebServer server(80);

const char INDEX_HTML[] PROGMEM = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <title>SolarTracker</title>
    <style>
      body { font-family: sans-serif; background:#f4f4f4; padding:20px; }
      h1 { margin-bottom: 10px; }
      button { padding:10px 18px; margin:5px; font-size:16px; }
      .ok { background:#4CAF50; color:white; border:none; }
      .stop { background:#f44336; color:white; border:none; }
      .info { background:#2196F3; color:white; border:none; }
      #statusBox { margin-top:15px; padding:10px; background:white; border:1px solid #ccc; }
      .time { font-size: 18px; font-weight: bold; color: #333; margin-bottom: 10px; }
      #visualContainer { margin-top:15px; padding:10px; background:white; border:1px solid #ccc; text-align:center; }
      #trackerCanvas { border:1px solid #ddd; background:#e8f4ff; }
    </style>
  </head>
  <body>
    <h1>SolarTracker ESP32</h1>
    <div id="timeBox" class="time">Zeit wird geladen...</div>
    <p>
      <button class="ok" onclick="fetch('/weather?rain=0&k=key-123').then(r=>loadStatus())">Sonne / Weiter</button>
      <button class="stop" onclick="fetch('/weather?rain=1&k=key-123').then(r=>loadStatus())">Regen / Stopp</button>
      <button class="info" onclick="loadStatus()">Status laden</button>
      <button class="info" onclick="window.location.href='/history'">Historie herunterladen</button>
      <button class="info" onclick="window.location.href='/geo'">📍 Position ändern</button>
      <button class="info" onclick="window.location.href='/wifi'">📡 WiFi ändern</button>
      <button class="info" onclick="window.location.href='/calibrate'">⚙️ Kalibrierung</button>
      <button class="info" onclick="window.location.href='/debug'">🐛 Debug-Modus</button>
    </p>
    <div id="visualContainer">
      <h3>Tracker Visualisierung</h3>
      <canvas id="trackerCanvas" width="400" height="400"></canvas>
    </div>
    <div id="statusBox">Status wird geladen...</div>

    <script>
      function drawTracker(j) {
        const canvas = document.getElementById('trackerCanvas');
        const ctx = canvas.getContext('2d');
        const w = canvas.width;
        const h = canvas.height;
        const cx = w / 2;
        const cy = h / 2;
        
        ctx.clearRect(0, 0, w, h);
        
        ctx.fillStyle = '#e8f4ff';
        ctx.fillRect(0, 0, w, h);
        
        ctx.strokeStyle = '#999';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.arc(cx, cy, 150, 0, 2 * Math.PI);
        ctx.stroke();
        
        ctx.strokeStyle = '#ccc';
        ctx.setLineDash([5, 5]);
        ctx.beginPath();
        ctx.moveTo(cx, cy - 150);
        ctx.lineTo(cx, cy + 150);
        ctx.moveTo(cx - 150, cy);
        ctx.lineTo(cx + 150, cy);
        ctx.stroke();
        ctx.setLineDash([]);
        
        ctx.fillStyle = '#666';
        ctx.font = '12px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('N', cx, cy - 165);
        ctx.fillText('S', cx, cy + 180);
        ctx.fillText('E', cx + 165, cy + 5);
        ctx.fillText('W', cx - 165, cy + 5);
        
        if (j.state === 'SLEEP') {
          ctx.font = 'bold 24px sans-serif';
          ctx.fillStyle = '#333';
          ctx.textAlign = 'center';
          ctx.fillText(j.weatherStop ? '☁️ Wetter-Stop' : '🌙 Nachtmodus', cx, cy - 30);
          ctx.font = '16px sans-serif';
          ctx.fillStyle = '#666';
          ctx.fillText('Tracker inaktiv', cx, cy + 10);
          
          if (j.weatherStop) {
            ctx.fillStyle = 'rgba(100, 100, 100, 0.3)';
            ctx.beginPath();
            ctx.arc(cx, cy + 60, 40, 0, 2 * Math.PI);
            ctx.fill();
            ctx.fillStyle = '#555';
            ctx.font = '50px sans-serif';
            ctx.fillText('☔', cx, cy + 80);
          } else {
            ctx.fillStyle = 'rgba(255, 255, 100, 0.3)';
            ctx.beginPath();
            ctx.arc(cx, cy + 60, 35, 0, 2 * Math.PI);
            ctx.fill();
            ctx.fillStyle = '#888';
            ctx.font = '45px sans-serif';
            ctx.fillText('🌙', cx, cy + 80);
          }
        } else if (j.state === 'TRACKING' && j.sunAzimuth != null && j.sunAltitude != null) {
          const azimuth = j.sunAzimuth;
          const altitude = j.sunAltitude;
          
          const azRad = (azimuth - 90) * Math.PI / 180;
          const radius = 150 * (1 - altitude / 90);
          const sunX = cx + radius * Math.cos(azRad);
          const sunY = cy + radius * Math.sin(azRad);
          
          ctx.strokeStyle = 'rgba(255, 200, 0, 0.5)';
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.moveTo(cx, cy);
          ctx.lineTo(sunX, sunY);
          ctx.stroke();
          
          const gradient = ctx.createRadialGradient(sunX, sunY, 5, sunX, sunY, 25);
          gradient.addColorStop(0, '#FFD700');
          gradient.addColorStop(0.5, '#FFA500');
          gradient.addColorStop(1, 'rgba(255, 165, 0, 0.3)');
          ctx.fillStyle = gradient;
          ctx.beginPath();
          ctx.arc(sunX, sunY, 25, 0, 2 * Math.PI);
          ctx.fill();
          
          ctx.strokeStyle = '#FFA500';
          ctx.lineWidth = 2;
          ctx.beginPath();
          ctx.arc(sunX, sunY, 25, 0, 2 * Math.PI);
          ctx.stroke();
          
          for (let i = 0; i < 8; i++) {
            const angle = i * Math.PI / 4;
            const x1 = sunX + 30 * Math.cos(angle);
            const y1 = sunY + 30 * Math.sin(angle);
            const x2 = sunX + 40 * Math.cos(angle);
            const y2 = sunY + 40 * Math.sin(angle);
            ctx.beginPath();
            ctx.moveTo(x1, y1);
            ctx.lineTo(x2, y2);
            ctx.stroke();
          }
          
          ctx.fillStyle = '#333';
          ctx.font = 'bold 14px sans-serif';
          ctx.textAlign = 'center';
          ctx.fillText('☀️ Sonne', cx, cy - 30);
          ctx.font = '12px sans-serif';
          ctx.fillText('Az: ' + azimuth.toFixed(1) + '°', cx, cy - 10);
          ctx.fillText('Alt: ' + altitude.toFixed(1) + '°', cx, cy + 5);
          
          if (j.yaw != null && j.tilt != null) {
            const yawRad = (j.yaw - 90) * Math.PI / 180;
            const tiltRadius = 100 * (1 - j.tilt / 90);
            const trackerX = cx + tiltRadius * Math.cos(yawRad);
            const trackerY = cy + tiltRadius * Math.sin(yawRad);
            
            ctx.fillStyle = 'rgba(76, 175, 80, 0.6)';
            ctx.beginPath();
            ctx.arc(trackerX, trackerY, 15, 0, 2 * Math.PI);
            ctx.fill();
            
            ctx.strokeStyle = '#4CAF50';
            ctx.lineWidth = 2;
            ctx.beginPath();
            ctx.arc(trackerX, trackerY, 15, 0, 2 * Math.PI);
            ctx.stroke();
            
            ctx.fillStyle = '#fff';
            ctx.font = 'bold 16px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('T', trackerX, trackerY + 5);
          }
        } else {
          ctx.font = 'bold 20px sans-serif';
          ctx.fillStyle = '#666';
          ctx.textAlign = 'center';
          ctx.fillText('Initialisierung...', cx, cy);
        }
      }
      
      function loadStatus() {
        fetch('/status')
          .then(r => r.json())
          .then(j => {
            if (j.time) {
              document.getElementById('timeBox').innerHTML = '📅 ' + j.time;
            } else {
              document.getElementById('timeBox').innerHTML = '⏰ Warte auf Zeitinitialisierung...';
            }
            document.getElementById('statusBox').innerHTML =
              '<b>Zustand:</b> ' + j.state +
              '<br><b>Yaw:</b> ' + j.yaw + '°' +
              '<br><b>Tilt:</b> ' + j.tilt + '°' +
              '<br><b>Sonnen-Azimut:</b> ' + (j.sunAzimuth || 'N/A') + '°' +
              '<br><b>Sonnen-Altitude:</b> ' + (j.sunAltitude || 'N/A') + '°' +
              '<br><b>Wetter-Stop:</b> ' + j.weatherStop +
              '<br><br><b>📍 Position:</b> ' + (j.latitude || 'N/A') + ', ' + (j.longitude || 'N/A') +
              '<br><b>🕐 Zeitzone:</b> UTC' + (j.timezoneOffset >= 0 ? '+' : '') + j.timezoneOffset + ' (' + (j.useAutoDST ? 'Auto DST' : 'Manuell') + ')' +
              '<br><b>⏰ Zeit:</b> ' + (j.timeInitialized ? '✓ Internet' : '⚠ Lokal') +
              '<br><b>📡 WiFi:</b> ' + (j.wifiConnected ? '✓' : '✗');
            
            drawTracker(j);
          })
          .catch(e => {
            document.getElementById('statusBox').innerHTML = 'Fehler beim Laden';
          });
      }
      loadStatus();
      setInterval(loadStatus, 5000);
    </script>
  </body>
  </html>
  )rawliteral";

void handleGeoConfig()
{
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Konfiguration</title>";
    html += "<style>body{font-family:sans-serif;padding:20px;background:#f4f4f4;}";
    html += "form{background:white;padding:20px;border:1px solid #ccc;max-width:500px;}";
    html += "input,select{padding:8px;margin:5px 0;width:100%;box-sizing:border-box;}";
    html += "button{padding:10px 18px;margin:10px 5px 0 0;font-size:16px;background:#2196F3;color:white;border:none;}";
    html += "</style></head><body>";
    html += "<h2>⚙️ System Konfiguration</h2>";
    html += "<form action='/geo/save' method='POST'>";
    html += "<h3>📍 Geo-Position</h3>";
    html += "Breitengrad (Latitude): <input type='number' name='lat' step='0.000001' value='";
    html += String(currentLatitude, 6);
    html += "' required><br>";
    html += "Längengrad (Longitude): <input type='number' name='lon' step='0.000001' value='";
    html += String(currentLongitude, 6);
    html += "' required><br>";
    html += "<h3>🕐 Zeitzone</h3>";
    html += "Automatische Sommerzeit (EU): <select name='autodst'>";
    html += "<option value='1'" + String(useAutoDST ? " selected" : "") + ">Aktiviert (MEZ/MESZ)</option>";
    html += "<option value='0'" + String(!useAutoDST ? " selected" : "") + ">Deaktiviert</option>";
    html += "</select><br>";
    html += "Manuelle Zeitzone (UTC Offset in Stunden): <input type='number' name='tzoffset' step='0.5' min='-12' max='14' value='";
    html += String(manualTimezoneOffset / 3600.0, 1);
    html += "'><br><small>Wird nur verwendet wenn Auto-Sommerzeit deaktiviert ist</small><br><br>";
    html += "<button type='submit'>💾 Speichern</button> ";
    html += "<a href='/'><button type='button'>🏠 Zurück</button></a>";
    html += "</form></body></html>";

    server.send(200, "text/html", html);
}

void handleGeoSave()
{
    if (server.hasArg("lat") && server.hasArg("lon") && server.hasArg("autodst") && server.hasArg("tzoffset"))
    {
        currentLatitude = server.arg("lat").toDouble();
        currentLongitude = server.arg("lon").toDouble();
        useAutoDST = (server.arg("autodst") == "1");
        manualTimezoneOffset = (int)(server.arg("tzoffset").toFloat() * 3600);
        saveGeoConfig();

        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<meta http-equiv='refresh' content='2;url=/'></head><body>";
        html += "<h2>✓ Konfiguration gespeichert!</h2>";
        html += "<p>Weiterleitung...</p></body></html>";
        server.send(200, "text/html", html);
    }
    else
    {
        server.send(400, "text/plain", "Fehlende Parameter");
    }
}

void handleRoot()
{
    server.send_P(200, "text/html", INDEX_HTML);
}

void handleStatus()
{
    String json = "{";
    json += "\"state\":\"";
    switch (trackerState)
    {
    case STATE_INIT:
        json += "INIT";
        break;
    case STATE_TRACKING:
        json += "TRACKING";
        break;
    case STATE_SLEEP:
        json += "SLEEP";
        break;
    }
    json += "\",\"yaw\":" + String(yawAngle);
    json += ",\"tilt\":" + String(tiltAngle);
    json += ",\"weatherStop\":" + String(weatherSaysStop ? "true" : "false");

    json += ",\"timeInitialized\":" + String(timeInitialized ? "true" : "false");
    json += ",\"rtcAvailable\":" + String(rtcAvailable ? "true" : "false");
    json += ",\"wifiConnected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false");
    json += ",\"lastNTPUpdate\":" + String(millis() - lastNTPUpdate);

    // Geo-Position
    json += ",\"latitude\":" + String(currentLatitude, 6);
    json += ",\"longitude\":" + String(currentLongitude, 6);
    json += ",\"useAutoDST\":" + String(useAutoDST ? "true" : "false");
    json += ",\"timezoneOffset\":" + String(getCurrentTimezoneOffset() / 3600.0, 1);

    if (timeInitialized)
    {
        DateTime now = getCurrentTime();
        char timeBuffer[30];
        sprintf(timeBuffer, "%04d-%02d-%02d %02d:%02d:%02d",
                now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second());
        json += ",\"time\":\"" + String(timeBuffer) + "\"";
    }

    if (timeInitialized)
    {
        json += ",\"sunAzimuth\":" + String(currentSunAzimuth, 1);
        json += ",\"sunAltitude\":" + String(currentSunAltitude, 1);
    }

    json += "}";
    server.send(200, "application/json", json);
}

void handleWeather()
{
    if (!server.hasArg("k") || server.arg("k") != API_KEY)
    {
        server.send(401, "text/plain", "unauthorized");
        return;
    }

    if (!server.hasArg("rain"))
    {
        server.send(400, "text/plain", "bad request: missing rain parameter");
        return;
    }

    String v = server.arg("rain");
    if (v == "1")
    {
        weatherSaysStop = true;
    }
    else if (v == "0")
    {
        weatherSaysStop = false;
    }
    else
    {
        server.send(400, "text/plain", "bad request: rain must be 0 or 1");
        return;
    }

    server.send(200, "text/plain", "ok");
}

void handleHistory()
{
    if (!LittleFS.exists("/history.csv"))
    {
        server.send(404, "text/plain", "Keine Historie verfügbar");
        return;
    }

    File file = LittleFS.open("/history.csv", "r");
    if (!file)
    {
        server.send(404, "text/plain", "No history available");
        return;
    }

    server.sendHeader("Content-Disposition", "attachment; filename=tracking_history.csv");
    server.streamFile(file, "text/csv");
    file.close();
}

void handleWifiConfig()
{
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>WiFi Konfiguration</title>";
    html += "<style>body{font-family:sans-serif;padding:20px;background:#f4f4f4;}";
    html += "form{background:white;padding:20px;border:1px solid #ccc;max-width:500px;}";
    html += "input{padding:8px;margin:5px 0;width:100%;box-sizing:border-box;}";
    html += "button{padding:10px 18px;margin:10px 5px 0 0;font-size:16px;background:#2196F3;color:white;border:none;}";
    html += ".warning{background:#fff3cd;border:1px solid #ffc107;padding:10px;margin:10px 0;border-radius:4px;}";
    html += "</style></head><body>";
    html += "<h2>📡 WiFi Konfiguration</h2>";
    html += "<div class='warning'>⚠️ Nach dem Speichern wird das Gerät versuchen, sich mit dem neuen Netzwerk zu verbinden. ";
    html += "Der Access Point (SolarTracker_AP) bleibt aktiv.</div>";
    html += "<form action='/wifi/save' method='POST'>";
    html += "SSID (Netzwerkname): <input type='text' name='ssid' value='";
    html += String(staSsid);
    html += "' maxlength='63' required><br>";
    html += "Passwort: <input type='password' name='pass' value='";
    html += String(staPass);
    html += "' maxlength='63'><br>";
    html += "<small>Lassen Sie das Passwort leer für offene Netzwerke</small><br><br>";
    html += "<button type='submit'>💾 Speichern & Verbinden</button> ";
    html += "<a href='/'><button type='button'>🏠 Zurück</button></a>";
    html += "</form></body></html>";

    server.send(200, "text/html", html);
}

void handleWifiSave()
{
    if (server.hasArg("ssid"))
    {
        String ssid = server.arg("ssid");
        String pass = server.hasArg("pass") ? server.arg("pass") : "";

        ssid.trim();
        pass.trim();

        if (ssid.length() > 0 && ssid.length() < 64)
        {
            ssid.toCharArray(staSsid, 64);
            pass.toCharArray(staPass, 64);

            saveWifiConfig();

            String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
            html += "<meta http-equiv='refresh' content='3;url=/'></head><body>";
            html += "<h2>✓ WiFi-Konfiguration gespeichert!</h2>";
            html += "<p>Verbinde mit neuem Netzwerk: <b>" + String(staSsid) + "</b></p>";
            html += "<p>Bitte warten Sie einen Moment. Das Gerät startet die Verbindung neu.</p>";
            html += "<p>Weiterleitung in 3 Sekunden...</p></body></html>";
            server.send(200, "text/html", html);

            delay(1000);
            WiFi.disconnect();
            delay(500);
            WiFi.begin(staSsid, staPass);

            Serial.println("WiFi-Konfiguration aktualisiert. Verbinde neu...");
        }
        else
        {
            server.send(400, "text/plain", "SSID ungültig (muss 1-63 Zeichen sein)");
        }
    }
    else
    {
        server.send(400, "text/plain", "Fehlende Parameter");
    }
}

void handleCalibrate()
{
    int savedYaw = 90;
    int savedTilt = 90;
    loadCalibrationData(savedYaw, savedTilt);

    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Kalibrierung</title>";
    html += "<style>body{font-family:sans-serif;padding:20px;background:#f4f4f4;}";
    html += ".container{background:white;padding:20px;border:1px solid #ccc;max-width:600px;}";
    html += "button{padding:10px 18px;margin:10px 5px 0 0;font-size:16px;background:#2196F3;color:white;border:none;}";
    html += ".start-btn{background:#4CAF50;}";
    html += ".info-box{background:#e3f2fd;border:1px solid #2196F3;padding:15px;margin:15px 0;border-radius:4px;}";
    html += ".warning{background:#fff3cd;border:1px solid #ffc107;padding:10px;margin:10px 0;border-radius:4px;}";
    html += "</style></head><body>";
    html += "<h2>⚙️ Automatische Kalibrierung</h2>";
    html += "<div class='container'>";
    html += "<div class='info-box'>";
    html += "<h3>Aktuelle Kalibrierungsdaten:</h3>";
    html += "<b>Yaw (Azimut):</b> " + String(savedYaw) + "°<br>";
    html += "<b>Tilt (Neigung):</b> " + String(savedTilt) + "°";
    html += "</div>";
    html += "<div class='warning'>⚠️ Die Kalibrierung führt einen Servo-Sweep durch und kann mehrere Minuten dauern. ";
    html += "Der Tracker sucht die Position mit maximaler Lichtausbeute und speichert diese als Startposition.</div>";
    html += "<button class='start-btn' onclick=\"if(confirm('Kalibrierung jetzt starten?')) window.location.href='/calibrate/start'\">🚀 Kalibrierung starten</button> ";
    html += "<a href='/'><button type='button'>🏠 Zurück</button></a>";
    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

void handleCalibrateStart()
{
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
    html += "<meta http-equiv='refresh' content='5;url=/calibrate'></head><body>";
    html += "<h2>⚙️ Kalibrierung gestartet...</h2>";
    html += "<p>Der Tracker führt jetzt einen Sweep durch und sucht die optimale Position.</p>";
    html += "<p>Dies kann einige Minuten dauern. Bitte warten Sie.</p>";
    html += "<p>Automatische Weiterleitung in 5 Sekunden...</p></body></html>";
    server.send(200, "text/html", html);

    delay(100);
    startCalibration();
}

void handleDebug()
{
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Debug-Modus</title>";
    html += "<style>body{font-family:sans-serif;padding:20px;background:#f4f4f4;}";
    html += ".container{background:white;padding:20px;border:1px solid #ccc;max-width:600px;}";
    html += "button{padding:10px 18px;margin:10px 5px 0 0;font-size:16px;background:#2196F3;color:white;border:none;}";
    html += ".toggle-btn{background:#4CAF50;font-size:18px;padding:15px 25px;}";
    html += ".info-box{background:#e3f2fd;border:1px solid #2196F3;padding:15px;margin:15px 0;border-radius:4px;}";
    html += ".status{font-size:20px;font-weight:bold;margin:10px 0;}";
    html += ".on{color:#4CAF50;} .off{color:#f44336;}";
    html += "</style></head><body>";
    html += "<h2>🐛 Debug-Modus</h2>";
    html += "<div class='container'>";
    html += "<div class='info-box'>";
    html += "<div class='status'>Status: <span class='";
    html += isDebugEnabled() ? "on'>AKTIVIERT ✓" : "off'>DEAKTIVIERT ✗";
    html += "</span></div>";
    html += "</div>";
    html += "<p>Der Debug-Modus steuert die serielle Ausgabe über USB. Bei Deaktivierung werden keine Debug-Meldungen mehr ausgegeben.</p>";
    html += "<button class='toggle-btn' onclick=\"window.location.href='/debug/set?enabled=";
    html += isDebugEnabled() ? "0" : "1";
    html += "'\">";
    html += isDebugEnabled() ? "🔇 Debug DEAKTIVIEREN" : "🔊 Debug AKTIVIEREN";
    html += "</button> ";
    html += "<a href='/'><button type='button'>🏠 Zurück</button></a>";
    html += "</div></body></html>";

    server.send(200, "text/html", html);
}

void handleDebugSet()
{
    if (server.hasArg("enabled"))
    {
        bool enable = (server.arg("enabled") == "1");
        setDebugEnabled(enable);

        String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'>";
        html += "<meta http-equiv='refresh' content='2;url=/debug'></head><body>";
        html += "<h2>✓ Debug-Modus ";
        html += enable ? "aktiviert" : "deaktiviert";
        html += "!</h2>";
        html += "<p>Weiterleitung...</p></body></html>";
        server.send(200, "text/html", html);
    }
    else
    {
        server.send(400, "text/plain", "Fehlende Parameter");
    }
}

void handleNotFound()
{
    server.send(404, "text/plain", "Not Found");
}
