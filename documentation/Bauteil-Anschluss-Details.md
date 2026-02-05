Die korrekte Verschaltung dieser Bauteile ist kritisch für die Stabilität des PCA9685.

### 1. Kondensator 10µF (0805) - "Der Tank"

Dieser dient als **Energiespeicher** für grobe Lastwechsel (wenn viele LEDs gleichzeitig angehen).

* **Frage**: Ich habe hier aktuell nur einen **100nF** Kondensator. Ist das schlecht?
* **Antwort**: Für **wenige LEDs** (z.B. nur 1-2 an) **reicht 100nF** oft aus.
  * Aber wenn **alle 9 LEDs** gleichzeitig PWM dimmen (z.B. beim Start), kann die Spannung kurz einbrechen. Das kann dazu führen, dass der ESP32 neustartet (Brownout).
  * **Empfehlung**: Plane im PCB lieber den Platz für einen **10µF** (0805 oder 1206) ein. Bestücken kannst du ihn zur Not erst später. Ein 100nF an dieser Stelle ist besser als nichts, aber evt. zu wenig "Tankvolumen".

* **Platzierung**: Irgendwo auf der 3.3V Leitung, **bevor** diese sich auf die LEDs/PCA aufteilt. Oft in der Nähe des Spannungsreglers (falls vorhanden) oder am Stecker-Eingang.
* **Anschluss**:
  * Eine Seite an **3.3V**.
  * Andere Seite an **GND**.

### 2. Kondensator 100nF (0603) - "Der Entstörer" (WICHTIG!)

Dieser filtert hochfrequente Störungen direkt am Chip. Ohne diesen stürzt der I2C-Bus ab.

* **Platzierung**: **So nah wie physikalisch möglich** an **Pin 28 (VDD)** des PCA9685.
* **Anschluss**:
  * Eine Seite direkt an **Pin 28 (VDD)**.
  * Andere Seite direkt an **Pin 14 (VSS/GND)** (oder die nächste GND-Fläche).
  * Strompfad: Der Strom sollte von der 3.3V Quelle *durch* das Pad des Kondensators in den Pin fließen (Idealfall).

### 3. Widerstand 10kΩ für OE (Output Enable)

Der OE-Pin (Pin 23) schaltet die Ausgänge an oder aus.

* **Funktion**: `LOW` (GND) = Ausgänge AKTIV. `HIGH` (3.3V) = Ausgänge DEAKTIVIERT.
* **Anschluss-Variante A ("Immer an" - Empfohlen):**
  * Verbinde **Pin 23 (OE)** über den **10kΩ Widerstand nach GND** (Pull-Down).
  * *Effekt*: Der Chip ist immer aktiv. (Der Widerstand ist hier "optional", man könnte auch direkt verbinden, aber der Widerstand ist sicherer gegen Layout-Fehler).
* **Anschluss-Variante B ("Steuerbar" via ESP32) - ✅ UMGESETZT auf PCB:**
  * Verbinde **Pin 23 (OE)** an **XIAO D3 (GPIO21)**.
  * Verbinde den **10kΩ Widerstand** zwischen **Pin 23** und **3.3V** (Pull-Up).
  * *Effekt*: Beim Start sind alle LEDs AUS (durch Widerstand). Der ESP32 zieht den Pin aktiv auf LOW, um die LEDs einzuschalten.

**Zusammenfassung für dein Layout:**

| Bauteil | Pin A | Pin B | Ort |
| :--- | :--- | :--- | :--- |
| **10µF Cap** | 3.3V Netz | GND Netz | Eingangsbereich (nah am FFC VCC) |
| **100nF Cap** | PCA Pin 28 (VDD) | PCA Pin 14 (VSS) | **Direkt am Chip** |
| **10kΩ Res** | PCA Pin 23 (OE) | **3.3V (Pull-Up)** | Nahe Pin 23 (**Verbunden mit XIAO D3**) |
