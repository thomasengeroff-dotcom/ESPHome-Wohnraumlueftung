# 🛠️ Hardware Setup: Prototyp & PCB

Dieses Dokument spezifiziert die Hardware-Komponenten, die Verkabelung für den Prototypen (Breadboard) und die Design-Vorgaben für das finale PCB (EasyEDA).

> [!IMPORTANT]
> **Pin-Referenz**: Die Pin-Belegung basiert auf der Nutzung des **Original VentoMaxx Bedienpanels** (14-Pin FFC), welches über einen **MCP23017 I/O Expander** via I²C angesteuert wird.

---

### Zentrale Einheit & Power

* **MCU**: Seeed Studio XIAO ESP32C6
* **Netzteil (High Voltage)**: Recom RAC10-12SK/277 (230V AC -> 12V DC, 10W)
* **Logic Power (MCU Supply)**: Recom R-78E5.0-0.5 oder Traco TSR 1-2450 (12V -> 5V)
  * *Versorgung*: Speist ausschließlich den XIAO ESP32C6 (via 5V Pin).
* **Peripheral Power (3.3V Buck)**: **Diodes Inc. AP63203WU-7** (12V -> 3.3V, 2A)
  * *Versorgung*: Speist alle Sensoren (BME680), I/O Expander und Bedienpanel.
  * *Hinweis*: Der interne 3.3V LDO des ESP32 wird **nicht** genutzt (Pin open).
* **Sicherung**: 1A Slow Blow (Träge)
* **Varistor**: S10K275 (Überspannungsschutz Eingang)
* **Pufferung**:
  * 470µF / 25V Elko (12V Rail)
  * 10µF / 50V Elko (Eingang 5V DC/DC)
  * **470µF / 16V Elko** (Ausgang 5V DC/DC) - *Wichtig für ESP32 WiFi Peaks!*
  * 100nF Kerko (HF-Entstörung 5V Rail)
  * **AP63203 Beschaltung** (siehe unten bei PCB Design)

### Aktoren & Sensoren

* **Lüfter (Option 1 - Aktuell)**: **ebm-papst VarioPro® 4412 FGM PR** (3-Pin)
  * *4412*: Baugröße 119x119x25mm, 12V DC.
  * *F*: Flache Bauweise (25mm).
  * *G*: Gleitlager (Sintec).
  * *M*: Mittlere Drehzahl/Leistung.
  * *PR*: Programmierbar (VarioPro), hier genutzt als 3-Pin Anschluss (Spannungsregelung).
  * *Reversierbar*: Ja (via Programmierung)
  * *Interface*: **Universal-Interface im 3-Pin Mode** (High-Side PWM + Filter).

* **Lüfter (Option 2 - Upgrade)**: **ebm-papst AxiRev** (4-Pin PWM)
  * *Besonderheit*: **Volle Reversierbarkeit** bei fast identischer Effizienz in beide Richtungen.
  * *Größe*: 126mm (Benötigt ggf. mechanische Anpassung).
  * *Interface*: **Universal-Interface im 4-Pin Mode** (Direktes PWM, Tacho, 12V Dauerplus).
  * *Vorteil*: Extrem leise Umschaltung und optimierte Strömungskennlinie für Pendellüfter.
* **BME680**: Bosch Umweltsensor (I2C) - Temperatur, Feuchte, Druck, IAQ
* **VentoMaxx Bedienpanel**: Original Panel mit 9 LEDs und 3 Tastern (14-Pin FFC Anschluss).
* **I/O Expander**: **MCP23017** (I2C) - Zur Ansteuerung des Panels.
* **NTC-Sensorik**: 2x 10kΩ NTC (Beta 3435)
  * *Beschaltung*: 10kΩ 0.1% Referenzwiderstand + 100nF Filter + 3.3V Zener/TVS

## 1. Detaillierte BOM (Bill of Materials)

Status: **Final (06.02.2026)** - ✅ Verified

| Nr. | Menge | Wert / Bauteil | Designator | Footprint | Hersteller Part | Notiz |
| :-- | :-- | :-- | :-- | :-- | :-- | :-- |
| 1 | 1 | 470µF / 25V | C1 | Radial BD10 | KM477M025G13RR | Main 12V Buffer |
| 2 | 1 | 10µF / 50V | C2 | 1206 | GRM31CR61H106MA12L | 5V Reg Input |
| 3 | 5 | 100nF | C6, C7, C10, C22, **C25** | 0805 | CC0805KRX7R... | Bypass / HF (C25 für AP63203) |
| 4 | 1 | 470µF / 16V | C13 | SMD BD6.3 | RVT1C471M0607 | 5V Output Buffer |
| 5 | 1 | 10µF / 25V | C20 | **0603** | CL10A106MA8NRNC | AP63203 Input (Bias beachten) |
| 6 | 2 | 22µF / 6.3V | C23, C24 | 0603 | CL10A226MQ8NRNC | AP63203 Output |
| 7 | 1 | **100µF / 25V** | **C27** | SMD BD6.3 | RVT1E101M0605 | Fan Filter (Ersetzt C11) ✅ |
| 8 | 1 | 10µF / 25V | C28 | 1210 | CL32B106KAJNNNE | Extra Buffer |
| 9 | 1 | 470µH | L1 | SMD | ASPI-0804T-471M-T | Fan Filter |
| 10 | 1 | 3.9µH | L2 | SMD | ANR4030T3R9M | AP63203 Inductor |
| 11 | 1 | AP63203WU-7 | U37 | TSOT-26 | AP63203WU-7 | 3.3V Regulator |
| ... | ... | ... | ... | ... | ... | ... |

