# 📟 Anleitung: MCP23017 Integration in EasyEDA Pro

Diese Anleitung beschreibt präzise, wie der **MCP23017 I/O Expander** (16-Bit, I2C) professionell in dein PCB-Design integriert wird.
Diese Anleitung bezieht sich spezifisch auf den **MCP23017T-E/SS** von Microchip.

## 1. Benötigte Komponenten (BOM)

| Bauteil | Wert | Bauform | Menge | Hinweise |
| :--- | :--- | :--- | :--- | :--- |
| **MCP23017T-E/SS** | - | **SSOP-28** | 1 | **Pitch 0.65mm**. LCSC Part: `C558584`. |
| **Kondensator** | **100nF** (0.1µF) | 0603 | 1 | **Bypass**. Extrem wichtig für Stabilität. |
| **Widerstand** | **10kΩ** | 0603 | 1 | **Pullup für RESET**. |
| **Widerstand** | **4.7kΩ** | 0603 | 2 | **I2C Pullups** (SDA/SCL). |
| **Stecker** | **14-Pin FFC** | 1.0mm Pitch | 1 | Passend zum VentoMaxx Kabel. |

> ⚠️ **Löthinweis:** Der **SSOP-28** Footprint hat einen feinen Pin-Abstand (0.65mm). Das Löten von Hand ist möglich (viel Flussmittel!), erfordert aber Erfahrung. Für Reflow/Hotplate ist es unproblematisch.

---

## 2. Schaltplan (Schematic) in EasyEDA Pro

Die Pin-Nummerierung für das SSOP-28 Gehäuse ist identisch zum SOIC-28.

### A. Stromversorgung & Reset

* **VDD (Pin 9)**: An **+3.3V**.
* **VSS (Pin 10)**: An **GND**.
* **RESET (Pin 18)**: Via **10kΩ** Pullup an +3.3V.
* **INTB (Pin 19)**: An **XIAO D3** (GPIO21). Dies meldet Tastendrücke (da Taster an Port B sind).
* **INTA (Pin 20)**: *Unbenutzt* (kann offen bleiben oder testweise auch auf D3 gelegt werden, wenn Open-Drain konfiguriert). Wir nutzen hier nur INTB.

### B. Adressierung (Adresse 0x20)

* **A0 (Pin 15)**: An **GND**.
* **A1 (Pin 16)**: An **GND**.
* **A2 (Pin 17)**: An **GND**.

### C. I2C Bus (3.3V Logic)

* **SCK (Pin 12)**: An **XIAO D5**.
* **SDA (Pin 13)**: An **XIAO D4**.
* *Wichtig*: **4.7kΩ** Pullups (SDA/SCL) sind ideal.
  * ⚠️ **Achtung**: Dein Wert von **470Ω** ist sehr niedrig (zu "stark")! Das verursacht ca. 7mA Strom. Viele I2C-Chips (Spec: max 3mA Sink-Current) schaffen es dann nicht, die Leitung auf "Low" (0V) zu ziehen -> Kommunikation schlägt fehl. Bitte tausche sie gegen **2.2kΩ bis 10kΩ**.

### D. Ausgänge zum Panel (FFC 14-Pin)

Verbinde die GPIOs des MCP mit dem FFC-Connector Symbol im Schaltplan.

| MCP Pin | Name | Zu FFC Pin (Beispiel\*) | Funktion |
| :--- | :--- | :--- | :--- |
| **GPA0 (21)** | LED_PWR | Pin 1 | LED Power |
| **GPA1 (22)** | LED_MST | Pin 2 | LED Master |
| **GPA2 (23)** | LED_L1 | Pin 3 | Level 1 |
| **GPA3 (24)** | LED_L2 | Pin 4 | Level 2 |
| **GPA4 (25)** | LED_L3 | Pin 5 | Level 3 |
| **GPA5 (26)** | LED_L4 | Pin 6 | Level 4 |
| **GPA6 (27)** | LED_L5 | Pin 7 | Level 5 |
| **GPA7 (28)** | LED_WRG | Pin 8 | Mode WRG |
| **GPB0 (1)** | LED_VEN | Pin 9 | Mode Vent |
| **GPB1..4** | - | - | *Frei lassen* |
| **GPB5 (6)** | BTN_PWR | Pin 10 | Taster Power |
| **GPB6 (7)** | BTN_MOD | Pin 11 | Taster Mode |
| **GPB7 (8)** | BTN_LVL | Pin 12 | Taster Level |

