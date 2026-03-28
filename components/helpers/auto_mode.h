#pragma once
#include "helpers/globals.h"

inline void evaluate_auto_mode() {
  if (!auto_mode_active->value() || !system_on->value() ||
      !ventilation_enabled->value())
    return;

  auto *v = ventilation_ctrl;
  // Default to the configured base level from Home Assistant (Moisture
  // protection / base ventilation)
  int target_level = automatik_min_fan_level->value();
  int internal_mode =
      v->state_machine.current_mode; // Copy current mode as starting point

  // Determine current hardware direction to correctly map NTC sensors during
  // MODE_VENTILATION
  if (v == nullptr)
    return;
  const bool is_intake =
      v->state_machine.get_target_state(millis()).direction_in;

  // --- 1. Compute Local Temperatures ---
  float local_in = NAN;
  float local_out = NAN;

  // SCD41 is always a valid indoor temperature if available
  if (temperature != nullptr && !std::isnan(temperature->state)) {
    local_in = temperature->state;
  }

  if (internal_mode == esphome::MODE_VENTILATION) {
    // Continuous flow mode: NTC sensors show true external/internal air
    // depending on direction
    if (is_intake) {
      // Blowing IN: Exhaust NTC gets true outside air, Supply NTC gets
      // mixed/flow air (ignore)
      if (temp_abluft != nullptr)
        local_out = temp_abluft->state;
    } else {
      // Blowing OUT: Supply NTC gets true inside air, Exhaust NTC gets
      // mixed/flow (ignore)
      if (std::isnan(local_in) && temp_zuluft != nullptr)
        local_in = temp_zuluft->state;
    }
  } else if (internal_mode == esphome::MODE_ECO_RECOVERY) {
    // WRG mode: Stable NTC values are valid representations (thanks to the 45s
    // filter logic)
    if (temp_abluft != nullptr)
      local_out = temp_abluft->state;
    if (std::isnan(local_in) && temp_zuluft != nullptr)
      local_in = temp_zuluft->state;
  }

  // Store in controller for ESP-NOW broadcast
  v->local_t_in = local_in;
  v->local_t_out = local_out;

  // --- 2. Compute Effective Temperatures (Group-wide) ---
  float eff_in = local_in;
  float eff_out = local_out;

  const uint32_t now = millis();
  // Use peer indoor temp if local is missing and peer value is fresh (<5 mins)
  if (std::isnan(eff_in) && !std::isnan(v->last_peer_t_in) &&
      (now - v->last_peer_t_in_time < PEER_TIMEOUT_MS)) {
    eff_in = v->last_peer_t_in;
  }
  // Use peer outdoor temp if local is missing and peer value is fresh (<5 mins)
  if (std::isnan(eff_out) && !std::isnan(v->last_peer_t_out) &&
      (now - v->last_peer_t_out_time < PEER_TIMEOUT_MS)) {
    eff_out = v->last_peer_t_out;
  }

  // --- 3. Summer Cooling Logic ---
  // [NEW v4] Strictly follows:
  // - Activation: Indoor > 22.0°C AND Outdoor cooler by at least 1.5°C
  // - Hysteresis: Disengage when Outdoor is < 0.5°C cooler than Indoor (or
  // warmer)
  if (!std::isnan(eff_in) && !std::isnan(eff_out)) {
    if (internal_mode != esphome::MODE_VENTILATION) {
      if (eff_in > 22.0f && eff_out < (eff_in - 1.5f)) {
        internal_mode = esphome::MODE_VENTILATION;
        ESP_LOGI("auto_mode",
                 "Sommer-Kühlung AKTIVIERT: Innen=%.1f°C, Außen=%.1f°C (Delta "
                 "> 1.5°C)",
                 eff_in, eff_out);
      } else if (internal_mode != esphome::MODE_ECO_RECOVERY) {
        internal_mode = esphome::MODE_ECO_RECOVERY;
      }
    } else {
      // Return to WRG if outdoor is no longer significantly cooler or indoor is
      // cool enough (<21.5 for basic hysteresis)
      if (eff_out >= (eff_in - 0.5f) || eff_in < 21.5f) {
        internal_mode = esphome::MODE_ECO_RECOVERY;
        ESP_LOGI("auto_mode",
                 "Sommer-Kühlung DEAKTIVIERT (Hysterese erreicht): "
                 "Innen=%.1f°C, Außen=%.1f°C",
                 eff_in, eff_out);
      }
    }
  } else if (internal_mode != esphome::MODE_ECO_RECOVERY &&
             internal_mode != esphome::MODE_VENTILATION) {
    internal_mode = esphome::MODE_ECO_RECOVERY;
  }

  // --- 4. Air Quality logic (CO2 & Humidity via PID) ---
  float max_pid_demand = 0.0f;

  if (co2_auto_enabled->value() && co2_pid_result != nullptr) {
    max_pid_demand = std::max(max_pid_demand, co2_pid_result->value());
  }

  if (scd41_humidity != nullptr && outdoor_humidity != nullptr) {
    const float in_hum = scd41_humidity->state;
    const float out_hum = outdoor_humidity->state;

    if (!std::isnan(in_hum) && !std::isnan(out_hum)) {
      if (out_hum < in_hum && humidity_pid_result != nullptr) {
        // Only use humidity demand if outdoor is drier
        max_pid_demand = std::max(max_pid_demand, humidity_pid_result->value());
      }
    }
  }

  // Save local demand to controller for ESP-NOW broadcast
  // Every second, this device tells the group how much ventilation it currently
  // demands.
  if (std::abs(max_pid_demand - v->local_pid_demand) > 0.05f) {
    v->trigger_sync();
  }
  v->local_pid_demand = max_pid_demand;

  // Compare with peer demand if fresh (<5 mins)
  // Synchronized ESP-NOW PID Logic:
  // If a peer device in the same room reports a higher air-clearing demand
  // (because someone is breathing near it, or humidity spiked locally), we
  // adopt that higher demand. This forces all devices in the room to scale up
  // together symmetrically, preventing situations where one device runs loudly
  // at 100% while the other idles at 10%.
  if (!std::isnan(v->last_peer_pid_demand) &&
      (now - v->last_peer_pid_demand_time < PEER_TIMEOUT_MS)) {
    max_pid_demand = std::max(max_pid_demand, v->last_peer_pid_demand);
  }
  // Clamp aggregated demand to [0.0, 1.0] range
  max_pid_demand = std::max(0.0f, std::min(1.0f, max_pid_demand));

  if (max_pid_demand > 0.01f) {
    int min_level = automatik_min_fan_level->value();
    int max_level = automatik_max_fan_level->value();

    // Calculate proportional LED target level (1-10 display feedback)
    // Note: The physical motor receives a precision float (0.0 - 1.0) via
    // update_fan_logic() for absolutely silent, stepless transitions. This
    // 'target_level' is kept just so the intensity LEDs on the unit faceplate
    // show a representative average.
    float scaled_level = min_level + max_pid_demand * (max_level - min_level);
    target_level = std::max(target_level, (int)std::round(scaled_level));
    // Clamp to hardware boundaries (1-10)
    target_level = std::max(1, std::min(10, target_level));

    ESP_LOGD("auto_mode",
             "PID Demand=%.2f (Local=%.2f, Peer=%.2f) -> Boost level to %d",
             max_pid_demand, v->local_pid_demand, v->last_peer_pid_demand,
             target_level);
  }

  // 5. Presence logic - Removed from Automatik mode as requested.
  // Presence compensation is now handled directly in get_current_target_speed()
  // for manual modes instead.

  // Apply the computed logic
  if (v->state_machine.current_mode != internal_mode) {
    v->set_mode((esphome::VentilationMode)internal_mode);
  }

  if (fan_intensity_level->value() != target_level) {
    fan_intensity_level->value() = target_level;
    fan_intensity_display->publish_state(target_level);
    ventilation_ctrl->set_fan_intensity(target_level);
    fan_speed_update->execute();
    update_leds->execute();
    ESP_LOGI("auto_mode", "Automode updated fan level to %d", target_level);
  }
}

/// @brief Sets fan PWM for the ebm-papst VarioPro reversible fan.
/// This fan uses a SINGLE PWM signal for both speed and direction:
///   - 50.0% duty cycle = STOP (active brake)
///   - 50% → 0%   = Direction: Abluft (Raus) (Stufe 1 @ 30%, Stufe 10 @ 5%)
///   - 50% → 100% = Direction: Zuluft (Rein) (Stufe 1 @ 70%, Stufe 10 @ 95%)
/// @param speed     Target speed as duty cycle fraction (0.1 = min, 1.0 = max).
/// @param direction Target direction: 0 = Abluft (PWM < 50%), 1 = Zuluft (PWM >
/// 50%).