> [!TIP]
> **Check-Ergebnis**:
>
> 1. **Sicherheit**: Das kritische C11 Problem ist gelöst. **C27 (100µF / 25V)** ist absolut sicher für die 12V Lüfter-Spannung.
> 2. **Stabilität**: **C25 (100nF)** wurde parallel zu C20 ergänzt. Das ist sehr gut für das HF-Verhalten des Buck-Converters.
> 3. **Hinweis zu C20**: Du nutzt `CL10A106MA8NRNC` (0603, 10µF, 25V).
>     * Das ist elektrisch sicher (Spannungsfestigkeit passt).
>     * *Physikalischer Effekt*: Bei 12V Bias hat ein 0603 MLCC nur noch ca. 20-30% seiner Kapazität (effektiv ~2-3µF). Da du aber C25 (HF) und C1 (470µF Bulk) hast, ist das in diesem Design **akzeptabel**.
>
> **Freigabe: BOM ist valide.**

---

## 2. Pin-Mapping

### A. XIAO ESP32C6 (Master)

| XIAO Pin | GPIO | Funktion | Anschluss-Details |
| :--- | :--- | :--- | :--- |
| **D0** | GPIO0 | **NTC Innen** | Analog In (10k Spannungsteiler) |
| **D1** | GPIO1 | **NTC Außen** | Analog In (10k Spannungsteiler) |
| **D2** | GPIO2 | *Reserve* | (Frei / Debug LED) |
| **D3** | GPIO21 | **MCP23017 INTB** | Interrupt von Port B (Taster). Pin 19. |
| **D4** | GPIO22 | **I2C SDA** | BME680, MCP23017 (mit 4.7kΩ Pullup) |
| **D5** | GPIO23 | **I2C SCL** | BME680, MCP23017 (mit 4.7kΩ Pullup) |
| **D6** | GPIO16 | **Fan PWM** | An Basis BC547 |
| **D7** | GPIO17 | **Fan Tacho** | Von Lüfter Pin 3 (mit Pullup & Zener) |
| **D8-D10** | - | *Reserve* | SPI / Frei |

### C. Universal-Lüfter Interface (3-Pin & 4-Pin Support)

Damit du sowohl moderne 4-Pin PWM-Lüfter als auch ältere/spezielle 3-Pin Lüfter (wie VarioPro) steuern kannst, brauchst du eine **Umschaltung**.

**Logik:**

* **4-Pin Mode**: 12V ist dauerhaft an. PWM-Signal geht an Pin 4.
* **3-Pin Mode**: 12V wird "gehackt" (High-Side PWM). Pin 4 ist inaktiv.

**Schaltplan-Erweiterung (High-Side Switch):**

1. **P-Channel MOSFET** (z.B. AO3401) in die 12V-Leitung zum Lüfter einfügen (Source an 12V, Drain an Lüfter Pin 2).
2. **Treiber**: NPN-Transistor (BC547) steuert das Gate des MOSFETs (Gate Pullup 10k auf 12V, NPN Collector an Gate).
3. **DIP-Switch / Jumper** (2-Wege Umschalter) für das PWM-Signal (XIAO D6):
    * **Position A (4-Pin)**: Leitet D6 direkt an Lüfter **Pin 4**. (In dieser Stellung muss der MOSFET dauerhaft leitend geschaltet werden, z.B. Gate manuell auf GND brücken oder zweiten Jumper nutzen).
    * **Position B (3-Pin)**: Leitet D6 an die Basis des NPN-Treibers -> MOSFET moduliert die 12V Versorgungsspannung.

> ⚠️ **Tacho-Signal & Geräusche (3-Pin Mode)**:
>
> 1. **RPM-Schwankung**: Ja, das betrifft nur das *Auslesen* der Drehzahl (Tacho), da dem Sensor der Strom fehlt. Das ist für die Regelung unkritisch, nur die Anzeige "zappelt".
> 2. **Spulenfiepen (Whine)**: "Gehackte" 12V (PWM) erzeugen bei Lüftern oft hörbares Fiepen.
>     * **Lösung für "maximale Sauberkeit"**: Baue einen **LC-Filter** (Spule + Kondensator) hinter den MOSFET. Das glättet das PWM-Signal zu einer sauberen Gleichspannung (Buck-Converter Prinzip).
>     * **Regelung**: Die Drehzahl ist weiterhin **voll regelbar**! (50% PWM ≈ 6V, 100% PWM = 12V).
>     * *Werte*: z.B. 100µH - 330µH Spule + 100µF Elko. Damit läuft der 3-Pin Lüfter lautlos und analog.

