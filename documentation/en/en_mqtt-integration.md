# 🔌 MQTT Integration for External Systems

[![Language: DE](https://img.shields.io/badge/Language-DE-red.svg)](../de/de_mqtt-integration.md)


VentoSync supports optional MQTT publishing for integration with **Node-RED**, **openHAB**, **ioBroker**, **IP-Symcon**, and any other MQTT-based automation platform — without affecting the native Home Assistant integration.

---

## ✨ Overview

The MQTT integration package (`packages/integration/mqtt.yaml`) adds ESPHome's built-in MQTT component alongside the existing native API. All registered entities (sensors, selects, numbers, switches, buttons, fans, text sensors, binary sensors, lights) are automatically published to an MQTT broker — no per-entity configuration required.

**Key design decisions:**

- **Additive, not replacing**: The native Home Assistant API (`api:`) remains active and unmodified. MQTT is a second, independent publishing channel.
- **`discovery: false`**: MQTT discovery is explicitly disabled to prevent ESPHome from auto-registering entities in Home Assistant via MQTT, which would create **duplicate entities** alongside the existing native API integration.
- **Opt-in only**: The MQTT package is never included by default in any hardware variant. Users must explicitly add it.

---

## 🛠️ Setup

### Prerequisites

- A running MQTT broker (e.g., [Mosquitto](https://mosquitto.org/) via the Home Assistant add-on, or a standalone instance)
- Broker credentials (username/password)

### Step 1: Add MQTT secrets

In your `secrets.yaml` (see `secrets_example.yaml` for the template), add:

```yaml
mqtt_broker: "192.168.1.100"     # IP or hostname of your MQTT broker
mqtt_port: "1883"                # Default MQTT port
mqtt_username: "your_username"
mqtt_password: "your_password"
```

### Step 2: Enable the MQTT package

Add a single line to the `packages:` section of your variant YAML (e.g., `ventosync.yaml`, `ventosync_nosensor.yaml`, etc.):

```yaml
packages:
  base: !include packages/base/ventosync_base.yaml
  # ... existing packages ...
  mqtt_integration: !include packages/integration/mqtt.yaml    # <-- Add this line
```

### Step 3: Compile and flash

```bash
esphome compile ventosync.yaml
esphome upload ventosync.yaml --device <IP>
```

---

## 📡 Example MQTT Topics

After enabling MQTT, the device publishes state updates to topics under the `topic_prefix`, which is automatically derived from the device's existing ESP-NOW addressing (Floor/Room/Device ID). For a device configured with `floor_id: 1`, `room_id: 1`, `device_id: 1`, the topic prefix is `ventosync/1/1/1`:

| Entity Type | Topic | Example Payload |
|---|---|---|
| **Sensor** (CO2) | `ventosync/1/1/1/sensor/scd41_co2/state` | `823` |
| **Sensor** (Temperature) | `ventosync/1/1/1/sensor/scd41_temperature/state` | `22.4` |
| **Select** (Mode) | `ventosync/1/1/1/select/luefter_modus/state` | `Smart-Automatik` |
| **Switch** (Child Lock) | `ventosync/1/1/1/switch/kindersicherung/state` | `ON` |
| **Number** (Fan Intensity) | `ventosync/1/1/1/number/fan_intensity_display/state` | `5` |
| **Fan** (HRV Fan) | `ventosync/1/1/1/fan/ventosync_hrv/state` | `ON` |
| **Binary Sensor** (Filter Alarm) | `ventosync/1/1/1/binary_sensor/filter_change_alarm/state` | `OFF` |
| **Text Sensor** (Direction) | `ventosync/1/1/1/text_sensor/direction_display/state` | `Zuluft (Rein)` |
| **Availability** | `ventosync/1/1/1/status` | `online` |

Command topics follow the pattern `ventosync/<floor>/<room>/<device>/<domain>/<entity_id>/command` for controllable entities.

---

## 🏠 Multi-Unit Setups

Multi-unit topic isolation works **automatically** — no additional configuration required.

The MQTT `topic_prefix` uses the same Floor/Room/Device ID substitutions (`${floor_id}`, `${room_id}`, `${device_id}`) that you already configure per device for ESP-NOW group coordination. Since every device in a VentoSync installation must have a unique address combination, the MQTT topics are guaranteed to be unique as well.

**Example:** Three units across two rooms produce fully isolated topic trees:

| Device | Floor | Room | Device ID | Topic Prefix |
|--------|-------|------|-----------|--------------|
| Office Unit 1 | 1 | 1 | 1 | `ventosync/1/1/1/...` |
| Office Unit 2 | 1 | 1 | 2 | `ventosync/1/1/2/...` |
| Bedroom | 1 | 2 | 1 | `ventosync/1/2/1/...` |

> [!TIP]
> You can subscribe to all devices on a floor with `ventosync/1/#`, all devices in a room with `ventosync/1/1/#`, or all VentoSync devices with `ventosync/#`.

---

## ⚙️ Broker Configuration Examples

### Mosquitto via Home Assistant Add-on

1. Install the **Mosquitto broker** add-on in Home Assistant
2. Create a dedicated MQTT user under **Settings > People > Users** (e.g., `ventosync_mqtt`)
3. Add the credentials to your `secrets.yaml`:
   ```yaml
   mqtt_broker: "homeassistant.local"
   mqtt_port: "1883"
   mqtt_username: "ventosync_mqtt"
   mqtt_password: "your_secure_password"
   ```

### Standalone Mosquitto

Example `mosquitto.conf` ACL entry for VentoSync:

```
# Allow VentoSync read/write on its own topic tree
user ventosync
topic readwrite ventosync/#

# Or restrict to a specific device (Floor 1, Room 1, Device 1):
# topic readwrite ventosync/1/1/1/#
```

---

## ❓ FAQ

### Why is `discovery: false` required?

Without `discovery: false`, ESPHome's MQTT component would publish Home Assistant MQTT discovery messages for every entity. Since VentoSync already uses the native ESPHome API for Home Assistant integration (`packages/integration/homeassistant.yaml`), this would create **duplicate entities** — one set from the native API, another set from MQTT discovery. Disabling discovery prevents this.

### Can I use MQTT without Home Assistant (no native API)?

Yes, but you should set `reboot_timeout: 0s` in the `api:` block to prevent the device from rebooting every 15 minutes when no HA instance connects:

```yaml
# In your variant YAML, add:
api:
  reboot_timeout: 0s
```

The `api:` component is still required for OTA updates and the web server to function correctly.

### Does enabling MQTT affect performance?

The MQTT component adds a small amount of RAM usage and network traffic. For the `nosensor` variant (the lightest resource budget), this is minimal and should not impact PID control loop timing. Monitor the `Freier Speicher (RAM)` diagnostic sensor after enabling MQTT to verify.
