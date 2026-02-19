# Erweiterung: Anwesenheitserkennung (Radar + PIR)

Dieses Dokument beschreibt die geplante Erweiterung der Wohnraumlüftungssteuerung um zwei Bewegungssensoren:

1. **HLK-LD2450** (24GHz mmWave Radar) für präzise Anwesenheitserkennung.
    * *Einbau:* Hinter der Kunststoffblende, verbunden per Kabel (JST-Stecker auf PCB).
2. **AM312** (Mini-PIR) als **Optionale Ergänzung**.
    * *Hinweis:* Da der PIR eine freie Sicht benötigt ("Guckloch"), kann er nicht unsichtbar im bestehenden Gehäuse verbaut werden. Er wird nur bei Bedarf extern ergänzt.
3. **Zukunftsfähigkeit (PCB):** Vorsehen von zusätzlichen Headern für I2C und UART.

Da der ESP32-C6 keine freien Pins mehr hat, werden die unkritischen Taster auf einen **MCP23017 I/O-Expander** ausgelagert, um die notwendigen High-Speed-Pins (UART) für das Radar freizumachen.

## 1. Hardware-Anpassungen (PCB & Verkabelung)

### A. MCP23017 I/O Expander Integration

Der MCP23017 wird an den bestehenden I2C-Bus angeschlossen.

* **Anschluss:**
  * **SDA:** An `GPIO22` (parallel zu BME680/SCD41)
  * **SCL:** An `GPIO23` (parallel zu BME680/SCD41)
  * **VCC:** 3.3V
  * **GND:** GND
  * **Adresse:** `0x20` (Standard, alle Adress-Pins A0-A2 auf GND)

### B. Pin-Umzug (Rewiring)

Die Taster wandern auf den Expander, um native GPIOs für UART freizugeben.

| Komponente | Alter Pin (ESP32-C6) | Neuer Pin (MCP23017) | Grund |
| :--- | :--- | :--- | :--- |
| **Button Power** | GPIO16 | **GPA0** (Pin 21) | Taster unkritisch, UART benötigt GPIO16 (TX) |
| **Button Mode** | GPIO20 | **GPA1** (Pin 22) | Taster unkritisch, Pin für PIR frei machen |
| **Button Level** | GPIO2 | **GPA2** (Pin 23) | Taster unkritisch, UART benötigt GPIO2 (RX) |

*Hinweis: Die Taster schalten gegen GND. Interne Pullups des MCP23017 aktivieren.*

### C. Anschluss der neuen Sensoren

#### 1. HLK-LD2450 (Radar)

Benötigt UART (Serial). **Achtung: 5V Versorgung empfohlen! Logic Level ist 3.3V kompatibel.**

| HLK-LD2450 Pin | Anschluss an ESP32-C6 | Funktion |
| :--- | :--- | :--- |
| **VCC** | **5V** (VBUS/5V Pin) | **WICHTIG:** 5V für stabile Funktion |
| **GND** | GND | Masse |
| **TX** | **GPIO2** (RX)* | Sendet Daten an ESP (ehemals Button Level) |
| **RX** | **GPIO16** (TX)* | Empfängt Konfig vom ESP (ehemals Button Power) |

*\*Kreuzung beachten: TX -> RX und RX -> TX*

#### 2. AM312 (PIR) - *OPTIONAL / EXTERN*

Digitaler Output. Kann nicht hinter geschlossener Blende montiert werden.

| AM312 Pin | Anschluss an ESP32-C6 | Funktion |
| :--- | :--- | :--- |
| **VCC** | 3.3V (oder 5V) | Spannungsversorgung |
| **GND** | GND | Masse |
| **OUT** | **GPIO20** | Digital Signal (High = Bewegung) |

### D. Zusätzliche Erweiterungs-Header (Future-Proofing)

Um für künftige Projekte oder externe Sensoren gerüstet zu sein, sollen auf dem PCB folgende Anschlüsse als Stiftleisten (JST oder Pin Header) herausgeführt werden:

1. **Zusätzlicher I2C Port (External I2C):**
    * Pins: SDA (GPIO22), SCL (GPIO23), 3.3V, GND
    * Zweck: Anschluss weiterer Sensoren (z.B. Display, weitere Umweltsensoren).
