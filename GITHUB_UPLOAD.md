# GitHub Upload Anleitung

Das Repository wurde lokal vorbereitet und committed. Um es auf GitHub hochzuladen:

## Option 1: Via GitHub Web Interface (Empfohlen)

1. **Gehe zu GitHub:**
   - Öffne [github.com](https://github.com)
   - Klicke auf "+" → "New repository"

2. **Repository erstellen:**
   - Name: `Solar-Tracker`
   - Description: `Smart Solar Tracker with ESP32 - Automatic sun tracking with web interface`
   - **WICHTIG:** ❌ KEINE README, .gitignore oder License hinzufügen (wir haben diese bereits lokal)
   - Public oder Private wählen
   - "Create repository" klicken

3. **Code pushen:**
   Kopiere die folgenden Befehle in dein Terminal:
   
   ```bash
   cd "C:\Users\mosei\Documents\PlatformIO\Projects\Solar-Tracker"
   git remote add origin https://github.com/DEIN_USERNAME/Solar-Tracker.git
   git branch -M main
   git push -u origin main
   ```
   
   **Ersetze `DEIN_USERNAME` mit deinem GitHub-Benutzernamen!**

## Option 2: Via GitHub CLI

Falls GitHub CLI installiert ist:

```bash
cd "C:\Users\mosei\Documents\PlatformIO\Projects\Solar-Tracker"
gh repo create Solar-Tracker --public --source=. --remote=origin --push
```

## Aktueller Status

✅ Git Repository initialisiert
✅ .gitignore erstellt (excludes .pio, build artifacts)
✅ README.md erstellt
✅ Initial commit erstellt (26 Dateien, 2567 Zeilen)
⏳ GitHub Remote noch nicht hinzugefügt

## Was wurde NICHT hochgeladen?

- ❌ `create_gt2_pulley/` - Wie gewünscht ausgeschlossen
- ❌ `.pio/` Build-Dateien - Via .gitignore ausgeschlossen
- ❌ `.vscode/` Editor-Konfiguration - Via .gitignore ausgeschlossen

## Enthaltene Dateien

### Haupt-Code
- `src/*.cpp` und `src/*.h` - Alle Quellcode-Module
- `platformio.ini` - PlatformIO Konfiguration

### Dokumentation
- `README.md` - Vollständige Projekt-Dokumentation
- `REFACTORING_SUMMARY.md` - Refactoring-Details
- `.gitignore` - Git ignore rules

### Module
- st_calibration - Kalibrierung
- st_config - Konfigurationsverwaltung
- st_debug - Debug-Steuerung
- st_helpers - Hilfsfunktionen
- st_init - Initialisierung
- st_webserver - Web-Server
- time_rtc - Zeitverwaltung

## Nach dem Push

Nach erfolgreichem Push kannst du:
- Repository auf GitHub ansehen
- Weitere Commits pushen mit: `git push`
- Branches erstellen und managen
- Issues und Pull Requests nutzen

## Hilfe

Falls Probleme auftreten:
```bash
# Status prüfen
git status

# Logs ansehen
git log --oneline

# Remote prüfen
git remote -v
```
