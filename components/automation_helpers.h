// ==========================================================================
// WRG Wohnraumlüftung – ESPHome Custom Component
// https://github.com/thomasengeroff-dotcom/ESPHome-Wohnraumlueftung
//
// Copyright (c) 2026 Thomas Engeroff
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// File:        automation_helpers.h
// Description: Helper functions for ESPHome YAML automations.
// Author:      Thomas Engeroff
// Created:     2026-03-07
// Modified:    2026-03-26
// ==========================================================================
#pragma once

// ARCHITECTURE NOTE:
// This header is #included directly from ESPHome's generated main.cpp.
// All component IDs (system_on, ventilation_ctrl, fan_direction, etc.) are
// declared as 'static' variables in main.cpp and are therefore visible here
// without any extern declarations or manual registration.
//
// DO NOT include this header from any other .cpp file.

#include "esp_mac.h"
#include "esphome.h"
#include <algorithm>
#include <deque>
#include <string>
#include <vector>

// Core and Component Headers
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/globals/globals_component.h"
#include "esphome/components/homeassistant/sensor/homeassistant_sensor.h"
#include "esphome/components/light/light_state.h"
#include "esphome/components/ntc/ntc.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/script/script.h"
#include "esphome/components/template/number/template_number.h"
#include "esphome/components/template/select/template_select.h"
#include "esphome/components/template/sensor/template_sensor.h"
#include "esphome/components/template/switch/template_switch.h"
#include "esphome/components/template/text_sensor/template_text_sensor.h"

// Custom Component Headers
#include "esphome/components/ventilation_group/ventilation_group.h"
#include "esphome/components/ventilation_logic/ventilation_logic.h"

// --- Global state variables for automation helpers ---

inline float last_fan_pwm_level = 0.5f;
inline std::deque<float> ntc_history[2];
inline uint32_t last_direction_change_time = 0;

static const uint32_t PEER_TIMEOUT_MS = 300000; // 5 minutes
static const float NTC_MAX_DEVIATION = 0.3f; // Max allowed deviation in window

// --- Math & Logic Wrappers ---

inline float level_to_speed(float level) {
  return VentilationLogic::calculate_fan_speed_from_intensity((int)level);
}

inline std::string get_co2_classification(float co2_ppm) {
  return VentilationLogic::get_co2_classification(co2_ppm);
}

// --- Component Helper functions ---

inline float get_current_target_speed() {
  if (fan_intensity_level == nullptr)
    return 0.5f;

  float intensity = (float)fan_intensity_level->value();

  if (auto_mode_active != nullptr && auto_mode_active->value()) {
    // --- 1. Automatik Mode (PID based) ---
    float max_pid_demand = 0.0f;
    if (co2_auto_enabled != nullptr && co2_auto_enabled->value() &&
        co2_pid_result != nullptr) {
      max_pid_demand = std::max(max_pid_demand, co2_pid_result->value());
    }
    if (humidity_pid_result != nullptr) {
      max_pid_demand = std::max(max_pid_demand, humidity_pid_result->value());
    }
    if (ventilation_ctrl != nullptr) {
      const uint32_t now = millis();
      if (ventilation_ctrl->has_peer_pid_demand &&
          (now - ventilation_ctrl->last_peer_pid_demand_time <
           PEER_TIMEOUT_MS)) {
        max_pid_demand =
            std::max(max_pid_demand, ventilation_ctrl->last_peer_pid_demand);
      }
    }

    float min_l = (float)automatik_min_fan_level->value();
    float max_l = (float)automatik_max_fan_level->value();

    if (max_pid_demand > 0.01f) {
      intensity = min_l + max_pid_demand * (max_l - min_l);
    } else {
      intensity = min_l;
    }
  } else {
    // --- 2. Manual Modes (WRG, Ventilation, Stoßlüftung) ---
    if (radar_presence != nullptr && radar_presence->has_state() &&
        radar_presence->state) {
      if (auto_presence_val != nullptr) {
        float comp = (float)auto_presence_val->value();
        if (comp != 0.0f) {
          intensity += comp;
        }
      }
    }
  }

  intensity = std::max(1.00f, std::min(10.00f, intensity));
  return level_to_speed(intensity);
}

