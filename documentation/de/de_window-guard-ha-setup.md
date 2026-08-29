# 🏠 Home Assistant Konfiguration: Fenstersperre (Window Guard)

[![Language: EN](https://img.shields.io/badge/Language-EN-red.svg)](../en/en_window-guard-ha-setup.md)


Die **Fenstersperre (Window Guard)** pausiert automatisch alle Lüftungsgeräte in einem Raum, sobald ein Fenster geöffnet wird, um Energieverschwendung und Wärmeverluste zu vermeiden.

---

## ✨ Leistungsmerkmale & Verhalten

- ⏱️ **Smart Pause (5s Verzögerung)**: Die Sperre greift erst nach 5 Sekunden durchgehender Fenster-Öffnung, um kurzes Lüften oder Nachschauen abzufedern. Alle VentoSync-Geräte im Raum stoppen sofort ihre Lüfter.
- 🔄 **Automatisches Fortsetzen**: Das System behält seinen aktuellen Betriebsmodus (z. B. Automatik oder Manuell) bei und nimmt den Betrieb nahtlos wieder auf, sobald alle Fenster geschlossen sind.
- 🔆 **Visuelles Feedback (35s Limit)**: Ein markantes Pulsieren der Master-LED (1s An, 2s Aus) signalisiert den Zustand "Pause durch Fenster". Zur Vermeidung von Lichtstörungen nachts stoppt das Pulsieren nach 35 Sekunden, während der Lüfter weiterhin sicher gestoppt bleibt.
- 📊 **HA Status-Entität**: Eine dedizierte Binär-Sensor-Entität (`binary_sensor.fenstersperre_aktiv`) bietet direkte Sichtbarkeit des Sperrstatus in Home Assistant.
- 🎛️ **Individueller Bypass-Schalter**: Über den Schalter **"Fenstersperre ignorieren"** (`switch.ignore_window_guard` / `switch.fenstersperre_ignorieren`) können einzelne Geräte bei Bedarf von der Raumsperre ausgenommen werden.

---

## 🛠️ Einrichtung in Home Assistant

Für die Einbindung deiner Fenstersensoren in den VentoSync Window Guard für einen bestimmten Raum (z. B. **Raum 1**) erstellst du in Home Assistant eine **Binärer Sensor-Gruppe**. Diese Gruppe bündelt die Sensoren zu einer einzigen Entität, auf die die Firmware automatisch zugreift.

**Standard-Entity-ID für Raum 1:** `binary_sensor.ventosync_window_lock_room_1`

### Option A: Über die Benutzeroberfläche (Empfohlen)
1. Gehe zu **Einstellungen** > **Geräte & Dienste** > **Helfer**.
2. Klicke auf **Helfer erstellen** > **Gruppe** > **Binärer Sensor-Gruppe**.
3. **Name**: `VentoSync Window Lock Room 1`
4. **Mitglieder**: Füge alle deine Fensterkontakte hinzu (z. B. `binary_sensor.fenster_dg_wohnraum_contact`).
5. **Status aller Entitäten**: Aktiviere **"Status einer Entität"** (Standard – d. h. wenn *irgendein* Fenster offen ist, ist die Gruppe `on`).
6. **Entitäts-ID**: Ändere diese manuell auf `ventosync_window_lock_room_1`.

### Option B: Über die `configuration.yaml`
Füge folgenden Code in deine Home Assistant Konfiguration ein:

```yaml
binary_sensor:
  - platform: group
    name: "VentoSync Window Lock Room 1"
    unique_id: ventosync_window_lock_room_1
    device_class: window
    entities:
      - binary_sensor.fenster_dg_wohnraum_contact
      - binary_sensor.fenster_dg_flur_contact
      - binary_sensor.fenster_dg_gaube_contact
```
