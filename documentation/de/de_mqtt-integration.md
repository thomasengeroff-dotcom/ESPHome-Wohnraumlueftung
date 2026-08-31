# 🔌 MQTT-Integration für externe Systeme

[![Language: EN](https://img.shields.io/badge/Language-EN-blue.svg)](../en/en_mqtt-integration.md)

VentoSync unterstützt optionales MQTT-Publishing zur Integration in **Node-RED**, **openHAB**, **ioBroker**, **IP-Symcon**, **LOXONE**, **Homey** und jede andere MQTT-basierte Automatisierungsplattform — ohne die native Home Assistant Integration zu beeinträchtigen.

---

## 🌐 Unterstützte Systeme & Kompatibilität

VentoSync lässt sich dank des hybriden Integrationsmodells (Native ESPHome API, MQTT und lokales Web-Dashboard) in nahezu jedes moderne Smart-Home-Ökosystem einbinden:

| Ökosystem / Plattform | Integrationsart | Beschreibung / Protokoll |
|---|---|---|
| **🏠 Home Assistant** | **Nativ** *(Primär)* | Tiefenintegration über die ESPHome Native API (`api:`). Automatische Erkennung aller Sensoren, Steuerungen, Betriebsmodi und Diagnose-Entitäten. |
| **🔴 Node-RED** | **Direkt** *(MQTT)* | Vollständige bidirektionale Steuerung und Telemetrie über MQTT-Topics und Befehle. |
| **⭕ openHAB** | **Direkt** *(MQTT)* | Anbindung über generische MQTT Things und Channels mit vollständiger Telemetrie- und Steuerungsfunktionalität. |
| **🔵 ioBroker** | **Direkt** *(MQTT)* | Integration über den ioBroker MQTT Broker/Client Adapter. |
| **🟦 IP-Symcon (SYMCON)** | **Direkt** *(MQTT)* | Bidirektionale Kommunikation über das native MQTT Server/Client Modul. |
| **🟩 LOXONE** | **Direkt** *(MQTT)* | Anbindung über das Loxone Miniserver MQTT-Gateway oder HTTP-Endpunkte. |
| **🌈 Homey** | **Direkt** *(MQTT)* | Integration über Homey MQTT Hub & Client Apps. |
| **🍏 Apple Home (HomeKit)** | **Indirekt** *(Bridge)* | Weiterleitung über die Home Assistant HomeKit Bridge oder Matter-Integration. |
| **🏠 Google Home** | **Indirekt** *(Bridge)* | Weiterleitung über die Home Assistant Google Assistant Integration. |
| **🔊 Amazon Alexa** | **Indirekt** *(Bridge)* | Weiterleitung über die Home Assistant Alexa Integration. |

---

## ✨ Übersicht

Das MQTT-Integrationspaket (`packages/integration/mqtt.yaml`) bindet die native ESPHome MQTT-Komponente parallel zur bestehenden Native API ein. Alle registrierten Entitäten (Sensoren, Selects, Numbers, Switches, Buttons, Fans, Text-Sensoren, Binärsensoren, Lights) werden automatisch an einen MQTT-Broker übertragen — ganz ohne Konfigurationsaufwand pro Entität.

**Zentrale Design-Entscheidungen:**

- **Additiv, kein Ersatz**: Die native Home Assistant API (`api:`) bleibt aktiv und unverändert. MQTT dient als zweiter, unabhängiger Datenkanal.
- **`discovery: false`**: Die MQTT-Discovery ist explizit deaktiviert, um zu verhindern, dass ESPHome Entitäten über MQTT doppelt in Home Assistant registriert (**keine doppelten Entitäten**).
- **Opt-In**: Das MQTT-Paket ist in keiner Hardwarevariante standardmäßig aktiv und muss bei Bedarf explizit einkommentiert werden.

---

## 🛠️ Einrichtung

### Voraussetzungen

- Ein laufender MQTT-Broker (z.B. [Mosquitto](https://mosquitto.org/) als Home Assistant Add-on oder eigenständige Instanz)
- Broker-Zugangsdaten (Benutzername/Passwort)

### Schritt 1: MQTT-Secrets hinzufügen

In deiner `secrets.yaml` (siehe `secrets_example.yaml` als Vorlage) eintragen:

```yaml
mqtt_broker: "192.168.1.100"     # IP oder Hostname deines MQTT-Brokers
mqtt_port: "1883"                # Standard MQTT-Port
mqtt_username: "dein_benutzername"
mqtt_password: "dein_passwort"
```

### Schritt 2: MQTT-Paket aktivieren

Füge eine einzelne Zeile im `packages:`-Abschnitt deiner Hardware-YAML ein (z.B. `ventosync.yaml`, `ventosync_nosensor.yaml`, etc.):

```yaml
packages:
  base: !include packages/base/ventosync_base.yaml
  # ... bestehende Pakete ...
  mqtt_integration: !include packages/integration/mqtt.yaml    # <-- Diese Zeile hinzufügen
```

### Schritt 3: Kompilieren und Flashen

```bash
esphome compile ventosync.yaml
esphome upload ventosync.yaml --device <IP>
```

---

## 📡 Beispiel MQTT-Topics

Nach der Aktivierung publiziert das Gerät Status-Updates unter dem `topic_prefix`, das automatisch aus der ESP-NOW Adressierung (Stockwerk/Raum/Geräte-ID) generiert wird. Für ein Gerät mit `floor_id: 1`, `room_id: 1`, `device_id: 1` lautet das Topic-Präfix `ventosync/1/1/1`:

| Entitätstyp | Topic | Beispiel-Payload |
|---|---|---|
| **Sensor** (CO2) | `ventosync/1/1/1/sensor/scd41_co2/state` | `823` |
| **Sensor** (Temperatur) | `ventosync/1/1/1/sensor/scd41_temperature/state` | `22.4` |
| **Select** (Modus) | `ventosync/1/1/1/select/luefter_modus/state` | `Smart-Automatik` |
| **Switch** (Kindersicherung) | `ventosync/1/1/1/switch/kindersicherung/state` | `ON` |
| **Number** (Lüfterstufe) | `ventosync/1/1/1/number/fan_intensity_display/state` | `5` |
| **Fan** (Lüfter) | `ventosync/1/1/1/fan/ventosync_hrv/state` | `ON` |
| **Binary Sensor** (Filteralarm) | `ventosync/1/1/1/binary_sensor/filter_change_alarm/state` | `OFF` |
| **Text Sensor** (Richtung) | `ventosync/1/1/1/text_sensor/direction_display/state` | `Zuluft (Rein)` |
| **Verfügbarkeit** | `ventosync/1/1/1/status` | `online` |

Befehls-Topics für steuerbare Entitäten folgen dem Muster `ventosync/<floor>/<room>/<device>/<domain>/<entity_id>/command`.

---

## 🏠 Multi-Geräte Setups

Die Trennung der Topics mehrerer Geräte erfolgt **vollautomatisch** ohne zusätzliche Konfiguration.

Das MQTT `topic_prefix` nutzt dieselben Stockwerk-/Raum-/Geräte-ID-Substitutionen (`${floor_id}`, `${room_id}`, `${device_id}`), die bereits für die ESP-NOW Gruppenkoordination konfiguriert sind. Da jedes Gerät eine eindeutige Adresskombination besitzt, sind auch die MQTT-Topics garantiert kollisionsfrei.

**Beispiel:** Drei Geräte in zwei Räumen erzeugen getrennte Topic-Bäume:

| Gerät | Stockwerk | Raum | Geräte-ID | Topic-Präfix |
|--------|-------|------|-----------|--------------|
| Büro Gerät 1 | 1 | 1 | 1 | `ventosync/1/1/1/...` |
| Büro Gerät 2 | 1 | 1 | 2 | `ventosync/1/1/2/...` |
| Schlafzimmer | 1 | 2 | 1 | `ventosync/1/2/1/...` |

> [!TIP]
> Du kannst alle Geräte eines Stockwerks mit `ventosync/1/#`, alle Geräte eines Raums mit `ventosync/1/1/#` oder alle VentoSync-Geräte mit `ventosync/#` abonnieren.

---

## ⚙️ Broker-Konfigurationsbeispiele

### Mosquitto via Home Assistant Add-on

1. Installiere das **Mosquitto broker** Add-on in Home Assistant
2. Erstelle einen dedizierten MQTT-Benutzer unter **Einstellungen > Personen > Benutzer** (z.B. `ventosync_mqtt`)
3. Trage die Zugangsdaten in deine `secrets.yaml` ein:
   ```yaml
   mqtt_broker: "homeassistant.local"
   mqtt_port: "1883"
   mqtt_username: "ventosync_mqtt"
   mqtt_password: "dein_sicheres_passwort"
   ```

### Eigenständiges Mosquitto

Beispiel `mosquitto.conf` ACL-Eintrag für VentoSync:

```
# Lese-/Schreibzugriff für VentoSync auf den gesamten Baum
user ventosync
topic readwrite ventosync/#

# Oder beschränkt auf ein bestimmtes Gerät (Stockwerk 1, Raum 1, Gerät 1):
# topic readwrite ventosync/1/1/1/#
```

---

## ❓ Häufige Fragen (FAQ)

### Warum ist `discovery: false` erforderlich?

Ohne `discovery: false` würde die MQTT-Komponente von ESPHome Home-Assistant-Discovery-Nachrichten für jede Entität senden. Da VentoSync bereits die native ESPHome API nutzt (`packages/integration/homeassistant.yaml`), würden dadurch **doppelte Entitäten** entstehen. Das Deaktivieren der Discovery verhindert dies zuverlässig.

### Kann ich MQTT auch ohne Home Assistant nutzen (keine native API)?

Ja. In diesem Fall empfiehlt es sich, `reboot_timeout: 0s` im `api:`-Block zu setzen, damit das Gerät nicht alle 15 Minuten neustartet, wenn keine HA-Instanz verbunden ist:

```yaml
# In deiner Hardware-YAML ergänzen:
api:
  reboot_timeout: 0s
```

Die `api:`-Komponente wird weiterhin für OTA-Updates und den Webserver benötigt.

### Beeinflusst MQTT die Systemleistung?

Die MQTT-Komponente benötigt minimal RAM und Netzwerktraffic. Selbst bei der ressourcenschonenden `nosensor`-Variante hat dies keinen spürbaren Einfluss auf das Timing der PID-Regelschleife. Über den Diagnosesensor `Freier Speicher (RAM)` kann dies nach Aktivierung überprüft werden.
