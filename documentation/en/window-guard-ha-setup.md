# 🏠 Home Assistant Configuration: Window Guard

[![Language: DE](https://img.shields.io/badge/Language-DE-red.svg)](../de/window-guard-ha-setup.md)


The **Window Guard** feature automatically pauses all ventilation units in a room when windows are opened to prevent heat loss and energy waste.

---

## ✨ Features & Behavior

- ⏱️ **Smart Pause (5s Delay)**: The guard engages after 5 seconds of continuous "open" state to prevent accidental triggers when briefly checking a window. All VentoSync units in the room immediately stop their fans to prevent energy waste.
- 🔄 **Automatic Resume**: The system preserves its current operating mode (e.g., Automatic, Heat Recovery, etc.) and resumes operation seamlessly as soon as all windows are closed.
- 🔆 **Visual Feedback (35s Limit)**: A distinct pulsing pattern on the Master LED (1s ON, 2s OFF) indicates the "Paused by Window" state. To avoid light pollution at night, the pulsing starts after 5 seconds and stops after 35 seconds while the fan remains safely stopped.
- 📊 **HA Status Entity**: A dedicated binary sensor (`binary_sensor.fenstersperre_aktiv`) provides real-time visibility of the lock status in Home Assistant.
- 🎛️ **Per-Device Bypass Switch**: Includes an **"Ignore Window Guard" switch** (`switch.ignore_window_guard` / `switch.fenstersperre_ignorieren`) to bypass the lock for specific individual units if needed.

---

## 🛠️ Home Assistant Setup

To integrate multiple window sensors into the VentoSync Window Guard for a specific room (e.g., **Room 1**), create a **Binary Sensor Group** in Home Assistant. This group bundles the sensors into a single entity that the firmware monitors.

**Default Entity ID for Room 1:** `binary_sensor.ventosync_window_lock_room_1`

### Option A: Via the User Interface (Recommended)
1. Go to **Settings** > **Devices & Services** > **Helpers**.
2. Click **Create Helper** > **Group** > **Binary Sensor Group**.
3. **Name**: `VentoSync Window Lock Room 1`
4. **Members**: Add all your window contacts (e.g., `binary_sensor.window_office_contact`).
5. **All Entities Status**: Set to **"Any entity"** (Default – if *any* window is open, the group is `on`).
6. **Entity ID**: Manually change this to `ventosync_window_lock_room_1`.

### Option B: Via `configuration.yaml`
Add the following code to your Home Assistant configuration:

```yaml
binary_sensor:
  - platform: group
    name: "VentoSync Window Lock Room 1"
    unique_id: ventosync_window_lock_room_1
    device_class: window
    entities:
      - binary_sensor.window_office_contact_1
      - binary_sensor.window_office_contact_2
```
