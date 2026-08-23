# VentoSync — Claude Code Context

## Project Overview

VentoSync is an advanced ESPHome-based smart heat recovery ventilation (HRV) controller
for the VentoMaxx V-WRG Series. It runs on a custom PCB with a **Seeed XIAO ESP32-C6**
and replaces the proprietary VentoMaxx control unit entirely.

**Key facts:**
- Platform: ESP32-C6 (RISC-V), ESPHome
- Hardware: Custom PCB with Traco power supply, MCP23017 GPIO expander, PCA9685 LED driver
- Sensors: SCD41 (CO2), BME680 (IAQ fallback), BMP390 (pressure), 2× NTC, HLK-LD2450 (mmWave radar)
- Communication: ESP-NOW for multi-device sync (no Wi-Fi required between units), ESPHome Native API to Home Assistant
- Language policy: **Code comments and all internal documentation in English.** HA entity names, UI labels, and user-facing strings remain in **German**.
- License: GPL v3
- Current version: see `version.json` (single source of truth, auto-bumped by `version_bump.py`)

---

## Repository Structure

```
VentoSync/
├── ventosync.yaml                  # Full variant (SCD41 + BME680 + LD2450)
├── ventosync_bme680_only.yaml      # Fallback: BME680, no SCD41, no radar
├── ventosync_radar_only.yaml       # Radar only, no climate sensors
├── ventosync_nosensor.yaml         # Basic fan control, no sensors
├── packages/
│   └── base/
│       └── ventosync_base.yaml     # Shared core (HW pins, UI, network, fan) — all variants include this
│   ├── actuators/                  # Fan logic, automation, safety shutdown
│   ├── sensors/                    # sensor_SCD41.yaml, sensor_BME680.yaml, sensor_LD2450.yaml, sensor_NTC.yaml, mock_*.yaml
│   ├── io/                         # GPIO expander, LED driver
│   ├── ui/                         # ui_lights.yaml, ui_diagnostics.yaml
│   ├── integration/                # homeassistant.yaml (HA-specific data points, isolated from core logic)
│   └── globals_*.yaml              # globals_ventilation / _automation / _ui / _network
├── components/                     # Custom ESPHome C++ components
│   ├── VentilationController       # Core state machine
│   ├── VentilationStateMachine
│   ├── VentilationLogic
│   ├── WrgDashboard                # Local web dashboard (Tailwind CSS + Chart.js via CDN)
│   └── helpers/                    # auto_mode.h, espnow_helpers.h, config_helpers.h,
│                                   # ha_fan_helpers.h, user_input.h, system_boot_helpers.h,
│                                   # led_feedback.h, ventilation_group.h, health_helpers.h,
│                                   # vacation_helpers.h, network_sync.h, system_lifecycle.h
├── tests/                          # Unit tests (compiled with -std=c++17 -fsanitize=address,undefined)
├── .github/workflows/              # build.yaml (multi-variant matrix), lint.yaml, release pipeline
├── ESPHome-VentoMaxx-Analyser/
├── EasyEDA-Pro/                    # PCB design files, BOM, schematics
├── documentation/                  # Entities_Documentation.md, Comparison-VentoMaxx.md, etc.
├── ha_integration_example/         # HA YAML examples, dashboard cards
├── version.json                    # Single source of truth for firmware version
├── version_bump.py                 # Auto-version bump script (called from CI)
├── upload_all.sh                   # Bulk OTA upload to all devices
├── secrets_example.yaml            # Template — actual secrets.yaml is gitignored
└── CHANGELOG.md                    # Keep a Changelog format, Semantic Versioning
```

---

## Hardware Variants & Build Flags

Each top-level YAML is a thin wrapper that includes `ventosync_base.yaml` plus sensor packages.
C++ preprocessor flags control conditional compilation in `globals.h`:

| Variant                     | Flags                                                      |
|-----------------------------|------------------------------------------------------------|
| `ventosync.yaml`            | (none — all sensors present)                               |
| `ventosync_bme680_only.yaml`| `-DVENTOSYNC_NO_SCD41`                                     |
| `ventosync_radar_only.yaml` | `-DVENTOSYNC_NO_CLIMATE`                                   |
| `ventosync_nosensor.yaml`   | `-DVENTOSYNC_NO_SCD41 -DVENTOSYNC_NO_BME680 -DVENTOSYNC_NO_RADAR` |

Missing sensors are replaced by `mock_*.yaml` packages that return clean `NaN`/`false` values.
Never add real sensor YAML without the corresponding mock for the fallback variants.

---

## ESPHome Commands (use in VS Code integrated terminal)

```bash
# Activate Python venv first
source venv/bin/activate

# Validate config (no compilation)
esphome config ventosync.yaml
esphome config ventosync_nosensor.yaml

# Compile only (generates .bin, no upload)
esphome compile ventosync.yaml

# Compile + OTA upload
esphome run ventosync.yaml --device <IP> --no-logs

# Upload only (if already compiled)
esphome upload ventosync.yaml --device <IP> --no-logs

# Bulk upload to all devices
./upload_all.sh

# Initial USB flash
esphome run ventosync.yaml --device /dev/ttyACM0
```

Always validate before uploading. The CI (`build.yaml`) pins ESPHome to `2026.5.0` —
use the same version locally to avoid config drift.

---

## Operating Modes

| Mode              | German name          | HA entity value   | LED_WRG | LED_VEN |
|-------------------|----------------------|-------------------|---------|---------|
| Smart automatic   | `Smart-Automatik`    | `Smart-Automatik` | pulses  | off     |
| Heat recovery     | `Wärmerückgewinnung` | `Eco Recovery`    | on      | off     |
| Cross-ventilation | `Durchlüften`        | `Ventilation`     | on      | on      |
| Boost ventilation | `Stoßlüftung`        | `Boost Ventilation` | off   | on      |
| Off               | `Aus`                | `Off`             | off     | off     |

**Critical:** The string `"Smart-Automatik"` must match exactly in both YAML select options
and all C++ code (`system_lifecycle.h`, `network_sync.h`, `user_input.h`).
Mismatch causes silent mode-switch failures (see CHANGELOG 0.8.169).

---

## ESP-NOW Protocol

- Protocol version: v4, magic header `0x42`
- Discovery: broadcast `ROOM_DISC` on boot → matching Floor+Room ID → unicast handshake
- Peers stored in NVS; max ~14 peers per device (254-char string limit in ESPHome Globals)
- LRU eviction at 10 entries in `VentilationController` to prevent heap fragmentation
- `VentilationMode` enum has explicit `uint8_t` underlying type — **do not renumber enum values**, they are serialized over the wire
- Heartbeat interval: `sync_interval_config` (default 1 min, was 3 min)
- Peer timeout: `PEER_TIMEOUT_MS = 900000` (15 min)

---

## C++ Architecture & Coding Rules

### Namespaces
Use `ventosync::` namespace for all new constants and helpers.
Sub-namespaces in use: `ventosync::vacation`, `ventosync::health`, `ventosync::config`, `ventosync::hw`.

### Header files (`components/helpers/`)
Complex YAML lambda logic is extracted to `.h` files. Each header has a single responsibility:
- `auto_mode.h` — PID demand calculation, absolute humidity guard
- `espnow_helpers.h` — `dispatch_espnow_packet()`, `RxSource` enum
- `config_helpers.h` — `update_config_id()`, `ConfigField` enum, `build_config_summary()`
- `ha_fan_helpers.h` — HA fan entity sync, guard logic (`GUARD_PROPAGATION_MS`, `GUARD_STUCK_TIMEOUT_MS`)
- `user_input.h` — `toggle_child_lock()`, `flash_all_leds_on()`, `restore_leds_after_flash()`
- `led_feedback.h` — null-checked LED access via `led_guard` helpers
- `system_boot_helpers.h` — `init_external_antenna()`, `boot_discovery_broadcast()`, `boot_finish_discovery()`
- `ventilation_group.h` — `VentilationController`, peer management, `on_packet_received()`
- `network_sync.h` — ESP-NOW mode/fan sync, mode string mapping