2. **Zusätzlicher UART Port (External UART):**
    * Pins: RX/TX (frei wählbar oder shared falls möglich, besser dediziert wenn Pins frei werden durch MCP-Nutzung), 3.3V, GND.
    * *Hinweis:* Da wir GPIO16/GPIO2 für das Radar nutzen, sind die Hardware-UARTs belegt. Ein weiterer Hardware-UART Port ist beim ESP32-C6 knapp.
    * **Lösung:** Wir führen die Pins **GPIO20** (PIR Option) und einen weiteren Pin (falls durch Optimierung frei) auf einen Header, um ggf. Software-Serial oder einen alternativen Bus nutzen zu können.
    * *Alternativ:* Ein zweiter MCP23017 oder freie Pins des ersten MCP23017 auf einen Header legen für langsame Signale.

### E. Überlegung: BMP388 / BMP390 Drucksensor (Onboard)

Als weitere Option soll direkt auf dem PCB ein Footprint für einen hochpräzisen Drucksensor (z.B. BMP390) vorgesehen werden.

* **Anschluss:** I2C (parallel zu BME680/SCD41).
* **Adresse:** `0x76` (SDO auf GND) oder `0x77` (SDO auf VCC - Konflikt mit BME680 prüfen!). **Wichtig:** BME680 hat meist 0x77. BMP390 muss auf **0x76** konfiguriert werden.
* **Anschluss:** I2C (parallel zu BME680/SCD41).
* **Adresse:** `0x76` (SDO auf GND) oder `0x77` (SDO auf VCC - Konflikt mit BME680 prüfen!). **Wichtig:** BME680 hat meist 0x77. BMP390 muss auf **0x76** konfiguriert werden.

**Mögliche Anwendungsfälle:**

1. **Filterüberwachung (indirekt):** Durch Messung des absoluten Luftdrucks im Ansaug- oder Abluftstrom *könnten* Rückschlüsse auf den Druckabfall (und damit Verschmutzung) gezogen werden (benötigt Referenz).
2. **Druckausgleich:** Erkennung von Unter-/Überdruck im Raum im Vergleich zu draußen (benötigt zweiten Sensor außen).
3. **Wetterstation:** Die BMP3xx-Serie ist extrem präzise (±8 Pa relative Genauigkeit) und eignet sich hervorragend als Referenz-Barometer.
4. **Lüftungssteuerung:** Erkennung, ob Fenster/Türen geöffnet wurden (plötzliche Druckänderung) -> Lüftung temporär stoppen.

### F. Erweiterte Luftqualität: SCD41 + SEN54/55 (PM & NOx)

Während der **SCD41** bereits für die CO2-Regelung vorgesehen ist, bietet die Ergänzung um einen Partikelsensor einen umfassenden Schutz vor Außenluftbelastung.
**Problem**: Das ist nur über ein separates Gerät möglich, da der Sensor außen angebracht werden muss und die Abmaße des SEN54/55 nicht in die Lüftungsbox passen. Es gibt aber eine Alternative: Der **Sensirion SPS30** ist kleiner und könnte passen.

* **SCD41 (CO2):** Misst verbrauchte Innenluft.
  * *Regelung:* "Wir brauchen mehr Sauerstoff".
  * *Status:* Bereits eingeplant/vorhanden.
* **Sensirion SEN54 oder SEN55 (PM1.0/2.5/4.0/10 + NOx):** Misst belastete Außenluft (Staub, Rauch, Abgase).
  * *Nutzen:* Verhindert, dass bei schlechter Außenluft (z.B. Kaminrauch des Nachbarn, Berufsverkehr) Schadstoffe ins Haus gesaugt werden.
  * *Regelung:* "Draußen ist Dreck, Lüftung reduzieren oder stoppen".
  * *Anschluss:* I2C (parallel zum Bus). Benötigt **5V** Spannungsversorgung!
  * *Hinweis:* Der SEN55 misst zusätzlich Stickoxide (NOx), was an stark befahrenen Straßen relevant ist.

### G. Differenzdruck-Sensor: SDP810 / SDP3x

Für die **echte Filterüberwachung** und Volumenstromregelung.