inline float calculate_virtual_fan_rpm(float raw_rpm) {
  float speed = get_current_target_speed();
  bool direction_in = true;
  float ramp_factor = 1.0f;
  if (ventilation_ctrl != nullptr) {
    auto state = ventilation_ctrl->state_machine.get_target_state(millis());
    direction_in = state.direction_in;
    ramp_factor = state.ramp_factor;
  }
  return VentilationLogic::calculate_virtual_fan_rpm(speed, direction_in,
                                                     ramp_factor);
}

inline float calculate_virtual_fan_rpm(float speed, bool direction_is_intake,
                                       float ramp_factor = 1.0f) {
  return VentilationLogic::calculate_virtual_fan_rpm(speed, direction_is_intake,
                                                     ramp_factor);
}

inline float calculate_heat_recovery_efficiency(float t_raum, float t_zuluft,
                                                float t_aussen) {
  return VentilationLogic::calculate_heat_recovery_efficiency(t_raum, t_zuluft,
                                                              t_aussen);
}

// --- ESP-NOW Utilities ---

inline esphome::optional<std::array<uint8_t, 6>>
parse_mac_local(const std::string &str) {
  std::array<uint8_t, 6> res;
  if (sscanf(str.c_str(), "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX", &res[0], &res[1],
             &res[2], &res[3], &res[4], &res[5]) == 6)
    return res;
  return {};
}

inline bool is_local_mac_addr(const uint8_t *mac) {
  uint8_t local_mac[6];
  esp_read_mac(local_mac, ESP_MAC_WIFI_STA);
  for (int i = 0; i < 6; i++) {
    if (mac[i] != local_mac[i])
      return false;
  }
  return true;
}

inline void register_peer_dynamic(const uint8_t *mac) {
  if (!esphome::espnow::global_esp_now)
    return;
  if (is_local_mac_addr(mac))
    return;

  char mac_buf[20];
  snprintf(mac_buf, sizeof(mac_buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);
  std::string mac_str(mac_buf);

  std::string &current_list = espnow_peers->value();
  if (current_list.find(mac_str) == std::string::npos) {
    if (!current_list.empty())
      current_list += ",";
    current_list += mac_str;
    ESP_LOGI("espnow_disc", "New peer discovered and saved: %s",
             mac_str.c_str());
  }
}

inline void load_peers_from_runtime_cache() {
  if (!esphome::espnow::global_esp_now)
    return;
  std::string current_list = espnow_peers->value();
  if (current_list.empty())
    return;

  size_t start = 0;
  size_t end = current_list.find(",");
  while (true) {
    std::string mac_str = (end == std::string::npos)
                              ? current_list.substr(start)
                              : current_list.substr(start, end - start);
    auto mac = parse_mac_local(mac_str);
    if (mac.has_value()) {
      esphome::espnow::global_esp_now->add_peer(mac->data());
    }
    if (end == std::string::npos)
      break;
    start = end + 1;
    end = current_list.find(",", start);
  }
}

inline void send_discovery_broadcast() {
  if (!esphome::espnow::global_esp_now)
    return;
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "ROOM_DISC:%d:%d",
           (int)config_floor_id->state, (int)config_room_id->state);
  std::string msg(buffer);
  std::vector<uint8_t> data(msg.begin(), msg.end());
  uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esphome::espnow::global_esp_now->send(broadcast_mac, data.data(),
                                        data.size());
}

inline void send_discovery_confirmation(const uint8_t *target_mac) {
  if (!esphome::espnow::global_esp_now)
    return;
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "ROOM_CONF:%d:%d",
           (int)config_floor_id->state, (int)config_room_id->state);
  std::string msg(buffer);
  std::vector<uint8_t> data(msg.begin(), msg.end());
  esphome::espnow::global_esp_now->send(target_mac, data.data(), data.size());
}

