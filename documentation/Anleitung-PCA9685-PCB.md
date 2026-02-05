# 📟 Anleitung: Control Panel PCB (PCA9685 & Buttons)

Diese Anleitung beschreibt das Design des PCBs für die Anbindung des VentoMaxx-Panels.
Wir verwenden einen **PCA9685** für **dimmbare LEDs** und direkte **GPIOs des ESP32-C6** für die **Taster**.

## 1. Konzept ändern

* **ALT**: MCP23017 (16 GPIOs für Taster & LEDs) -> Nur An/Aus für LEDs.
* **NEU**:
  * **PCA9685 (I2C PWM Driver)**: Steuert die LEDs und ermöglicht echtes **Dimmen** (Soft-Fade Effekte).
  * **Direkte GPIOs**: Die 3 Taster (Power, Mode, Level) werden direkt an freie Pins des XIAO Connectors angeschlossen.

## 2. Benötigte Komponenten (BOM)

| Bauteil | Wert | Bauform | Menge | Funktion |
| :--- | :--- | :--- | :--- | :--- |
| **PCA9685PW** | - | **TSSOP-28** | 1 | 16-Kanal PWM Driver (I2C). LCSC: `C2678753` |
| **Kondensator** | **10uF** | 0805 | 1 | Power Buffer (VDD) |
| **Kondensator** | **100nF** | 0603 | 1 | Bypass (direkt am Chip) |
| **Widerstand** | **10kΩ** | 0603 | 1 | Pullup für OE (Output Enable) |
| **Widerstand** | **4.7kΩ** | 0603 | 2 | Pullups für I2C (SDA/SCL) / schon erfolgt für BME680 |
| **Stecker** | **14-Pin FFC** | 1.0mm/0.5mm* | 1 | Zum Panel (*Pitch geprüft*) |
| **Widerstand** | **470Ω*** | 0603 | 9 | Vorwiderstände für LEDs (durch Messung ermittelt) |
| **Resistor Array** | **10kΩ** | 0603 (4-fach) | 1 | Pullups für Taster (LCSC: `C136853`) |
| **RC-Filter** | **100Ω/100nF** | 0603 | 3x | Optional: Entprellung für Taster |

## 3. Schaltplan Details

### A. PCA9685 (PWM für LEDs) - Pinout (TSSOP-28)

* **VDD (Pin 28)**: An **3.3V**.
* **VSS (Pin 14)**: An **GND**.
* **SDA (Pin 27)**: An **XIAO SDA (D4/GPIO22)**. (Korrigiert: Pin 27, nicht 26)
* **SCL (Pin 26)**: An **XIAO SCL (D5/GPIO23)**. (Korrigiert: Pin 26, nicht 25)
* **OE (Pin 23)**: An **XIAO D3 (GPIO21)**. **PullUp 10k** an 3.3V. (Active LOW).
* **EXTCLK (Pin 25)**: External Clock Input. Via **GND** deaktivieren (nutze internen Oszillator). (Korrigiert: Pin 25)
* **A0-A4 (Pin 1-5)**: Address Inputs. An **GND**.
* **A5 (Pin 24)**: Address Input. An **GND**. (Korrigiert: Pin 24)
  * Alle Adress-Pins an GND = Adresse **0x40**.

### B. LED Zuordnung (PCA9685 Outputs)

| Funktion | PCA9685 Kanal | TSSOP-28 Pin | FFC Pin |
| :--- | :--- | :--- | :--- |
| **LED Power** | LED0 | Pin 6 | Pin 6 |
| **LED Master**| LED1 | Pin 7 | Pin 5 |
| **LED L1** | LED2 | Pin 8 | Pin 11 |
| **LED L2** | LED3 | Pin 9 | Pin 10 |
| **LED L3** | LED4 | Pin 10| Pin 9 |
| **LED L4** | LED5 | Pin 11| Pin 8 |
| **LED L5** | LED6 | Pin 12| Pin 7 |
| **LED WRG** | LED7 | Pin 13| Pin 13 |
| **LED Vent** | LED8 | Pin 15| Pin 12 |