* **Empfehlung:** Sensirion **SDP810** (Robust) oder **SDP3x** (Winzige SMD-Variante).
* **Warum?**
  * Misst den Druckabfall über dem Filter (Druck VOR Filter vs. Druck NACH Filter).
  * Das ist der "Gold-Standard" gegenüber der reinen Zeitmessung oder dem absoluten Druck (BMP390).
* **Use-Case:**
    1. **Smarter Filterwechsel:** Meldung genau dann, wenn der Filter tatsächlich voll ist.
    2. **Konstant-Volumenstrom:** Der Lüfter regelt automatisch nach, wenn der Filter sich zusetzt, um die Luftmenge konstant zu halten.
* **Anschluss:** I2C.

### H. Helligkeitssensor: BH1750 / VEM7700

Für den Schlafzimmer-Betrieb (LED-Dimmung).

* **Empfehlung:** **BH1750** (Günstig, gut) oder **VEM7700** (Präziser, Low-Light).
* **Warum?**
  * Die Status-LEDs der Lüftung können nachts stören.
* **Use-Case:**
    1. **Auto-Dimming:** LEDs werden gedimmt oder ganz abgeschaltet, wenn der Raum dunkel ist.
    2. **Sleep-Mode:** Automatische Erkennung "Nachtruhe" -> Lüfter fährt auf flüsterleise Stufe runter.
* **Anschluss:** I2C.
  * *Montage:* Benötigt Lichtöffnung im Gehäuse oder Lichtleiter.

---

## 2. Software-Anpassungen (ESPHome YAML)

### A. MCP23017 Konfiguration (Update)

Falls noch nicht vorhanden, `pca9685` ist für PWM, `mcp23017` ist neu für Inputs.

```yaml
i2c:
  sda: GPIO22
  scl: GPIO23

mcp23017:
  - id: mcp_buttons
    address: 0x20
```

### B. Taster auf MCP23017 umziehen

Die `binary_sensor` Einträge für die Buttons müssen angepasst werden.

```yaml
binary_sensor:
  - platform: gpio
    name: "Button Power"
    id: button_power
    pin:
      mcp23xxx: mcp_buttons
      number: 0 # GPA0
      mode:
        input: true
        pullup: true # MCP hat interne Pullups
      inverted: true # Schaltet gegen GND
    # ... on_click logic bleibt gleich ...

  - platform: gpio
    name: "Button Mode"
    id: button_mode
    pin:
      mcp23xxx: mcp_buttons
      number: 1 # GPA1
      mode:
        input: true
        pullup: true
      inverted: true
    # ... on_click logic bleibt gleich ...

  - platform: gpio
    name: "Button Level"
    id: button_level
    pin:
      mcp23xxx: mcp_buttons
      number: 2 # GPA2
      mode:
        input: true
        pullup: true
      inverted: true
    # ... on_click logic bleibt gleich ...
```

### C. HLK-LD2450 Konfiguration

UART und Komponente hinzufügen.

```yaml
uart:
  id: uart_radar
  tx_pin: GPIO16 # An Sensor RX
  rx_pin: GPIO2  # An Sensor TX
  baud_rate: 256000
  parity: NONE
  stop_bits: 1

ld2450:
  uart_id: uart_radar
  id: radar_sensor

binary_sensor:
  - platform: ld2450
    ld2450_id: radar_sensor
    has_target:
      name: "Radar Anwesenheit"
    # Optional: Zone detection triggers
```

### D. AM312 Konfiguration

```yaml
binary_sensor:
  - platform: gpio
    pin: GPIO20
    name: "PIR Bewegung"
    device_class: motion
    filters:
      - delayed_off: 5s # Kurzes Entprellen
```

## 3. Platzierungsempfehlung

* **Radar (HLK-LD2450):**
  * Montage **hinter** der Kunststofffront der Lüftungsanlage (unsichtbar).
  * Verbunden über ein Kabel mit dem PCB (Steckverbinder vorsehen).
  * Ausrichtung: Leicht geneigt (5-10°) nach unten.
* **PIR (AM312) - Optional:**
  * Benötigt "Sichtkontakt" zum Raum (Linse muss herausschauen).
  * Wird nur bei explizitem Bedarf nachgerüstet (z.B. seitlich am Gehäuse).