inline void send_sync_to_all_peers(const std::vector<uint8_t> &data) {
  if (!esphome::espnow::global_esp_now)
    return;
  std::string current_list = espnow_peers->value();
  if (current_list.empty())
    return;
  size_t start = 0;
  size_t end = current_list.find(",");
  while (true) {
    std::string mac_str = (end == std::string::npos)
                              ? current_list.substr(start)
                              : current_list.substr(start, end - start);
    auto mac = parse_mac_local(mac_str);
    if (mac.has_value())
      esphome::espnow::global_esp_now->send(mac->data(), data.data(),
                                            data.size());
    if (end == std::string::npos)
      break;
    start = end + 1;
    end = current_list.find(",", start);
  }
}

inline bool handle_discovery_payload(const std::string &payload,
                                     const uint8_t *src_addr) {
  if (payload.find("ROOM_DISC:") == 0 || payload.find("ROOM_CONF:") == 0) {
    if (is_local_mac_addr(src_addr))
      return true;
    int floor, room;
    if (sscanf(payload.c_str() + 10, "%d:%d", &floor, &room) == 2) {
      if (floor == (int)config_floor_id->state &&
          room == (int)config_room_id->state) {
        register_peer_dynamic(src_addr);
        if (payload.find("ROOM_DISC:") == 0)
          send_discovery_confirmation(src_addr);
        return true;
      }
    }
  }
  return false;
}

// --- Status & UI Helpers ---

inline void update_leds_logic() {
  if (ui_active == nullptr || !ui_active->value()) {
    // Dim all LEDs when inactive
    if (status_led_l1)
      status_led_l1->turn_off().perform();
    if (status_led_l2)
      status_led_l2->turn_off().perform();
    if (status_led_l3)
      status_led_l3->turn_off().perform();
    if (status_led_l4)
      status_led_l4->turn_off().perform();
    if (status_led_l5)
      status_led_l5->turn_off().perform();
    if (status_led_power)
      status_led_power->turn_off().perform();
    if (status_led_master)
      status_led_master->turn_off().perform();
    if (status_led_mode_wrg)
      status_led_mode_wrg->turn_off().perform();
    if (status_led_mode_vent)
      status_led_mode_vent->turn_off().perform();
    return;
  }

  float max_b = max_led_brightness->value();

  // Power & Mode LEDs
  if (system_on->value()) {
    if (status_led_power)
      status_led_power->turn_on().set_brightness(max_b).perform();
  } else {
    if (status_led_power)
      status_led_power->turn_off().perform();
  }

  auto mode = ventilation_ctrl->state_machine.current_mode;
  if (mode == esphome::MODE_ECO_RECOVERY) {
    if (status_led_mode_wrg)
      status_led_mode_wrg->turn_on().set_brightness(max_b).perform();
    if (status_led_mode_vent)
      status_led_mode_vent->turn_off().perform();
  } else if (mode == esphome::MODE_VENTILATION) {
    if (status_led_mode_wrg)
      status_led_mode_wrg->turn_off().perform();
    if (status_led_mode_vent)
      status_led_mode_vent->turn_on().set_brightness(max_b).perform();
  } else {
    if (status_led_mode_wrg)
      status_led_mode_wrg->turn_off().perform();
    if (status_led_mode_vent)
      status_led_mode_vent->turn_off().perform();
  }

  // Level LEDs (L1-L5) mapping 1-10 intensity
  int level = fan_intensity_level->value();
  if (status_led_l1) {
    if (level >= 1)
      status_led_l1->turn_on()
          .set_brightness(level == 1 ? 0.5f * max_b : max_b)
          .perform();
    else
      status_led_l1->turn_off().perform();
  }
  if (status_led_l2) {
    if (level >= 3)
      status_led_l2->turn_on()
          .set_brightness(level == 3 ? 0.5f * max_b : max_b)
          .perform();
    else
      status_led_l2->turn_off().perform();
  }
  if (status_led_l3) {
    if (level >= 5)
      status_led_l3->turn_on()
          .set_brightness(level == 5 ? 0.5f * max_b : max_b)
          .perform();
    else
      status_led_l3->turn_off().perform();
  }
  if (status_led_l4) {
    if (level >= 7)
      status_led_l4->turn_on()
          .set_brightness(level == 7 ? 0.5f * max_b : max_b)
          .perform();
    else
      status_led_l4->turn_off().perform();
  }
  if (status_led_l5) {
    if (level >= 9)
      status_led_l5->turn_on()
          .set_brightness(level == 9 ? 0.5f * max_b : max_b)
          .perform();
    else
      status_led_l5->turn_off().perform();
  }

  // Master LED (Blinks if peer missing, else ON)
  if (status_led_master) {
    if (peer_check_enabled->value() && ventilation_ctrl->peers.empty()) {
      status_led_master->turn_on()
          .set_effect("Slow Blink")
          .set_brightness(max_b)
          .perform();
    } else {
      status_led_master->turn_on()
          .set_effect("None")
          .set_brightness(max_b)
          .perform();
    }
  }
}

