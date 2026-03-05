# Funktionsvergleich: ESPHome-Steuerung vs. VentoMaxx Original

Dieser Vergleich basiert auf der Analyse der VentoMaxx Bedienungsanleitung (`MA-BA_Endmontage_V1_V3-WRG_DE_2502.pdf`) und dem aktuellen Funktionsumfang der ESPHome-Firmware (Stand: März 2026).

## 🏆 Zusammenfassung

Die **ESPHome-Steuerung** bietet **100% der Original-Funktionalität** (inkl. aller Sicherheitsmechanismen wie Tacho-Überwachung und Frostschutz) und erweitert diese massiv um:

- **Smart-Home-Integration** (WiFi, Home Assistant, OTA-Updates)
- **Präzisions-Sensorik** (SCD41 CO2, BMP390 Luftdruck, HLK-LD2450 Radar, NTC-Temperaturen)
- **Intelligente Regelalgorithmen** (PID-gesteuerte CO2/Feuchte-Regelung, adaptive Zykluszeiten)
- **Prädiktive Wartung** (Filterwechsel-Alarm basierend auf Betriebsstunden + Kalenderzeit)
- **Kabelloses Mesh-Netzwerk** (ESP-NOW für autonomen Gruppenbetrieb)

---

## ⚙️ Betriebsmodi & Logik

| Feature | VentoMaxx V-WRG (Original) | ESPHome Smart WRG (Dieses Projekt) | Vorteil ESPHome |
| :--- | :--- | :--- | :--- |
| **Wärmerückgewinnung (WRG)** | Statischer Zyklus: **70 Sek.** (fix) | **Dynamischer Zyklus**: 50s – 90s (je nach Stufe/Temp) | ✅ **Höhere Effizienz** durch angepasste Zykluszeiten |
| **Querlüftung (Sommer)** | Nur im Paar-Verbund möglich | ✅ Ja (via App/Panel/Automatik) | ⚖️ Gleichwertig |
| **Auto-Sommerbetrieb** | ❌ Nein (manuell umschalten) | ✅ **Automatische Querlüftung** wenn Außentemp. < Innentemp. (NTC + ESP-NOW Gruppendaten) | ✅ **Kein manuelles Umschalten nötig** |
| **Stoßlüftung** | 15 Min. Intensiv, dann Pause | ✅ Ja (Konfigurierbar: Zeit/Stufe) | ✅ **Flexibler** |
| **Lüfterregelung** | 3 feste Stufen | **10 Stufen + stufenlose PID-Regelung** (lautlos, keine hörbaren Drehzahlsprünge) | ✅ **Feingranular & unhörbar** |
| **Automatik (CO2)** | ❌ Nein (nur optionale VOC-Schätzung) | ✅ **SCD41 (echtes CO2)**: Stufenlose PID-Regelung mit Deadband-Hysterese, konfigurierbarem Min/Max-Level | ✅ **Präzise & leise** |
| **Automatik (Feuchte)** | Schwellwerte fest: 55%, 65%, 75% r.F. | ✅ **PID-Regler** mit konfigurierbarem Grenzwert (40-100%), Outdoor-Feuchte-Vergleich, Deadband-Hysterese (±2%) | ✅ **Anpassbar** + Outdoor-Check |
| **Anwesenheit** | ❌ Nein | ✅ **mmWave-Radar (HLK-LD2450)**: 4 Profile (Keine Anpassung, Intensiv, Normal, Gering) | ✅ **Bedarfsgerecht** je Raum |
| **Nachtmodus** | ❌ Nein (manuell ausschalten) | 📋 Geplant: Zeitgesteuerte Drosselung/Dimming | ✅ **Komfort** (in Planung) |

---

## 🛡️ Sensorik & Monitoring

| Feature | VentoMaxx V-WRG (Original) | ESPHome Smart WRG (Dieses Projekt) | Vorteil ESPHome |
| :--- | :--- | :--- | :--- |
| **CO2-Messung** | ❌ Nein (nur optionales VOC-Modul) | ✅ **SCD41**: Photoacoustic sensing, 400-5000 ppm | ✅ **Echte CO2-Werte** statt Schätzung |
| **Temperatur** | ❌ Keine Anzeige | ✅ **NTC-Sensoren** (Zuluft/Abluft) + HA Wetterdaten | ✅ **Visualisierung & Automatik** |
| **Luftfeuchtigkeit** | Rudimentär (feste Schwellen) | ✅ **SCD41** (Innen) + **Außenfeuchte** (HA Wetterdienst/Sensor) | ✅ **PID-geregelt + Outdoor-Check** |
| **Luftdruck** | ❌ Nein | ✅ **BMP390**: Wettertrend, Sturmwarnung, SCD41-Höhenkompensation | ✅ **Zusätzliche Umweltdaten** |
| **Anwesenheit** | ❌ Nein | ✅ **HLK-LD2450 mmWave-Radar**: Präsenz, Moving/Still Targets, Target Count | ✅ **Raumgenaue Erfassung** |
| **Drehzahl** | Tacho-Signal (LED blinkt bei Fehler) | ✅ **Pulse Counter**: RPM-Wert in Home Assistant + Fehleralarm | ✅ **Quantitative Überwachung** |

---

