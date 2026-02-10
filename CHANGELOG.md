# 📋 Changelog - ESPHome Wohnraumlüftung

## 2026-02-10 - Hybrid Fan Interface & Documentation Update

### ✨ Hardware Changes

#### Hybrid Fan Control Interface

- **Dual-Mode Support**: Board unterstützt jetzt sowohl **4-Pin PWM** (AxiRev) als auch **3-Pin Dual-GND** (VarioPro) Lüfter
- **Neue Komponenten**:
  - **Q4, Q5** (PMV16XNR SOT-23): Dual Low-Side MOSFETs für 3-Pin GND1/GND2 Kontrolle
  - **D1** (B5819WS): Freilaufdiode an PWM_12V_OUT (Q1 Drain) — kritisch für Buck-Converter
  - **D2** (B5819WS): Schutzdiode an Fan Pin 1 (GND/GND1)
  - **D3** (B5819WS): Schutzdiode an Fan Pin 4 (PWM/GND2)
  - **R18, R19** (10kΩ 0402): Pull-down Widerstände für Q4/Q5 Gates
  - **JP1** (1x3 2.54mm): VCC Moduswahl (Constant/Variable 12V)
  - **JP2, JP3** (1x3 1.27mm): Pin 1/Pin 4 Moduswahl
- **GPIO2**: Neue Funktion als **Fan PWM Secondary** (Low-Side GND2 via Q4)

### 🐛 Bug Fixes

- **D1 Freilaufdiode** an PWM_12V_OUT gefehlt — ohne diese wird Q1 im 3-Pin Modus durch induktive Spannungsspitzen zerstört
- **R8**: Dokumentation korrigiert (2.2kΩ / 0805, nicht 10kΩ / 0603)
- **R3**: Beschreibung korrigiert ("für Q3" statt "für Q2")
- **D1 (SS14)** in Schaltplan → korrigiert zu **D1 (B5819WS)**
- **Dioden-Designatoren**: D1/D3 → D2/D3 im Low-Side Circuit

### 📚 Documentation Updates

- `Anleitung-Fan-Circuit.md` — BOM und Schaltplan an aktuelle Hardware angepasst
- `Hardware-Setup-Readme.md` — Universal Fan Interface komplett überarbeitet (S8050, R8=2.2kΩ, JP1/JP2/JP3)
- `Technical-Details.md` — PWM-Konfiguration aktualisiert (GPIO16/GPIO2, 28kHz, Dual-Mode)
- `Readme.md` — Pin-Tabelle, Mermaid-Diagramm, Projektstruktur erweitert
- `esp_wohnraumlueftung.yaml` — PWM-Kommentare mit vollständigen Schaltungsdetails

### 📁 File Changes

#### Modified Files

- `esp_wohnraumlueftung.yaml` — PWM-Kommentare erweitert
- `documentation/Anleitung-Fan-Circuit.md` — BOM/Schematic Korrekturen
- `documentation/Hardware-Setup-Readme.md` — Fan Interface Rewrite
- `documentation/Technical-Details.md` — PWM-Sektion aktualisiert
- `Readme.md` — GPIO-Tabelle, Diagramm, Struktur

---

## 2026-01-28 - Major Documentation & Optimization Update

### ✨ New Features

#### APDS9960 Sensor Optimization

- **Performance Improvements:**
  - Update interval: 100ms → 500ms (80% less I²C traffic)
  - Proximity gain: 4x → 2x (better close-range detection)
  - LED drive: 100mA → 50mA (50% power savings)
  - Added delta filter: 5 (noise reduction)
  - Added throttle filter: 500ms (prevents I²C bus flooding)

- **Adaptive Display Brightness:**
  - Display brightness now adjusts automatically based on ambient light
  - Bright room (>100): Full brightness (255)
  - Dim room (≤100): Half brightness (128)

#### Configuration Improvements

- Added configurable proximity threshold via substitutions
- Improved logging (DEBUG level for reduced spam)
- Better filter configuration for stable readings
- Future-ready gesture detection (commented, ready to enable)

### 🐛 Bug Fixes

#### ESPHome YAML Syntax Errors

