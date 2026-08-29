# VentoSync – Home Assistant Lovelace Dashboard

[![Language: DE](https://img.shields.io/badge/Language-DE-red.svg)](../de/de_ha-lovelace-dashboard.md)

## Overview

This document describes the modular Lovelace dashboard design for **VentoSync** devices in Home Assistant. The dashboard utilizes a reusable **Decluttering Template** that can be instantiated for any number of ventilation units with just 3 lines of YAML per device.

![Dashboard Preview](../screenshots/Control-Dashboard1.png)

---

## Prerequisites

### HACS Custom Cards

The following custom cards must be installed via [HACS](https://hacs.xyz/) (Frontend):

| Card | Repository | Purpose |
|------|-----------|---------|
| **Mushroom** | [piitaya/lovelace-mushroom](https://github.com/piitaya/lovelace-mushroom) | Template, Chips, Select, and Number cards |
| **Mini Graph Card** | [kalkih/mini-graph-card](https://github.com/kalkih/mini-graph-card) | Historical trend graphs |
| **Card Mod** | [thomasloven/lovelace-card-mod](https://github.com/thomasloven/lovelace-card-mod) | Dynamic CSS animations and styling |
| **Stack In Card** | [custom-cards/stack-in-card](https://github.com/custom-cards/stack-in-card) | Seamless card grouping |
| **Decluttering Card** | [jcwillox/lovelace-decluttering-card](https://github.com/jcwillox/lovelace-decluttering-card) | Reusable UI templates |

### System Requirements

- Home Assistant **2024.1** or newer
- At least one configured VentoSync device via ESPHome
- Standardized entity naming scheme (generated automatically by ESPHome)

---

## Installation

### 1. Install HACS Cards
In HACS → Frontend → "+ Explore & Download Repositories", search and install: `mushroom`, `mini-graph-card`, `card-mod`, `stack-in-card`, and `decluttering-card`.

### 2. Clear Browser Cache
Force reload your browser cache (`Ctrl + Shift + R` on Windows/Linux, `Cmd + Shift + R` on macOS).

### 3. Configure Dashboard
1. Open your Lovelace Dashboard → Top right menu `⋮` → **Raw Configuration Editor**.
2. Add the `decluttering_templates:` block **before** the `views:` section.
3. In the desired view, instantiate each unit using `custom:decluttering-card`.

---

## Dashboard Architecture

```text
┌─────────────────────────────────────────────┐
│                HERO SECTION                 │
│    Animated Fan Icon + Mode + Metrics       │
│  ┌───────────────────────────────────────┐  │
│  │ Chips: Direction │ Window │ Enclosure │  │
│  └───────────────────────────────────────┘  │
├─────────────────────────────────────────────┤
│          DETAIL CARDS (4 Columns)           │
│ Speed (RPM) │ PWM Duty │ Encl Temp │ Window │
├─────────────────────────────────────────────┤
│                 MINI GRAPH                  │
│    Speed (RPM) + PWM (%) + Enclosure (°C)   │
├─────────────────────────────────────────────┤
│                 CONTROLS                    │
│          Operating Mode (Select)            │
│          Fan Level (Slider)                 │
│          LED Brightness (Slider)            │
├─────────────────────────────────────────────┤
│            SWITCHES (2 Columns)             │
│         Window Guard │ Child Protection     │
└─────────────────────────────────────────────┘
```

### Reusability Example

```yaml
decluttering_templates:
  ventosync:             # Template defined once
    card: ...            # Contains [[device]] variable

# Usage in view:
views:
  cards:
    - type: custom:decluttering-card
      template: ventosync
      variables:
        - device: ventosync_office
    - type: custom:decluttering-card
      template: ventosync
      variables:
        - device: ventosync_living_room
```

---

## Component Breakdown

### 1. Hero Section
The focal point of the dashboard. Displays the operating mode with an animated fan icon:
- **Direction-Aware Rotation:** Rotates clockwise (Intake / Zuluft) or counter-clockwise (Exhaust / Abluft).
- **RPM-Scaled Animation Speed:** Dynamically accelerates with fan speed.
- **Dynamic Color Coding:** Blue for fresh air intake, orange for exhaust, gray when idle.

### 2. Status Chips
Three color-coded status pills:
- **Direction:** `↓ Zuluft` (blue), `↑ Abluft` (orange), `⇅ Heat Recovery` (teal), `⏸ Idle` (gray).
- **Window Guard:** 🟢 Closed / 🔴 Open (ventilation paused).
- **Enclosure Temperature:** Real-time BMP390 power supply temperature.

### 3. Detail Cards
Four Mushroom template cards displaying:
- **RPM Speed:** Direction arrow and live tachometer speed.
- **PWM Duty Cycle:** Color-coded power demand.
- **Enclosure Temp:** Hardware thermal guard.
- **Window Guard:** Room window state.

### 4. Mini Graph Card
Real-time 12-minute rolling trend graph displaying RPM, PWM %, and power supply temperature.

### 5. Controls
- **Operating Mode:** Select dropdown for Smart-Automatik, Heat Recovery, Boost, Cross-Ventilation, or Off.
- **Fan Level Slider:** Manual speed control (1–10).
- **LED Brightness Slider:** Panel ambient brightness control.

### 6. Switches
- **Window Guard Bypass:** Override window sensor pause.
- **Child Lock:** Lock physical buttons on the device panel.

---

## Entity Reference

Entities follow the standardized schema `{domain}.{device}_{suffix}`:

| Domain | Suffix | Type | Description |
|--------|--------|-----|-------------|
| `select` | `_luftermodus` | Select | Operating mode |
| `number` | `_lufter_intensitat` | Number | Manual fan level |
| `number` | `_maximale_led_helligkeit` | Number | Panel LED brightness |
| `sensor` | `_lufter_drehzahl` | Sensor | Fan speed in RPM |
| `sensor` | `_lufter_pwm` | Sensor | PWM duty cycle % |
| `sensor` | `_lufter_richtung` | Sensor | Airflow direction text |
| `sensor` | `_bmp390_temperatur` | Sensor | Enclosure temperature (°C) |
| `sensor` | `_fenstersperre_aktiv` | Sensor | Window guard active state |
| `switch` | `_fenstersperre_ignorieren` | Switch | Window guard bypass switch |
| `switch` | `_kindersicherung` | Switch | Child lock toggle |
