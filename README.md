# Smart Solar Tracker ESP32

Ein intelligenter Solar-Tracker basierend auf ESP32 mit Web-Interface, automatischer Kalibrierung und RTC-Unterstützung.

## Features

- 🌞 **Automatisches Sun-Tracking** mit 4 LDR-Sensoren
- 🌐 **Web-Interface** für Kontrolle und Konfiguration
- ⚙️ **Automatische Kalibrierung** zur optimalen Ausrichtung
- 🕐 **RTC + NTP Zeitverwaltung** für präzises astronomisches Tracking
- 📍 **GPS-Koordinaten konfigurierbar** für Sonnenpositionsberechnung
- 📊 **Position-History** mit CSV-Export
- 🌙 **Auto-Sleep** bei Nacht/Wolken/Regen
- 🐛 **Runtime Debug-Steuerung** über Web-Interface
- 💾 **Persistent Storage** mit LittleFS

## Hardware

- **MCU:** ESP32 Dev Module
- **Servos:** 2x (Yaw/Azimut + Tilt/Neigung)
- **Sensoren:** 4x LDR (Lichtsensoren)
- **Optional:** DS3231 RTC Modul
- **Pins:** Siehe `src/config.h`

## Installation

1. **PlatformIO installieren**
   - [VS Code](https://code.visualstudio.com/) + [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

2. **Projekt klonen**
   ```bash
   git clone https://github.com/DEIN_USERNAME/Solar-Tracker.git
   cd Solar-Tracker
   ```

3. **Dependencies installieren** (automatisch via PlatformIO)
   - ESP32Servo
   - NTPClient
   - SolarCalculator
   - RTClib
   - Adafruit BusIO

4. **Upload**
   ```bash
   pio run --target upload
   ```

## Konfiguration

### Erste Schritte

1. **AP-Modus:** Nach dem Start erstellt der ESP32 einen Access Point
   - SSID: `SolarTracker_AP`
   - Passwort: `solar123`

2. **Web-Interface öffnen:** `http://192.168.4.1`

3. **WiFi konfigurieren:**
   - Über Web-Interface → "📡 WiFi ändern"
   - Eigenes WLAN einrichten für Internet-Zeitabgleich

4. **Position einstellen:**
   - "📍 Position ändern"
   - GPS-Koordinaten eingeben (Breitengrad/Längengrad)
   - Zeitzone konfigurieren

5. **Kalibrierung:**
   - "⚙️ Kalibrierung"
   - Automatischer Sweep zur optimalen Ausrichtung

## API Endpunkte

### Status & Kontrolle
- `GET /` - Haupt-Interface
- `GET /status` - JSON Status (Winkel, Zustand, Sonnenposition)
- `GET /weather?rain=0&k=key-123` - Wetter-Kontrolle (0=Sonne, 1=Regen)

### Konfiguration
- `GET /geo` - Position & Zeitzone ändern
- `POST /geo/save` - Position speichern
- `GET /wifi` - WiFi konfigurieren
- `POST /wifi/save` - WiFi speichern

### Kalibrierung & Debug
- `GET /calibrate` - Kalibrierungs-Interface
- `GET /calibrate/start` - Kalibrierung starten
- `GET /debug` - Debug-Modus Interface
- `GET /debug/set?enabled=1` - Debug ein/aus

### Historie
- `GET /history` - CSV-Download der Position-History

## Projekt-Struktur

```
src/
├── main.cpp              # Hauptprogramm (setup, loop)
├── config.h/cpp          # Hardware-Konfiguration & Konstanten
├── time_rtc.h/cpp        # RTC & NTP Zeitverwaltung
├── st_debug.h/cpp        # Debug-Steuerung
├── st_calibration.h/cpp  # Kalibrierungs-Funktionen
├── st_config.h/cpp       # Konfigurations-Dateiverwaltung
├── st_helpers.h/cpp      # Hilfsfunktionen (History, Watchdog)
├── st_init.h/cpp         # System-Initialisierung
└── st_webserver.h/cpp    # Web-Server & API-Handler
```

## Betriebsmodi

### STATE_INIT
Initialisierung beim Start:
- LittleFS mounten
- Konfiguration laden
- WiFi verbinden
- RTC/NTP synchronisieren
- Kalibrierte Position laden

### STATE_TRACKING
Aktives Tracking:
- LDR-basiertes Fine-Tuning
- Astronomische Sonnenposition
- Hybride Steuerung beider Methoden
- Position-History logging

### STATE_SLEEP
Ruhe-Modus bei:
- Nacht (alle LDRs unter Schwellwert)
- Wolken (Durchschnittslicht zu niedrig)
- Wetter-Stop via API
- Schutzposition: Yaw=90°, Tilt=70°

## Watchdog

Software-Watchdog überwacht Loop-Aktivität:
- Timeout: 60 Sekunden
- Nach mehrfachen Timeouts: Auto-Reset
- Schützt vor Freeze/Deadlock

## Flash-Speicher Schutz

- History-Schreibvorgänge begrenzt auf max. alle 5 Minuten
- Reduziert Flash-Wear
- Erzwungenes Schreiben bei wichtigen Events (Kalibrierung, Moduswechsel)

## Entwicklung

### Debug aktivieren/deaktivieren
Web-Interface → "🐛 Debug-Modus" → Toggle

### Neue Module hinzufügen
1. Header `.h` im `src/` Ordner erstellen
2. Implementation `.cpp` erstellen
3. In `main.cpp` einbinden
4. PlatformIO erkennt neue Files automatisch

### Build & Upload
```bash
# Kompilieren
pio run

# Upload
pio run --target upload

# Serial Monitor
pio device monitor
```

## Hardware-Anschluss

### Servos
- **Yaw (Azimut):** GPIO 13
- **Tilt (Neigung):** GPIO 12

### LDR-Sensoren (analog)
- **Top-Left:** GPIO 36
- **Top-Right:** GPIO 39
- **Bottom-Left:** GPIO 34
- **Bottom-Right:** GPIO 35

### I2C (RTC)
- **SDA:** GPIO 21
- **SCL:** GPIO 22

## Lizenz

MIT License - Siehe LICENSE Datei

## Autor

Entwickelt mit ESP32 & PlatformIO

---

**Status:** ✅ Stabil | Flash: 67% | RAM: 14%