## 🔧 Sicherheit & Wartung

| Feature | VentoMaxx V-WRG (Original) | ESPHome Smart WRG (Dieses Projekt) | Vorteil ESPHome |
| :--- | :--- | :--- | :--- |
| **Lüfter-Überwachung** | Tacho-Signal (LED blinkt bei Fehler) | ✅ **Tacho-Monitor**: Alarm in Home Assistant + LED | ✅ **Push-Benachrichtigung** bei Ausfall |
| **Frostschutz** | Abschaltung bei Zuluft < 5°C | ✅ **NTC-Überwachung** (Abschaltung/Vorheizregister) | ✅ **Visualisierung** der Temperaturen |
| **Filterwechsel** | Timer-basiert (LED blinkt) | ✅ **Prädiktiver Alarm**: Betriebsstunden (>365d) + Kalenderzeit (>3 Jahre) + HA Push-Benachrichtigung + Reset-Button | ✅ **Präziser & proaktiv** |
| **Fehlererkennung** | LED blinkt bei Tachofehler | ✅ **Master LED Strobe**: WiFi-Ausfall + ESP-NOW Timeout (5 Min) | ✅ **Umfassender** |
| **Kommunikation** | Kabelgebunden (Steuerleitung) | **ESP-NOW Funk** (Ausfallsicher, kein Router nötig) | ✅ **Keine Phasenkoppler nötig** |
| **Synchronisierung** | Master/Slave (DIP-Schalter) | **Mesh-Netzwerk** (Auto-Gruppenfindung, PID-Demand Sync) | ✅ **Automatisch + identische Lüfterleistung** |
| **Stromverbrauch** | Standby < 1W | Standby **~0.5W** (ESP32-C6) | ⚖️ Vergleichbar |

---

## 📱 Bedienung & Schnittstellen

| Feature | VentoMaxx V-WRG (Original) | ESPHome Smart WRG (Dieses Projekt) | Vorteil ESPHome |
| :--- | :--- | :--- | :--- |
| **Bedienpanel** | Taster + LEDs | ✅ **Unterstützt Original-Panel** (volle Funktion) | ⚖️ **Identische Optik/Haptik** |
| **Display** | ❌ Nein | ✅ **SH1106 OLED** (128x64): 3 Diagnoseseiten (System, Klima, Netzwerk) | ✅ **Sofort-Diagnose am Gerät** |
| **Fernbedienung** | IR-Fernbedienung (optional) | **Jedes Smartphone / Tablet / PC** | ✅ **Keine Zusatzkosten** |
| **Smart Home** | ❌ Nein (proprietär) | ✅ **Native Home Assistant API** | ✅ **Volle Integration** |
| **Gruppensteuerung** | Master/Slave über Kabel | ✅ **ESP-NOW Group Controller**: Alle Geräte als eine Einheit im HA Dashboard | ✅ **Intuitiv + kabellos** |
| **Daten-Transparenz** | ❌ Keine (Blackbox) | ✅ **Vollständige Historie** (Temp, CO2, Feuchte, Druck, RPM, Betriebsstunden) | ✅ **Datenbasierte Optimierung** |
| **Konfig-Sync** | ❌ Nein (jedes Gerät einzeln) | ✅ **Automatisch via ESP-NOW**: Alle Einstellungen gruppenweit synchronisiert | ✅ **Einmal einstellen, überall aktiv** |
| **Updates** | ❌ Nein (Hardware-Tausch nötig) | ✅ **OTA-Updates** (Over-the-Air) | ✅ **Zukunftssicher** |
| **Konfiguration** | DIP-Schalter auf Platine | **Web-Oberfläche / YAML / HA Slider** | ✅ **Komfortabel** |
| **System & Lizenz** | Geschlossen (proprietär) | ✅ **100% Open Source (MIT)** | ✅ **Kein Vendor-Lock-In** |

---

## 💡 Fazit

Die ESPHome-Lösung ist ein **"Drop-in Replacement"** für die originale VentoMaxx-Steuerung. Sie behält die bewährte Bedienung über das Originalpanel bei, beseitigt aber die Limitierungen des Originals und erweitert den Funktionsumfang um ein Vielfaches:

| Kategorie | VentoMaxx (Original) | ESPHome Smart WRG |
| :--- | :---: | :---: |
| Betriebsmodi | 3 | **5+** (inkl. Automatiken) |
| Sensorik | 0-1 (opt. VOC) | **6** (CO2, Temp, Feuchte, Druck, Radar, Tacho) |
| Lüfterstufen | 3 | **10 + stufenlos (PID)** |
| Smart Home | ❌ | ✅ Home Assistant |
| Wartungsalarm | Timer-LED | ✅ **Prädiktiv + Push** |
| Kabel für Sync | Ja | ❌ **Kabellos (ESP-NOW)** |
| Open Source | ❌ | ✅ MIT-Lizenz |

**Empfehlung:** Wer eine **vollständige Smart-Home-Integration** sucht und **keine neuen Kabel** ziehen möchte, erhält mit der ESPHome-Lösung einen massiv erweiterten Funktionsumfang bei gleicher mechanischer Kompatibilität — inklusive Präzisionssensorik, prädiktiver Wartung und intelligenter Automatik.