### Type safety rules
- Use `static_cast<>` everywhere; no C-style casts
- Prefer `constexpr` over `const` for compile-time constants
- Add `static_assert` for critical constants (sizes, ranges, enum underlying types)
- Use `std::clamp` for all float/int bounds enforcement — never silent overflow
- ESP-NOW packet deserialization: use `std::memcpy` to a stack-local struct, **never** cast `uint8_t*` directly to a struct pointer (strict aliasing UB)
- `const std::vector<uint8_t>&` for packet receive parameters — no unnecessary heap copies

### ESPHome-specific patterns
- Always call `->publish_state()` after direct state mutation; never mutate `->state` without publishing
- Use `make_call()` for fan state changes, not direct state assignment
- `internal: true` on HA entities that depend on sensors absent in some variants
- Template sensors that read C++ globals need explicit `update_interval: 1s` or `5s` — without it they fall back to 60s polling

### NVS / Flash wear
- Filter runtime: accumulate in RAM, write to NVS every **8 hours** max (was 30 min → caused 1440 writes/day)
- BME680 baseline counter: RAM-based; flash save gated by `save_interval_ms`
  (1h minimum) and `save_delta_pct` (2% change) in `bme680_iaq_engine.h`
- `restore_mode` native for UI switches where possible (no extra global variable)

---

## PID Controller (Smart-Automatik mode)

- CO2 PID: `kp = 0.001`, `ki = 0.0000005` (very slow integral — level transitions every 20-30 min)
- Humidity PID: `kp = 0.05`, `ki = 0.00001`
- Fan demand = `std::max(co2_demand, humidity_demand)` — neither signal is suppressed
- Fan changes by **at most ±1 level per 10-second cycle**
- Min level: `automatik_min_luefterstufe` (default 2), Max: `automatik_max_luefterstufe` (default 7)
- Absolute humidity guard (Magnus formula): humidity demand is forced to zero when outdoor air
  is more humid than indoor air. Requires `sensor.outdoor_humidity` in Home Assistant.

---

## Fan PWM Curve (VentoMaxx V-curve)

50% PWM = standstill. Direction A: 50% → 5% (increasing speed). Direction B: 50% → 95%.

| Level | PWM Dir A | PWM Dir B | RPM (approx.) |
|-------|-----------|-----------|---------------|
| OFF   | 50.0%     | 50.0%     | 0             |
| 1     | 30.0%     | 70.0%     | 420           |
| 5     | 18.9%     | 81.1%     | 1680          |
| 10    | 5.0%      | 95.0%     | 4200          |

Slew-rate limiter: ~5% per second, 1-second call interval.
Direction change includes a 5-second brake + soft-start ramp.

---

## CI / GitHub Actions

- `build.yaml`: Matrix build of all 4 variants, pinned to ESPHome `2026.5.0`
- `lint.yaml`: YAML validation + C++ checks with `-std=c++17 -Wall -Wextra -Wpedantic -fsanitize=address,undefined`
- Release: tag → OTA `.bin` files + `manifest.json` as GitHub Release assets
- Secrets stripped from release binaries — devices use NVS-stored Wi-Fi credentials
- pip deps and PlatformIO toolchains are cached

---

## Files to Never Modify / Commit

- `secrets.yaml` — gitignored, use `secrets_example.yaml` as template
- `build.log` — gitignored build artifact
- `.version_bump_lock` — deleted automatically before each release build

---

## CHANGELOG Convention

Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), Semantic Versioning.
Sections: `Added`, `Changed`, `Fixed`, `Removed`, `Security / Stability`.
Security items are tagged **K-n** (Kritisch/Critical), **H-n** (High), **M-n** (Medium).
Version is bumped automatically by `version_bump.py` — do not edit `version.json` manually.

---

## Documentation

All user-facing documentation exists in both English (`Readme.md`) and German (`Readme_de.md`).
When making changes that affect behavior, update both README files and `CHANGELOG.md`.
Entity reference: `documentation/Entities_Documentation.md`.