> ✅ **Bestätigung für Common Cathode (Gemeinsames GND)**:
>
> * **Verkabelung**: Kathoden der LEDs an **GND** (via Pin 13/14). Anoden an den PCA9685.
> * **Spannung**: Die LEDs benötigen 3.3V (High-Pegel vom PCA9685).
> * **Vorwiderstände**: **JA**, diese sind **zwingend erforderlich** (z.B. 470Ω - 1kΩ), da der PCA9685 eine Spannungsquelle ist (Totem-Pole) und den Strom nicht selbst begrenzt.
> * **Chip-Modus**: Der PCA9685 startet laut Datasheet (NXP) standardmäßig im **Totem-Pole Modus** (MODE2 Register = 0x04). Das passt perfekt für Common Cathode.

**Achtung bei 3.3V Logic:**
Schließe VDD des PCA9685 an 3.3V an. Der Chip treibt die LEDs direkt aus VDD. Daher ist die Entkopplung (siehe unten) extrem wichtig.

### C. Taster (Direkt an ESP32)

Die Taster-Pins vom FFC gehen direkt an den XIAO.
**WICHTIG:** Da du externe Pullups verbaut hast, sind die Pins "Active Low" (Gedrückt = GND).

| Funktion | FFC Pin | XIAO Pin | ESP32 GPIO | Pullup |
| :--- | :--- | :--- | :--- | :--- |
| **BTN Power**| Pin 2 | **D8** | GPIO19 | **10k an 3.3V** |
| **BTN Mode** | Pin 4 | **D9** | GPIO20 | **10k an 3.3V** |
| **BTN Level**| Pin 3 | **D10** | GPIO18 | **10k an 3.3V** |
| **GND** | PIN 1 | -| - | - |
| **GND** | PIN 14 | -| - | - |

**Gesamte Pin-Belegung (XIAO ESP32-C6):**

| Pin | Funktion |
| :--- | :--- |
| **D0 (GPIO0)** | NTC Sensor Innen (ADC) |
| **D1 (GPIO1)** | NTC Sensor Außen (ADC) |
| **D2 (GPIO2)** | *Frei* |
| **D3 (GPIO21)**| PCA9685_OE (PCA9685 Output Enable) |
| **D4 (GPIO22)**| I2C SDA (BME680, PCA9685) |
| **D5 (GPIO23)**| I2C SCL (BME680, PCA9685) |
| **D6 (GPIO16)**| Lüfter PWM |
| **D7 (GPIO17)**| Lüfter Tacho |
| **D8 (GPIO19)**| Taster Power |
| **D9 (GPIO20)**| Taster Mode |
| **D10 (GPIO18)**| Taster Level |

> ✅ **Lösung**: Durch die Nutzung der rückseitigen/kleinen Pads D8-D10 für die Taster bleiben D0 und D1 für die analogen Temperatursensoren frei. Das Design ist somit **konfliktfrei**.

## 4. PCB Layout Empfehlungen

1. **Entkopplung**: 100nF Kondensator direkt an VDD des PCA9685.
2. **I2C**: Saubere Leiterbahnführung für SDA/SCL.
3. **Taster**: Leitungen zu den Buttons ggf. mit 100nF gegen GND entprellen (Hardware Debounce), entlastet die Software.

---

## Software Konfiguration (ESPHome)

In der `yaml` wird der `mcp23017` Block entfernt und durch `pca9685` ersetzt.
Die `binary_sensor` (Taster) wechseln von `platform: gpio (mcp)` zu `platform: gpio (internal)`.

### Beispiel Ausschnitt

```yaml
i2c:
  sda: GPIO22
  scl: GPIO23

pca9685:
  - id: 'pwm_hub'
    frequency: 1000Hz # Gute Frequenz für LEDs (flimmerfrei)

binary_sensor:
  - platform: gpio
    pin:
      number: GPIO0
      mode: INPUT_PULLUP
      inverted: true
    name: "Button Power"
    # ...
```