inline void check_master_led_error() {
  if (ui_active->value())
    update_leds_logic();
}

// --- Fan logic wrappers ---

inline void update_fan_logic() {
  if (ventilation_ctrl == nullptr || fan_pwm_primary == nullptr)
    return;

  esphome::HardwareState hw =
      ventilation_ctrl->state_machine.get_target_state(millis());

  if (!hw.fan_enabled) {
    fan_pwm_primary->set_level(0.5f);
    last_fan_pwm_level = 0.5f;
    return;
  }

  float target_speed = get_current_target_speed();
  float pwm = VentilationLogic::calculate_fan_pwm(target_speed * hw.ramp_factor,
                                                  hw.direction_in);
  fan_pwm_primary->set_level(pwm);
  last_fan_pwm_level = pwm;
}

inline void set_fan_logic(float speed, int direction) {
  if (fan_pwm_primary == nullptr)
    return;
  float pwm = VentilationLogic::calculate_fan_pwm(speed, direction);
  fan_pwm_primary->set_level(pwm);
  last_fan_pwm_level = pwm;
}

inline void notify_fan_direction_changed() {
  last_direction_change_time = millis();
  ntc_history[0].clear();
  ntc_history[1].clear();
}

inline esphome::optional<float> filter_ntc_stable(int sensor_idx,
                                                  float new_value) {
  if (ventilation_ctrl == nullptr ||
      ventilation_ctrl->state_machine.cycle_duration_ms == 0)
    return new_value;

  uint32_t cycle_duration_ms =
      ventilation_ctrl->state_machine.cycle_duration_ms;
  uint32_t wait_time_ms =
      std::max((uint32_t)20000, (uint32_t)(cycle_duration_ms * 0.6f));
  if (wait_time_ms >= cycle_duration_ms)
    wait_time_ms = cycle_duration_ms > 5000 ? cycle_duration_ms - 5000 : 0;

  uint32_t remaining_time_ms =
      cycle_duration_ms > wait_time_ms ? cycle_duration_ms - wait_time_ms : 0;
  size_t target_window_size =
      std::max((size_t)1, (size_t)(remaining_time_ms / 3000));

  if (millis() - last_direction_change_time < wait_time_ms)
    return {};

  auto &history = ntc_history[sensor_idx];
  history.push_back(new_value);
  while (history.size() > target_window_size)
    history.pop_front();
  if (history.size() < target_window_size)
    return {};

  auto [min_it, max_it] = std::minmax_element(history.begin(), history.end());
  if ((*max_it - *min_it) <= NTC_MAX_DEVIATION)
    return new_value;
  return {};
}

// --- Mode Evaluation ---

inline void cycle_operating_mode(int mode_index) {
  auto *v = ventilation_ctrl;
  switch (mode_index) {
  case 0: // Automatik
    auto_mode_active->value() = true;
    system_on->value() = true;
    ventilation_enabled->value() = true;
    break;
  case 1: // WRG
    auto_mode_active->value() = false;
    system_on->value() = true;
    ventilation_enabled->value() = true;
    v->set_mode(esphome::MODE_ECO_RECOVERY);
    break;
  case 2: // Durchlüften
    auto_mode_active->value() = false;
    system_on->value() = true;
    ventilation_enabled->value() = true;
    v->set_mode(esphome::MODE_VENTILATION);
    break;
  case 3: // Stoßlüftung
    auto_mode_active->value() = false;
    system_on->value() = true;
    ventilation_enabled->value() = true;
    v->set_mode(esphome::MODE_STOSSLUEFTUNG);
    break;
  case 4: // Aus
    system_on->value() = false;
    ventilation_enabled->value() = false;
    v->set_mode(esphome::MODE_OFF);
    if (system_sleep)
      system_sleep->execute();
    break;
  }
  if (mode_index != 4 && system_wakeup)
    system_wakeup->execute();
  v->trigger_sync();
}