### B. MCP23017 Expander (Adresse 0x20)

Der Expander steuert das 14-Pin FFC Kabel des Panels.

| MCP Pin | Funktion | Panel Komponente |
| :--- | :--- | :--- |
| **GPA0** | OUTPUT | LED Power (On/Off) |
| **GPA1** | OUTPUT | LED Master (Status) |
| **GPA2** | OUTPUT | LED Intensität 1 |
| **GPA3** | OUTPUT | LED Intensität 2 |
| **GPA4** | OUTPUT | LED Intensität 3 |
| **GPA5** | OUTPUT | LED Intensität 4 |
| **GPA6** | OUTPUT | LED Intensität 5 |
| **GPA7** | OUTPUT | LED Modus: Wärmerückgewinnung |
| **GPB0** | OUTPUT | LED Modus: Durchlüften |
| **GPB5** | INPUT_PULLUP | Taster: Power |
| **GPB6** | INPUT_PULLUP | Taster: Modus |
| **GPB7** | INPUT_PULLUP | Taster: Intensität |

---

## 3. Verkabelung & Bedienpanel (FFC 14-Pin)

Das VentoMaxx Panel wird über ein 14-Pin Flachbandkabel angeschlossen. Die Belegung muss auf dem PCB auf den I/O Expander geroutet werden.

**Logik-Annahme**:

* **LEDs**: Common Cathode (oder Anode) -> *Muss geprüft werden!* (Planung geht von Standard-Logik aus).
* **Taster**: Schalten gegen GND (Common Line).

---

## 4. PCB Design Hinweise

* **MCP23017**: Reset-Pin auf VCC (High) legen. Adress-Pins (A0, A1, A2) auf GND (Addr 0x20).
* **FFC Connector**: Passenden 14-Pin FPC/FFC Connector (Pitch messen! Meist 1.0mm oder 0.5mm) einplanen.
* **ESD Schutz**: Alle Leitungen zum FFC Connector sollten mit TVS-Dioden geschützt werden, da das Panel extern bedienbar ist.

### AP63203 (12V -> 3.3V) Implementierung (Peripherie-Versorgung)

Der **AP63203WU-7** versorgt alle externen 3.3V Komponenten (Sensoren, MCP23017), während der ESP32 separat über 5V läuft.

**Power-Architektur:**

* **ESP32**: Wird via **5V Pin** vom Recom/Traco Wandler (5V) versorgt.
* **Peripherie**: Wird via **AP63203 (3.3V)** versorgt.
* **WICHTIG**:
  * Der 3.3V Pin des ESP32 bleibt **unbeschaltet** (NC).
  * **Ground (GND)** von 5V-Kreis und 3.3V-Kreis müssen verbunden sein!
  * I2C Pullups müssen an die **3.3V (AP63203)** Rail angeschlossen werden (um die Sensoren zu schützen, da ESP32 3.3V tolerant ist).

**Komponenten-Werte (Schematic):**

* **U1**: Diodes Inc. AP63203WU-7 (3.3V Fixed, 2A Synch Buck)
* **C_IN (Eingang)**:
  * 1x **10µF** / 25V (oder 35V) MLCC (X7R, 1206) - Nahe an VIN/GND Pins!
  * 1x **100nF** (0.1µF) / 50V MLCC (0603) - HF-Entkopplung.
* **L1 (Spule)**:
  * **3.9µH** (Empfohlen laut Datasheet Table 2 für 3.3V Output).
  * *Alternativ*: 4.7µH möglich (SparkFun nutzt dies), aber 3.9µH ist das Optimum für maximale Effizienz/Stabilität.
  * *Rating*: Sättigungsstrom (Isat) > 2.5A. DCR < 50mΩ.
* **C_OUT (Ausgang)**:
  * 2x **22µF** / 10V (oder 6.3V) MLCC (X7R, 0805/1206).
  * *Wichtig*: Keramik-Kondensatoren nutzen (Low ESR).
* **C_BOOT**:
  * 1x **100nF** (0.1µF) / 16V MLCC (0603) zwischen BST und SW Pin.

**Layout-Tipps:**

1. **Input-Cap**: Der 10µF Eingangskondensator muss so nah wie möglich an den PINs VIN und GND des ICs sitzen (kleinster Loop Area).
2. **SW Node**: Die Verbindung vom SW-Pin zur Spule kurz und breit halten (hohe HF-Emissionen).
3. **Power Path**: Breite Traces für VIN, GND und VOUT (mind. 20-30 mil, besser Flächen).
4. **Thermal**: GND-Pin gut an die Ground-Plane anbinden (Vias) zur Wärmeabfuhr.
