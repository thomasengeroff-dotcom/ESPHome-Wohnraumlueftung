# 🧠 Ventilation Logic Component

This component provides a pure, hardware-agnostic C++ utility library (`VentilationLogic`) containing deterministic math, fan PWM calculations, and thermal physics formulas. It has zero external dependencies, making it fully testable via native C++ unit tests (`tests/`).

## 📄 File Overview

| File | Description |
| :--- | :--- |
| **`ventilation_logic.h` / `.cpp`** | **Static Math & Logic Utilities (`VentilationLogic`)**: Stateless, pure functions for heat recovery efficiency calculation, bidirectional fan PWM mapping, dynamic cycle timing, virtual RPM estimation, and linear ramping. |
| **`__init__.py`** | **ESPHome Code Injector**: Registers the component with ESPHome and injects the `#include` header globally into the generated firmware build. |

## ⚙️ Key Calculations & Functions

- **Heat Recovery Efficiency ($\eta_{WRG}$)**: Computes sensible heat recovery percentage $\frac{T_{\text{supply}} - T_{\text{outdoor}}}{T_{\text{indoor}} - T_{\text{outdoor}}} \times 100$ with $\Delta T \ge 2\,^\circ\text{C}$ guard condition against sensor noise.
- **Bidirectional Fan PWM**: Translates target speed $[0.1 \dots 1.0]$ and direction into the reversible VentoMaxx PWM duty cycle ($50\% = \text{Stop}$, $<50\% = \text{Exhaust}$, $>50\% = \text{Intake}$).
- **Dynamic WRG Cycle Duration**: Scales the direction reversal interval dynamically from 70 seconds (Level 1) down to 50 seconds (Level 10) to optimize heat transfer at higher airflow volumes.
- **Virtual RPM & Software Ramping**: Provides linear acceleration ramp calculations ($0.0 \dots 1.0$) and virtual tachometer RPM estimation.
- **Unit Test Coverage**: Directly validated by native desktop test runners without requiring an ESP32 hardware target.