inline void evaluate_auto_mode() {
  if (auto_mode_active == nullptr || system_on == nullptr ||
      ventilation_enabled == nullptr)
    return;
  if (!auto_mode_active->value() || !system_on->value() ||
      !ventilation_enabled->value())
    return;

  auto *v = ventilation_ctrl;
  int target_level = automatik_min_fan_level->value();
  int internal_mode = v->state_machine.current_mode;
  const bool is_intake =
      v->state_machine.get_target_state(millis()).direction_in;

  float local_in = NAN;
  float local_out = NAN;

  if (temperature != nullptr && !std::isnan(temperature->state))
    local_in = temperature->state;

  if (internal_mode == esphome::MODE_VENTILATION) {
    if (is_intake) {
      if (temp_abluft != nullptr)
        local_out = temp_abluft->state;
    } else {
      if (std::isnan(local_in) && temp_zuluft != nullptr)
        local_in = temp_zuluft->state;
    }
  } else if (internal_mode == esphome::MODE_ECO_RECOVERY) {
    if (temp_abluft != nullptr)
      local_out = temp_abluft->state;
    if (std::isnan(local_in) && temp_zuluft != nullptr)
      local_in = temp_zuluft->state;
  }

  v->local_t_in = local_in;
  v->local_t_out = local_out;

  float eff_in = local_in;
  float eff_out = local_out;
  const uint32_t now = millis();

  if (std::isnan(eff_in) && !std::isnan(v->last_peer_t_in) &&
      (now - v->last_peer_t_in_time < PEER_TIMEOUT_MS))
    eff_in = v->last_peer_t_in;
  if (std::isnan(eff_out) && !std::isnan(v->last_peer_t_out) &&
      (now - v->last_peer_t_out_time < PEER_TIMEOUT_MS))
    eff_out = v->last_peer_t_out;

  if (!std::isnan(eff_in) && !std::isnan(eff_out)) {
    if (internal_mode != esphome::MODE_VENTILATION) {
      if (eff_in > 22.0f && eff_out < (eff_in - 1.5f)) {
        internal_mode = esphome::MODE_VENTILATION;
        ESP_LOGI("auto_mode", "Summer cooling engaged: In=%.1fC, Out=%.1fC",
                 eff_in, eff_out);
      } else if (internal_mode != esphome::MODE_ECO_RECOVERY) {
        internal_mode = esphome::MODE_ECO_RECOVERY;
      }
    } else {
      if (eff_out >= (eff_in - 0.5f)) {
        internal_mode = esphome::MODE_ECO_RECOVERY;
        ESP_LOGI("auto_mode", "Summer cooling aborted: In=%.1fC, Out=%.1fC",
                 eff_in, eff_out);
      }
    }
  }

  float max_pid_demand = 0.0f;
  if (co2_auto_enabled->value() && co2_pid_result != nullptr)
    max_pid_demand = std::max(max_pid_demand, co2_pid_result->value());

  if (scd41_humidity != nullptr && outdoor_humidity != nullptr) {
    const float in_hum = scd41_humidity->state;
    const float out_hum = outdoor_humidity->state;
    if (!std::isnan(in_hum) && !std::isnan(out_hum)) {
      if (out_hum < in_hum && humidity_pid_result != nullptr)
        max_pid_demand = std::max(max_pid_demand, humidity_pid_result->value());
    }
  }

  if (std::abs(max_pid_demand - v->local_pid_demand) > 0.05f)
    v->trigger_sync();
  v->local_pid_demand = max_pid_demand;

  if (!std::isnan(v->last_peer_pid_demand) &&
      (now - v->last_peer_pid_demand_time < PEER_TIMEOUT_MS))
    max_pid_demand = std::max(max_pid_demand, v->last_peer_pid_demand);

  if (max_pid_demand > 0.01f) {
    int min_level = automatik_min_fan_level->value();
    int max_level = automatik_max_fan_level->value();
    float scaled_level =
        (float)min_level + max_pid_demand * (float)(max_level - min_level);
    target_level = std::max(target_level, (int)std::round(scaled_level));
  }

  if (target_level != fan_intensity_level->value()) {
    fan_intensity_level->value() = target_level;
    if (fan_intensity_display != nullptr)
      fan_intensity_display->publish_state(target_level);
  }
  if (internal_mode != v->state_machine.current_mode)
    v->set_mode((esphome::VentilationMode)internal_mode);
}

