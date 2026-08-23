# System Architecture - ESPHome Ventilation Control

This document provides a high-level overview of the hardware and software architecture of the smart ventilation system.

## Hardware Architecture

The system is centered around the **Seeed Studio XIAO ESP32-C6**, utilizing its RISC-V core and multi-protocol connectivity.

```mermaid
graph TB
    subgraph MCU ["Central MCU (XIAO ESP32-C6)"]
        ESP32[ESP32-C6 RISC-V]
    end

    subgraph I2C_Bus ["I2C Bus (3.3V)"]
        MCP[MCP23017 GPIO Expander] -->|FFC Connection| Panel[Physical Control Panel]
        PCA[PCA9685 PWM Driver] -->|LED Control| Panel
        SCD[SCD41 CO2/Temp/Hum]
        BMP[BMP390 Pressure/Alt]
    end

    subgraph Direct_IO ["Direct IO & Sensors"]
        Fan[Fan 12V PWM/Tacho]
        NTC_In[NTC Intake Temp]
        NTC_Out[NTC Exhaust Temp]
        Radar[HLK-LD2450 mmWave Radar]
    end

    subgraph Power ["Power Management"]
        AC[230V AC] -->|TRACO POWER| 12V[12V DC]
        12V -->|Buck Converter| 5V[5V DC]
        12V -->|Buck Converter| 3V3[3.3V DC]
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

## Software & Connectivity Architecture

The project leverages the modularity of ESPHome combined with custom C++ components for the dashboard and mesh synchronization.

```mermaid
graph LR
    subgraph Software_Layers ["ESPHome Software Stack"]
        direction TB
        Core[ESPHome Core]
        Logic[Logic & Automation<br/>PID Controllers, Modes]
        CustomCPP["Custom C++ Components"]
        subgraph CustomCPP_Detail ["Dashboard & Mesh"]
            Dashboard[wrg_dashboard<br/>Async Web + Tailwind]
            Mesh[ventilation_group<br/>ESP-NOW Sync]
        end
    end

    subgraph Connectivity
        WiFi[WiFi 2.4GHz]
        ESPNOW[ESP-NOW Mesh]
    end

    subgraph External_Systems ["External Interface"]
        HA[Home Assistant]
        Browser[Web Browser]
    end

    Core --- Logic
    Core --- CustomCPP
    Logic --- Mesh
    Dashboard --- Browser
    Core --- WiFi
    WiFi --- HA
    HA ---|State Updates| Core
    Mesh --- ESPNOW
    ESPNOW ---|Sync Data| PeerNodes[Peer Ventilation Nodes]
```

## Functional Overview

1.  **Sensors**: Data is collected from SCD41, BMP390, Radar, and NTCs.
2.  **Logic**: The `logic_automation` layer processes sensor data using PID controllers to determine the optimal fan speed.
3.  **Mesh Sync**: The `ventilation_group` component via ESP-NOW ensures all nodes in a room/floor operate with synchronized phases and modes.
4.  **Dashboard**: The `wrg_dashboard` provides a real-time, high-performance UI (using Tailwind CSS) independent of Home Assistant.
5.  **Control**: User input from the physical panel (via MCP23017), Web UI, or Home Assistant is processed and synchronized across the group.
