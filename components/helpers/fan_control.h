#pragma once
#include "helpers/globals.h"

inline void set_fan_logic(float speed, int direction) {
  if (fan_pwm_primary == nullptr)
    return;

  float pwm = VentilationLogic::calculate_fan_pwm(speed, direction);

  fan_pwm_primary->set_level(pwm);
  last_fan_pwm_level = pwm;
}

/// @brief Converts a user-facing level (1-10) to actual hardware PWM speed (10%
/// - 100%)
inline float level_to_speed(float level) {
  return VentilationLogic::calculate_fan_speed_from_intensity(
      (int)std::round(level));
}

/// @brief Calculates the exact target speed (0.0 to 1.0) based on intensity
/// level and auto PID rules.
inline float get_current_target_speed() {
  if (fan_intensity_level == nullptr)
    return 0.0f;

  float intensity = (float)fan_intensity_level->value();

  if (auto_mode_active != nullptr && auto_mode_active->value()) {
    // --- 1. Automatik Mode (PID based) ---
    float max_pid_demand = 0.0f;
    if (co2_auto_enabled->value() && co2_pid_result != nullptr) {
      max_pid_demand = std::max(max_pid_demand, co2_pid_result->value());
    }
    if (humidity_pid_result != nullptr) {
      max_pid_demand = std::max(max_pid_demand, humidity_pid_result->value());
    }
    if (ventilation_ctrl != nullptr) {
      const uint32_t now = millis();
      // FIXED W3: Use has_peer_pid_demand flag — time==0 sentinel unreliable
      // after overflow
      if (ventilation_ctrl->has_peer_pid_demand &&
          (now - ventilation_ctrl->last_peer_pid_demand_time <
           PEER_TIMEOUT_MS)) {
        max_pid_demand =
            std::max(max_pid_demand, ventilation_ctrl->last_peer_pid_demand);
      }
    }
    // Clamp aggregated demand to [0.0, 1.0] range
    max_pid_demand = std::max(0.0f, std::min(1.0f, max_pid_demand));

    float min_l = (float)automatik_min_fan_level->value();
    float max_l = (float)automatik_max_fan_level->value();

    if (max_pid_demand > 0.01f) {
      intensity = min_l + max_pid_demand * (max_l - min_l);
    } else {
      intensity = min_l;
    }
  } else {
    // --- 2. Manual Modes (WRG, Ventilation, Stoßlüftung) ---
    // Apply presence compensation only in non-automatic modes
    if (radar_presence != nullptr && radar_presence->has_state() &&
        radar_presence->state) {
      float comp = (float)auto_presence_val->value();
      if (comp != 0.0f) {
        intensity += comp;
        ESP_LOGD("fan", "Applying presence compensation: %.1f (Base: %d)", comp,
                 fan_intensity_level->value());
      }
    }
  }

  // Clamp to valid 1-10 level domain
  intensity = std::max(1.00f, std::min(10.00f, intensity));
  return level_to_speed(intensity);
}

/// @brief Calculates the virtual fan RPM including ramping and air direction.
/// @param raw_rpm The physical RPM reading from the pulse counter (if any).
inline float calculate_virtual_fan_rpm(float raw_rpm) {
  if (raw_rpm > 10.0f) { // Real 4-pin signal
    return raw_rpm;
  }

  float speed = get_current_target_speed();
  bool direction_in = true;
  float ramp_factor = 1.0f;

  if (ventilation_ctrl != nullptr) {
    esphome::HardwareState state =
        ventilation_ctrl->state_machine.get_target_state(millis());
    ramp_factor = state.ramp_factor;
    direction_in = state.direction_in;
  } else if (fan_direction != nullptr && !fan_direction->state) {
    direction_in = false;
  }

  return VentilationLogic::calculate_virtual_fan_rpm(speed, direction_in,
                                                     ramp_factor);
}

/// @brief Updates filter operating hours and initializes transition timestamp.
/// Called every 60s from logic_automation.yaml.
inline void update_fan_logic() {
  if (fan_intensity_level == nullptr)
    return;
  const int intensity = fan_intensity_level->value();

  // Dynamic Cycle duration mapped to fan level:
  if (ventilation_ctrl != nullptr) {
    uint32_t dynamic_cycle_ms =
        VentilationLogic::calculate_dynamic_cycle_duration(intensity);
    if (ventilation_ctrl->state_machine.cycle_duration_ms != dynamic_cycle_ms) {
      ventilation_ctrl->set_cycle_duration(dynamic_cycle_ms);
    }
  }

  float speed = get_current_target_speed();

  // Apply software ramping factor if a controller is available
  if (ventilation_ctrl != nullptr) {
    // FIXED #4: Use fully-qualified esphome::HardwareState (matches
    // calculate_virtual_fan_rpm)
    esphome::HardwareState state =
        ventilation_ctrl->state_machine.get_target_state(millis());
    speed *= state.ramp_factor;

    // Rate-limited log during transition phases for debugging
    const uint32_t now = millis();
    if (state.ramp_factor < 0.99f && state.ramp_factor > 0.01f &&
        (now - ventilation_ctrl->last_ramp_log_ms > 1000)) {
      ESP_LOGD("fan_ramp", "Ramping speed: %.2f (Base: %.2f, Factor: %.2f)",
               speed, get_current_target_speed(), state.ramp_factor);
      ventilation_ctrl->last_ramp_log_ms = now;
    }
  }

  // Read current direction from the fan_direction switch:
  // OFF = Direction: Abluft (Raus) (PWM < 50%)
  // ON  = Direction: Zuluft (Rein) (PWM > 50%)
  const int direction =
      (fan_direction != nullptr && fan_direction->state) ? 1 : 0;

  set_fan_logic(speed, direction);
}

/// @brief Returns true if the manual speed slider is below the off threshold.
inline bool is_fan_slider_off(float value) {
  return VentilationLogic::is_fan_slider_off(value);
}

/// @brief Linear ramp-up for soft fan start (0.0…1.0 over 100 iterations).
inline float calculate_ramp_up(int iteration) {
  return VentilationLogic::calculate_ramp_up(iteration);
}

/// @brief Linear ramp-down for soft fan stop (1.0…0.0 over 100 iterations).
inline float calculate_ramp_down(int iteration) {
  return VentilationLogic::calculate_ramp_down(iteration);
}

/// @brief Applies the operating mode corresponding to the given index.
/// Index mapping: 0 = Automatik, 1 = WRG, 2 = Durchlüften, 3 = Stoßlüftung, 4 =
/// Aus. Manages system_on/ventilation_enabled state, syncs HA select, and
/// triggers fan update.
