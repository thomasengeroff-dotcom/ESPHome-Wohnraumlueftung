# 🛠️ Hardware Setup: Prototyp & PCB

Dieses Dokument spezifiziert die Hardware-Komponenten, die Verkabelung für den Prototypen (Breadboard) und die Design-Vorgaben für das finale PCB (EasyEDA).

> [!IMPORTANT]
> **Pin-Referenz**: Die Pin-Belegung basiert auf der Nutzung des **Original VentoMaxx Bedienpanels** (14-Pin FFC), welches über einen **MCP23017 I/O Expander** via I²C angesteuert wird.

---

## 1. Übersicht & BOM (Bill of Materials)

### Zentrale Einheit & Power

* **MCU**: Seeed Studio XIAO ESP32C6
* **Netzteil (High Voltage)**: Recom RAC10-12SK/277 (230V AC -> 12V DC, 10W)
* **Logic Power (DC/DC)**: Recom R-78E5.0-0.5 oder Traco TSR 1-2450 (12V -> 5V)
* **Sicherung**: 1A Slow Blow (Träge)
* **Varistor**: S10K275 (Überspannungsschutz Eingang)
* **Pufferung**:
  * 470µF / 25V Elko (12V Rail)
  * 10µF / 50V Elko (Eingang DC/DC)
  * 10µF / 16V Elko (Ausgang DC/DC 5V)
  * 100nF Kerko (HF-Entstörung 5V Rail)

### Aktoren & Sensoren

* **Lüfter**: 120mm PWM Lüfter (12V) -> *Geplant: ebm-papst AxiRev*
  * *Interface*: BC547 NPN-Transistor + 1kΩ Basiswiderstand
* **BME680**: Bosch Umweltsensor (I2C) - Temperatur, Feuchte, Druck, IAQ
* **VentoMaxx Bedienpanel**: Original Panel mit 9 LEDs und 3 Tastern (14-Pin FFC Anschluss).
* **I/O Expander**: **MCP23017** (I2C) - Zur Ansteuerung des Panels.
* **NTC-Sensorik**: 2x 10kΩ NTC (Beta 3435)
  * *Beschaltung*: 10kΩ 0.1% Referenzwiderstand + 100nF Filter + 3.3V Zener/TVS

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
