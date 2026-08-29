# 🏛️ Systemarchitektur – ESPHome Lüftungssteuerung

[![Language: EN](https://img.shields.io/badge/Language-EN-blue.svg)](../en/en_system-architecture.md)

Dieses Dokument bietet einen Überblick über die Hardware- und Softwarearchitektur des smarten dezentralen Lüftungssystems VentoSync.

## Hardware-Architektur

Das System basiert auf dem **Seeed Studio XIAO ESP32-C6** mit 32-Bit RISC-V Kern und Multi-Protokoll-Konnektivität (Wi-Fi 6, Bluetooth 5, Zigbee/Thread, ESP-NOW).

```mermaid
graph TB
    subgraph MCU ["Zentraler MCU (XIAO ESP32-C6)"]
        ESP32[ESP32-C6 RISC-V]
    end

    subgraph I2C_Bus ["I2C-Bus (3.3V)"]
        MCP[MCP23017 GPIO-Expander] -->|FFC-Kabel| Panel[Physisches Bedienpanel]
        PCA[PCA9685 PWM-Treiber] -->|LED-Ansteuerung| Panel
        SCD[SCD43 CO2/Temp/Feuchte]
        BMP[BMP390 Druck/Höhe]
    end

    subgraph Direct_IO ["Direkte IO & Sensoren"]
        Fan[Lüfter 12V PWM/Tacho]
        NTC_In[NTC Zuluft-Temp]
        NTC_Out[NTC Abluft-Temp]
        Radar[HLK-LD2450 mmWave-Radar]
    end

    subgraph Power ["Spannungsversorgung"]
        AC[230V AC] -->|TRACO POWER Netzteil| 12V[12V DC]
        12V -->|Buck-Converter| 5V[5V DC]
        12V -->|Buck-Converter| 3V3[3.3V DC]
    end

    ESP32 --- I2C_Bus
    ESP32 ---|D8 PWM / D9 Tacho| Fan
    ESP32 ---|D0/D1 ADC| NTC_In
    ESP32 ---|D0/D1 ADC| NTC_Out
    ESP32 ---|D6/D7 UART| Radar
    
    5V --- MCU
    3V3 --- MCU
    12V --- Fan
```

## Software- & Konnektivitäts-Architektur

Das Projekt kombiniert den modularen Aufbau von ESPHome mit maßgeschneiderten C++ Komponenten für das Dashboard und die Mesh-Synchronisation.

```mermaid
graph LR
    subgraph Software_Layers ["ESPHome Software-Stack"]
        direction TB
        Core[ESPHome Core]
        Logic[Logik & Automatisierung<br/>PID-Regler, Betriebsmodi]
        CustomCPP["Custom C++ Komponenten"]
        subgraph CustomCPP_Detail ["Dashboard & Mesh"]
            Dashboard[wrg_dashboard<br/>Async Web + Tailwind]
            Mesh[ventilation_group<br/>ESP-NOW Sync]
        end
    end

    subgraph Connectivity ["Konnektivität"]
        WiFi[WLAN 2.4GHz]
        ESPNOW[ESP-NOW Mesh]
    end

    subgraph External_Systems ["Externe Schnittstellen"]
        HA[Home Assistant]
        Browser[Webbrowser]
    end

    Core --- Logic
    Core --- CustomCPP
    Logic --- Mesh
    Dashboard --- Browser
    Core --- WiFi
    WiFi --- HA
    HA ---|Zustandsaktualisierungen| Core
    Mesh --- ESPNOW
    ESPNOW ---|Sync-Daten| PeerNodes[Peer-Lüftungsgeräte]
```

## Funktionsübersicht

1. **Sensoren**: Erfassung von Umweltdaten über SCD43, BMP390, mmWave-Radar und NTCs.
2. **Logik**: Die Schicht `logic_automation` verarbeitet Sensordaten über PID-Regler zur Ermittlung der optimalen Lüfterdrehzahl.
3. **Mesh-Synchronisierung**: Die Komponente `ventilation_group` stellt über ESP-NOW sicher, dass alle Geräte in einem Raum/Stockwerk mit synchronisierten Phasen und Betriebsmodi arbeiten.
4. **Dashboard**: Die Komponente `wrg_dashboard` stellt eine performante Echtzeit-Weboberfläche (mit Tailwind CSS & Chart.js) unabhängig von Home Assistant bereit.
5. **Bedienung**: Benutzereingaben über das physische Panel (MCP23017), die Web-UI oder Home Assistant werden verarbeitet und im Gruppenverbund synchronisiert.