// --- HA Interface Helpers ---

inline void set_sync_interval_slider(float value) {
  ventilation_ctrl->set_sync_interval((uint32_t)value * 60 * 1000);
}

inline void set_sync_interval_handler(float value) {
  set_sync_interval_slider(value);
}

inline void set_ventilation_timer(float value) {
  auto *v = ventilation_ctrl;
  v->state_machine.ventilation_duration_ms = (uint32_t)value * 60 * 1000;
  v->trigger_sync();
}

inline void set_fan_intensity_slider(float value) {
  int val = (int)value;
  fan_intensity_level->value() = val;
  ventilation_ctrl->set_fan_intensity(val);
  fan_speed_update->execute();
  ui_active->value() = true;
  update_leds->execute();
  ui_timeout_script->execute();
}

inline void set_operating_mode_select(const std::string &x) {
  int mode_index = 0;
  if (x == "Automatik")
    mode_index = 0;
  else if (x == "Wärmerückgewinnung")
    mode_index = 1;
  else if (x == "Durchlüften")
    mode_index = 2;
  else if (x == "Stoßlüftung")
    mode_index = 3;
  else if (x == "Aus")
    mode_index = 4;
  current_mode_index->value() = mode_index;
  cycle_operating_mode(mode_index);
  ui_active->value() = true;
  update_leds->execute();
  ui_timeout_script->execute();
}

// --- Analytics & Maintenance ---

inline void update_filter_analytics() {
  if (system_on->value() && ventilation_enabled->value()) {
    filter_operating_hours->value() += (1.0f / 60.0f);
  }

  auto now = sntp_time->now();
  if (filter_last_change_ts->value() == 0 && now.is_valid()) {
    filter_last_change_ts->value() = (int)now.timestamp;
    ESP_LOGI("filter", "Initial filter timestamp set: %d",
             filter_last_change_ts->value());
  }
}

// --- Event Handlers ---

inline void handle_button_mode_click() {
  current_mode_index->value() = (current_mode_index->value() + 1) % 5;
  cycle_operating_mode(current_mode_index->value());
  ui_active->value() = true;
  update_leds->execute();
  ui_timeout_script->execute();
  ESP_LOGI("button", "Mode button pressed, new index: %d",
           current_mode_index->value());
}

inline void handle_button_power_short_click() {
  if (!ventilation_enabled->value()) {
    ventilation_enabled->value() = true;
    cycle_operating_mode(current_mode_index->value());
    fan_speed_update->execute();
    ui_timeout_script->execute();
  } else {
    cycle_operating_mode(4); // Aus
    ui_timeout_script->execute();
  }
}

inline void handle_button_power_long_click() {
  if (ventilation_enabled->value()) {
    system_on->value() = false;
    ventilation_enabled->value() = false;
    auto *v = ventilation_ctrl;
    v->set_mode(esphome::MODE_OFF);
    lueftung_fan->turn_off();
    fan_pwm_primary->set_level(0.5f);
    if (system_sleep != nullptr)
      system_sleep->execute();
    ui_timeout_script->execute();
  }
}

inline void handle_button_level_click() {
  if (!ventilation_enabled->value())
    return;
  int level = fan_intensity_level->value();
  level = VentilationLogic::get_next_fan_level(level);
  fan_intensity_level->value() = level;
  if (fan_intensity_display != nullptr)
    fan_intensity_display->publish_state(level);
  ventilation_ctrl->set_fan_intensity(level);
  fan_speed_update->execute();
  ui_active->value() = true;
  update_leds->execute();
  ui_timeout_script->execute();
}