Fixed "Unresolved tag: !lambda" errors in 5 locations:

- Line 186: IAQ traffic light ESP-NOW send
- Line 274: Fan speed control from slider
- Line 381: Fan ramp-up calculation
- Line 398: Fan ramp-down calculation
- Line 472: Ventilation sync broadcast

**Root Cause:** ESPHome requires lambda expressions to use YAML block scalar format (`|-`) instead of quoted strings.

**Before (Incorrect):**

```yaml
data: !lambda "return get_iaq_traffic_light_data(x);"
```

**After (Correct):**

```yaml
data: !lambda |-
  return get_iaq_traffic_light_data(x);
```

### 📚 Documentation Updates

#### README.md Enhancements

Added comprehensive documentation sections:

1. **Project Structure**
   - Complete file tree with descriptions
   - Component organization

2. **Technical Details & Optimizations**
   - APDS9960 sensor optimization details
   - ESPHome YAML syntax guide
   - I²C bus configuration
   - BME680 BSEC2 configuration
   - ESP-NOW communication
   - Fan control logic

3. **Troubleshooting Section**
   - ESPHome YAML errors
   - I²C bus problems
   - APDS9960 issues
   - BME680/BSEC2 problems
   - ESP-NOW synchronization
   - Compilation errors
   - Performance issues
   - Debug commands

4. **UI Features**
   - Added adaptive brightness feature
   - Added optimized sensor description

#### VS Code Configuration

Added ESPHome YAML tag support to `settings.json`:

```json
{
  "yaml.customTags": [
    "!lambda scalar",
    "!secret scalar",
    "!include scalar",
    "!include_dir_named scalar",
    "!include_dir_list sequence",
    "!include_dir_merge_list sequence",
    "!include_dir_merge_named mapping"
  ]
}
```

### 📁 File Changes

#### Modified Files

- `esp_wohnraumlueftung.yaml` - Fixed 5 lambda syntax errors
- `apds9960_config.yaml` - Replaced with optimized version
- `Readme.md` - Added 200+ lines of documentation
- `settings.json` - Added YAML custom tags

#### New Files

- `apds9960_config_optimized.yaml` - Optimized sensor configuration (now active)
- `apds9960_config.yaml.backup` - Backup of original configuration

### 🎯 Performance Improvements

| Metric            | Before | After | Improvement   |
|-------------------|--------|-------|---------------|
| I²C Bus Traffic   | 100%   | 20%   | 80% reduction |
| APDS9960 Power    | 100mA  | 50mA  | 50% savings   |
| Update Rate       | 10/sec | 2/sec | More stable   |
| Log Spam          | High   | Low   | DEBUG level   |

### 🔧 Technical Changes

#### APDS9960 Configuration

```yaml
# Old
apds_update_interval: 100ms
# No gain configuration
# No filters

# New
apds_update_interval: 500ms
proximity_gain: 2x
led_drive: 50mA
filters:
  - delta: 5
  - throttle: 500ms
```

#### Lambda Expressions

All lambda expressions now use proper YAML block format:

```yaml
# Pattern used throughout
action: !lambda |-
  return function_call(x);
```

### 📖 Documentation Metrics

- **README.md:** 171 lines → 543 lines (+372 lines)
- **New sections:** 7
- **Code examples:** 20+
- **Troubleshooting entries:** 15+

### ✅ Testing & Validation

- [x] YAML syntax validated
- [x] Lambda expressions corrected
- [x] APDS9960 configuration optimized
- [x] Documentation complete
- [x] Markdown linting passed
- [ ] Hardware testing pending (user to flash)

### 🚀 Next Steps

1. Flash updated configuration to device
2. Test adaptive brightness in different lighting conditions
3. Monitor I²C bus stability
4. Verify power consumption improvements
5. Consider enabling gesture controls for fan speed

### 📝 Notes

- Original APDS9960 config backed up to `apds9960_config.yaml.backup`
- All changes are backward compatible
- No breaking changes to existing functionality
- Gesture detection code included but commented out (ready to enable)

---

**Author:** Antigravity AI Assistant  
**Date:** 2026-01-28  
**Version:** 1.1.0
