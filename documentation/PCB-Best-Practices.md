# 🔧 PCB Best Practices — ESPHome Wohnraumlüftung

> **Stand**: 10.02.2026 | **Referenz**: [Sierra Circuits Best Practices](https://www.protoexpress.com/blog/best-electronic-circuit-design-practices/)

---

## 1. Bypass / Decoupling Caps (ESP32-C6)

**Status**: ✅ Umgesetzt — C13 (470µF / 16V) als Bulk-Cap am 5V Output.

> [!NOTE]
> C13 (470µF Elko) ist ein **Bulk-Pufferkondensator** für WiFi-Peaks. Er deckt niedrige Frequenzen ab (µs–ms Bereich). Für hochfrequentes Rauschen (MHz-Bereich) wäre **zusätzlich** ein 100nF Kerko *direkt* an den Power-Pins des XIAO sinnvoll. Falls Board-Platz knapp ist: Das XIAO-Modul hat intern bereits HF-Caps — daher als **optional** einzustufen.

---

## 2. Galvanische Trennung (High-Voltage)

**Status**: ✅ Umgesetzt — Fräsungsslot und Warnhinweis auf PCB-Rückseite vorhanden.

---

## 3. Entladewiderstände (Bleeder Resistors)

**Zweck**: Wenn die Stromversorgung getrennt wird (Netzstecker gezogen), bleiben geladene Elkos auf Spannung. Das ist:

- **Sicherheitsrisiko** bei Wartungsarbeiten (bis zu 12V an offenen Pins)
- **Funktionsproblem** bei Neustart (alte Ladung kann zu undefiniertem Verhalten führen)

### 3.1 Schaltung: Bleeder-Widerstand

```
+12V ──────┬──── Last
           │
          [R_bleeder]    ← Widerstand parallel zum Elko
           │
C1 (470µF) ┤
           │
GND ───────┘
```

### 3.2 Dimensionierung

| Kondensator | Spannung | Bleeder | Verlustleistung | Entladezeit (auf <1V) |
|:---|:---|:---|:---|:---|
| **C1** (470µF / 25V) | 12V Rail | **47kΩ / 0805** | ~3mW | ~4.4s (5τ) |
| **C13** (470µF / 16V) | 5V Rail | **47kΩ / 0805** | ~0.5mW | ~4.4s (5τ) |

**Berechnung:**

```
τ = R × C = 47000 × 0.000470 = 22.1s (1τ → 37%)
Vollständige Entladung: ~5τ = ~110s ← konservativ, da 12V→1V schneller

Genauer: t = -R·C · ln(V_final/V_initial)
         t = -47000 · 0.00047 · ln(1/12) = 22.09 · 2.485 = ~55s

Verlustleistung: P = V² / R = (12)² / 47000 = 3.06 mW ← vernachlässigbar
```

### 3.3 Umsetzung in EasyEDA Pro

1. **Schaltplan**: Jeweils einen **47kΩ Widerstand (0805)** parallel zu C1 und C13 einzeichnen
2. **PCB**: So nah wie möglich am jeweiligen Elko platzieren
3. **Designator**: `R_B1` (12V Rail), `R_B2` (5V Rail)
4. **BOM-Eintrag**: `47kΩ ±5% 0805 1/8W` — z.B. RC0805FR-0747KL

> [!TIP]
> 47kΩ ist ein guter Kompromiss: Schnell genug entladen (<1 Minute), aber nur ~3mW Dauerverlust im Betrieb. Kein messbarer Einfluss auf die Gesamteffizienz.

---

## 4. Trace-Breiten (Power-Pfade)

**Status**: 🟡 Vom User zu prüfen.

**Referenz-Werte für 1oz Kupfer (35µm), ΔT=10°C:**

| Strom | Min. Breite (extern) | Empfohlen |
|:---|:---|:---|
| 500mA | 0.25mm / 10mil | 0.5mm |
| 1A | 0.50mm / 20mil | 0.8mm |
| 2A | 1.00mm / 40mil | 1.5mm |

**Zu prüfende Pfade:**

- [ ] 12V von Netzteil → Fan VCC
- [ ] 12V → Buck-Converter (AP63203) Input
- [ ] GND-Rückführung der MOSFETs Q4, Q5 → GND-Plane
- [ ] 5V Output (Recom) → XIAO 5V Pin

---

## 5. Thermal Vias unter Power-ICs

**Zweck**: Power-ICs (AP63203, MOSFETs) erzeugen Wärme. Thermal Vias leiten diese Wärme durch das PCB zum Kupfer-Pour auf der Rückseite, was die effektive Kühlfläche vergrößert.

### 5.1 Welche Bauteile brauchen Thermal Vias?

| Bauteil | Package | Grund | Anzahl Vias |
|:---|:---|:---|:---|
| **U37** (AP63203WU-7) | TSOT-26 | Buck-Converter, ~85% Effizienz, Wärme-Pad | 3–5 Vias |
| **Q5** (PMV16XNR) | SOT-23 | Low-Side MOSFET, bis 1A Lüfterstrom | 2–3 Vias |
| **Q4** (PMV16XNR) | SOT-23 | Low-Side MOSFET, bis 1A Lüfterstrom | 2–3 Vias |
| **Q1** (AO3401) | SOT-23 | P-MOSFET High-Side Switch | 2–3 Vias |

### 5.2 Anleitung: Thermal Vias in EasyEDA Pro

#### Schritt 1: Via-Größe festlegen

- **Via-Durchmesser**: 0.6mm (Pad) / 0.3mm (Bohrloch)
- **Typ**: Plated Through-Hole (Standard-Via)

#### Schritt 2: Vias platzieren

1. Öffne das **PCB-Layout** in EasyEDA Pro
2. Wähle das **Via-Tool** (Shortcut: `V`)
3. Setze die Via-Eigenschaften:
   - **Hole Size**: 0.3mm
   - **Via Diameter**: 0.6mm
   - **Net**: `GND` (oder das entsprechende Thermal-Pad-Netz)
4. Platziere die Vias in einem **Raster-Muster** unter dem IC:

```
    ┌─────────────────┐
    │    AP63203       │
    │  Exposed Pad     │
    │   ○  ○  ○       │   ← 0.6mm Vias, 1mm Raster
    │   ○  ○  ○       │
    │                  │
    └─────────────────┘
```

#### Schritt 3: Thermal Relief einstellen (wichtig!)

1. Wähle die platzierten Vias aus
2. Rechtsklick → **Properties**
3. Unter **Copper Zone Connection**: Wähle **Direct Connect** (NICHT Thermal Relief!)

> [!IMPORTANT]
> **Direct Connect** ist hier korrekt, weil wir maximale Wärmeableitung wollen.
> **Thermal Relief** (Speichenverbindung) wird normalerweise bei Durchsteck-Bauteilen verwendet, um das Löten zu erleichtern — bei Thermal Vias unter SMD-Pads ist das kontraproduktiv.

#### Schritt 4: Rückseite prüfen

1. Wechsle zur **Bottom Layer**
2. Stelle sicher, dass die Vias vom **GND-Copper-Pour** erfasst werden
3. Keine Inseln oder fehlende Verbindungen

#### Schritt 5: Solder-Mask Tenting (optional, empfohlen)

- **Tented Vias**: Lötstoppmaske bedeckt die Vias auf der Rückseite
- Verhindert, dass Lötzinn in die Vias fließt und Kurzschlüsse verursacht
- In EasyEDA Pro: Via-Properties → **Solder Mask Expansion** = 0 (oder negativ)

### 5.3 Via-Platzierung pro Bauteil

**AP63203 (U37):**

```
  Pin 1 ─┐     ┌─ Pin 6
         │     │
  Pin 2 ─┤  EP ├─ Pin 5     EP = Exposed Pad (GND)
         │ ○○○ │
  Pin 3 ─┘ ○○  └─ Pin 4
             ↑
        5 Vias im EP, 1mm Abstand
```

**PMV16XNR (Q4/Q5):**

```
  Gate ─┐
        │        
  Drain ┤   ← 2 Vias unter Drain-Pad
        │      (größtes Pad, führt den Strom)
  Source┘
```

---

## 6. I2C Routing — Analyse

### 6.1 Deine Werte

| Signal | Länge | Ziel |
|:---|:---|:---|
| SCL | 15.5mm | SCD41 CO2 Sensor |
| SDA | 14.6mm | SCD41 CO2 Sensor |

### 6.2 Bewertung

**Kein Buffer nötig.** ✅

I2C ist für bis zu **~30cm** bei Standard-Mode (100kHz) und **~10cm** bei Fast-Mode (400kHz) ohne Puffer zuverlässig. Deine Leitungslängen liegen bei nur **~15mm** — das ist weit unter den kritischen Grenzen:

| I2C-Modus | Max. Bus-Kapazität | Max. Leitungslänge (typisch) | Deine Länge |
|:---|:---|:---|:---|
| Standard (100kHz) | 400pF | ~30cm | **15mm** ✅ |
| Fast-Mode (400kHz) | 400pF | ~10cm | **15mm** ✅ |
| Fast-Mode Plus (1MHz) | 550pF | ~5cm | **15mm** ✅ |

**Warum kein Problem?**

- Bus-Kapazität bei 15mm: ~2–3pF (Leiterbahn) + ~5pF (SCD41 Pin) ≈ **~8pF total**
- Budget: 400pF → du nutzt nur **~2%** der erlaubten Kapazität
- Pull-ups (4.7kΩ) sind korrekt dimensioniert für 3.3V I2C

### 6.3 Empfehlungen (trotzdem sinnvoll)

1. **Pair-Routing**: SDA und SCL möglichst dicht nebeneinander routen (gleicher Layer, parallel)
2. **Keine Durchführung** durch Power-Planes wenn vermeidbar (erzeugt Impedanzsprünge)
3. **Gleiche Länge**: Dein Delta ist nur 0.9mm (15.5mm vs 14.6mm) — das ist perfekt, keine Anpassung nötig

> [!NOTE]
> Ein Bus-Buffer (z.B. PCA9600) wäre erst bei >20cm Gesamtlänge oder >3 I2C-Slaves an langen Stichen nötig. Mit 15mm und 2-3 Slaves bist du weit im sicheren Bereich.

---

## 7. Test Points

**Status**: ✅ Umgesetzt — 31 Test Points vorhanden.

**Abdeckung (aus CSV):**

| Kategorie | Test Points | Status |
|:---|:---|:---|
| **Power Rails** | 12V (TP9), 5V (TP4, TP7), 3V3 (TP1), GND (TP5, TP8, TP10) | ✅ Vollständig |
| **Buttons** | BTN_MOD (TP1, TP13), BTN_PWR (TP6, TP14), BTN_LVL (TP7) | ✅ Vollständig |
| **LEDs** | LED_WRG, LED_VEN, LED_L1–L5, LED_PWR, LED_MST | ✅ Vollständig |
| **Fan Circuit** | PWM_12V_OUT (TP16), DC_VAR_12V (TP18), FAN_TACHO (TP26) | ✅ Vollständig |
| **I2C** | SCL (TP20) | ⚠️ SDA fehlt |
| **NTC Sensoren** | NTC_IN_SIG (TP1), NTC_OUT_SIG (TP3) | ✅ Vollständig |
| **PCA9685** | PCA9685_OE (TP13) | ✅ |

> [!WARNING]
> **Fehlend**: Test Point für **SDA**. Du hast nur SCL (TP20). Empfehlung: TP für SDA hinzufügen, damit du beide I2C-Leitungen mit dem Oszilloskop/Logic-Analyzer messen kannst.

> [!NOTE]
> **Doppelte Designatoren**: Die CSV enthält mehrfach TP1, TP7, TP9, TP13. Das kann zu Konflikten führen. → Einheitlich durchnummerieren (TP1–TP31).

---

## 8. Zero-Ohm Widerstände als Sektions-Trenner

### 8.1 Was ist ein Zero-Ohm Widerstand?

Ein 0Ω-Widerstand (auch „Jumper-Widerstand") ist ein SMD-Bauteil mit (nahezu) keinem Widerstand. Er sieht aus wie ein normaler Widerstand, hat aber **0 Ohm** und verhält sich wie eine Drahtbrücke.

```
  ┌──────┐
──┤ 0Ω   ├──     ← SMD 0805, sieht aus wie normaler Widerstand
  └──────┘         Markierung: "000" oder einfach schwarz
```

### 8.2 Warum benutzt man ihn?

**Hauptzweck: Schaltbare Verbindung ohne Jumper-Kabel**

| Anwendung | Beschreibung | Beispiel in deinem Design |
|:---|:---|:---|
| **Sektions-Trennung** | Verbindung kann durch Entfernen des 0Ω getrennt werden | Trennung 12V → Lüfter-Sektion |
| **Messpunkt** | Strom messen: 0Ω entfernen, Amperemeter einsetzen | Stromverbrauch des Lüfters messen |
| **Debug-Isolation** | Einzelne Schaltungsteile isolieren ohne Traces zu schneiden | I2C Bus: PCA9685 abkoppeln |
| **Konfigurations-Option** | Verschiedene Bestückungsvarianten auf gleichem PCB | Pull-up auf 3.3V oder 5V |
| **Layer-Brücke** | Track-Routing vereinfachen auf Single-Layer Boards | Trace über GND-Plane führen |
| **Sicherung (einfach)** | Bei Überlast schmilzt der 0Ω-Widerstand durch (nicht zertifiziert) | Schutz vor Kurzschluss |

### 8.3 Konkretes Beispiel: Lüfter-Sektion isolieren

**Ohne 0Ω-Widerstand:**

```
12V ━━━━━━━━━━━━━━━━━━━━━━━━ Fan VCC
                                 │
                              [Lüfter]
                                 │
GND ━━━━━━━━━━━━━━━━━━━━━━━━ Fan GND

→ Problem: Bei Fehler in der Lüfter-Sektion musst du
  Traces schneiden oder entlöten, um zu isolieren
```

**Mit 0Ω-Widerstand (R0_FAN):**

```
12V ━━━━━━┤ R0_FAN (0Ω) ├━━━━━━ Fan VCC
                                    │
                                 [Lüfter]
                                    │
GND ━━━━━━━━━━━━━━━━━━━━━━━━━━━ Fan GND

→ Vorteil: 0Ω entfernen → Lüfter komplett abgetrennt
  → Rest des Boards funktioniert weiter
  → Amperemeter anstelle des 0Ω → Strom messen
```

### 8.4 Sinnvolle Positionen in deinem Design (optional)

| Position | Netz | Zweck |
|:---|:---|:---|
| **R0_FAN** | 12V → Lüfter VCC | Lüfter-Sektor isolieren / Strom messen |
| **R0_I2C** | SDA-Leitung vor PCA9685 | PCA9685 vom I2C-Bus trennen bei Debugging |
| **R0_3V3** | 3.3V Rail → Sensor-Sektion | Sensoren abkoppeln (Stromverbrauch messen) |

### 8.5 Technische Daten

| Parameter | Wert |
|:---|:---|
| **Package** | 0805 (empfohlen) oder 0603 |
| **Max. Strom** | 0805: ~2A, 0603: ~1A |
| **Widerstand** | 0Ω ±0Ω (typisch <50mΩ) |
| **BOM** | z.B. `RC0805JR-070RL` (Yageo) |
| **Kosten** | ~0.005€ / Stück |

> [!TIP]
> Zero-Ohm Widerstände sind in der **Prototyp-Phase** besonders wertvoll. Im Seriendesign werden sie oft durch direkte Traces ersetzt, wenn die Schaltung validiert ist. Für dein V1.0 PCB sind sie eine risikolose Absicherung — sie kosten praktisch nichts und brauchen nur ein 0805-Pad.

---

## Zusammenfassung der Maßnahmen

| # | Maßnahme | Priorität | Status |
|:---|:---|:---|:---|
| 1 | Bypass Caps ESP32 | ✅ Erledigt | C13 vorhanden (optional: 100nF ergänzen) |
| 2 | Galvanische Trennung | ✅ Erledigt | Slot + Warnung vorhanden |
| 3 | Bleeder-Widerstände | 🔴 Offen | 2× 47kΩ (0805) an C1 und C13 |
| 4 | Trace-Breiten | 🟡 Prüfen | ≥0.5mm für 1A, ≥0.8mm für 12V-Haupt |
| 5 | Thermal Vias | 🟡 Offen | AP63203: 5 Vias, Q4/Q5: 2–3 Vias |
| 6 | I2C Routing | ✅ OK | 15mm kein Buffer nötig |
| 7 | Test Points | ✅ Fast komplett | 31 TPs, SDA TP fehlt, Nummern-Duplikate bereinigen |
| 8 | Zero-Ohm Widerstände | 🟢 Optional | R0_FAN, R0_I2C, R0_3V3 empfohlen |