inline void handle_intensity_bounce() {
  if (!ventilation_enabled->value())
    return;
  int current_level = fan_intensity_level->value();
  bool up = intensity_bounce_up->value();
  if (up) {
    if (current_level < 10)
      current_level++;
    else {
      current_level = 9;
      intensity_bounce_up->value() = false;
    }
  } else {
    if (current_level > 1)
      current_level--;
    else {
      current_level = 2;
      intensity_bounce_up->value() = true;
    }
  }
  fan_intensity_level->value() = current_level;
  if (fan_intensity_display != nullptr)
    fan_intensity_display->publish_state(current_level);
  ventilation_ctrl->set_fan_intensity(current_level);
  fan_speed_update->execute();
  ui_active->value() = true;
  update_leds->execute();
  ui_timeout_script->execute();
}

inline void handle_espnow_receive(std::vector<uint8_t> data) {
  if (data.size() < sizeof(esphome::VentilationPacket))
    return;
  std::string payload(data.begin(), data.end());
  uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (handle_discovery_payload(payload, broadcast_mac))
    return;
  if (ventilation_ctrl != nullptr) {
    if (data.size() < sizeof(esphome::VentilationPacket))
      return;
    if (ventilation_ctrl->on_packet_received(data))
      update_leds->execute();
  }
}

inline void handle_co2_change(float value) {
  if (fan_intensity_display != nullptr)
    fan_intensity_display->publish_state(value);
  update_leds->execute();
}

inline void handle_humidity_change(float value) { update_leds->execute(); }

inline void handle_presence_change(bool state) {
  if (state) {
    ui_active->value() = true;
    ui_timeout_script->execute();
    update_leds->execute();
  }
}

inline std::vector<uint8_t>
build_and_populate_packet(esphome::MessageType type) {
  auto *v = ventilation_ctrl;
  std::vector<uint8_t> data = v->build_packet(type);
  esphome::VentilationPacket *pkt = (esphome::VentilationPacket *)data.data();
  pkt->co2_auto_enabled = co2_auto_enabled->value();
  pkt->automatik_min_fan_level = (uint8_t)automatik_min_fan_level->value();
  pkt->automatik_max_fan_level = (uint8_t)automatik_max_fan_level->value();
  pkt->auto_co2_threshold_val = (uint16_t)auto_co2_threshold_val->value();
  pkt->auto_humidity_threshold_val =
      (uint8_t)auto_humidity_threshold_val->value();
  pkt->auto_presence_val = (int8_t)auto_presence_val->value();
  pkt->sync_interval_min = (uint16_t)(v->sync_interval_ms / 60 / 1000);
  pkt->vent_timer_min =
      (uint16_t)(v->state_machine.ventilation_duration_ms / 60 / 1000);
  return data;
}

// --- Boot-time Initialization Helpers ----------------------------------

inline void sync_config_to_controller() {
  auto *v = ventilation_ctrl;
  if (v == nullptr)
    return;
  v->set_floor_id((uint8_t)config_floor_id->state);
  v->set_room_id((uint8_t)config_room_id->state);
  v->set_device_id((uint8_t)config_device_id->state);
  bool is_phase_a =
      (config_phase->current_option() == "Phase A (Startet mit Zuluft)");
  v->set_is_phase_a(is_phase_a);
  ESP_LOGI(
      "boot",
      "Synced Config to Controller: Floor %d, Room %d, Device %d, Phase: %s",
      v->floor_id, v->room_id, v->device_id, is_phase_a ? "A" : "B");
}

inline void run_led_self_test() {
  if (status_led_mode_wrg == nullptr || status_led_mode_vent == nullptr ||
      status_led_power == nullptr || status_led_master == nullptr)
    return;
  auto call_wrg = status_led_mode_wrg->turn_on();
  call_wrg.set_effect("None");
  call_wrg.perform();
  status_led_mode_vent->turn_on().perform();
  status_led_power->turn_on().perform();
  status_led_master->turn_on().perform();
  if (status_led_l1)
    status_led_l1->turn_on().perform();
  if (status_led_l2)
    status_led_l2->turn_on().perform();
  if (status_led_l3)
    status_led_l3->turn_on().perform();
  if (status_led_l4)
    status_led_l4->turn_on().perform();
  if (status_led_l5)
    status_led_l5->turn_on().perform();
  ESP_LOGI("boot", "LED self-test: all LEDs on");
}