> ⚠️ **\*WICHTIG**:
>
> 1. **Pitch messen!** Du nanntest "0.5-14P" (0.5mm Pitch). Messen Sie am Panel-Kabel nach! Viele Ventomaxx Panels haben 1.0mm Pitch. Passt der Stecker nicht, ist die Platine nutzlos.
> 2. **VCC/GND**: Vergiss nicht **VCC** und **GND** am Stecker bereitzustellen.

### E. Schutzbeschaltung am Stecker (ESD & Puffer)

Da das Panel berührbar ist (ESD-Gefahr) und LEDs Strom ziehen, sind weitere Bauteile am FFC-Stecker **dringend empfohlen**:

1. **Puffer-Kondensator**: Setze einen **1µF bis 10µF Kerko (0805)** zwischen VCC und GND direkt am FFC-Stecker.
    * *Grund*: Verhindert flackernde LEDs bei schnellen Wechseln.
2. **ESD-Schutz (TVS-Dioden)**: Setze Schutzdioden (z.B. ESD-Array **SRV05-4** oder diskrete TVS-Dioden) auf alle Signalleitungen (LEDs/Taster) zwischen Chip und Stecker.
    * *Grund*: Statische Aufladung vom Finger kann sonst den MCP23017 zerstören.
    * *Alternativ*: Wenn Platz knapp ist, nutze zumindest 100Ω Serienwiderstände in den Leitungen.

---

### F. Widerstände für LEDs & Taster (Pullups & Vorwiderstände)

1. **Taster (Buttons)**:
    * **Pullups**: Du brauchst **keine externen Widerstände**.
    * *Grund*: Der MCP23017 hat **integrierte 100kΩ Pullups**, die wir per Software einschalten. Das reicht für kurze Kabel völlig aus.

2. **LEDs**:
    * **Vorwiderstände (Series Resistors)**: ⚠️ **Prüfung nötig!**
    * Du musst prüfen, ob das VentoMaxx-Panel bereits SMD-Widerstände für die LEDs eingebaut hat (miss den Widerstand zwischen Pin im Stecker und LED).
    * **Fall A (Panel hat Widerstände)**: Du brauchst nichts tun. Direkt anschließen.
    * **Fall B (Panel hat KEINE Widerstände)**: Du brauchst zwingend **Vorwiderstände** (z.B. **470Ω - 1kΩ**) in jeder LED-Leitung zwischen MCP und Stecker, sonst brennen die LEDs oder der Chip durch!

---

## 3. PCB Layout (Board Design)

Beachte beim Platzieren und Routen folgende Regeln in EasyEDA:

1. **Placement (Platzierung)**:
    * Platziere den **100nF Kondensator** so nah wie physikalisch möglich an Pin 9 (VDD) und Pin 10 (VSS). Das ist der wichtigste Schritt für Stabilität!
    * Platziere den MCP23017 nahe am FFC-Stecker, um die 12 Leiterbahnen kurz zu halten.

2. **Routing (Leiterbahnen)**:
    * **I2C (SDA/SCL)**: Führe diese Leitungen parallel und halte sie fern von PWM-Leitungen des Lüfters (Störsignale!).
    * **Breite**: Standard 0.254mm (10mil) reicht für Logiksignale. VDD/GND gerne etwas breiter (0.4-0.5mm).

3. **Ground Plane**:
    * Fülle die nicht genutzten Flächen auf Top- und Bottom-Layer mit einer **GND-Pour (Polygon)**. Verbinde Pin 10 (VSS) und die Adress-Pins A0-A2 direkt via Vias mit dieser Fläche.

---

## 4. Checkliste für EasyEDA Pro

1. [ ] Bauteil **MCP23017T-E/SS** suchen (LCSC: `C558584`).
2. [ ] Footprint **SSOP-28_208mil** (oder ähnlich) prüfen.
3. [ ] **100nF** Kondensator (0603) direkt an Pin 9/10 platzieren.
4. [ ] Pads beim Löten nicht brücken (Viel Flussmittel verwenden!).
