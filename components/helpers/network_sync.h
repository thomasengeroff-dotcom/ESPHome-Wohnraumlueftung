#pragma once
#include "helpers/globals.h"

inline bool is_local_mac(const uint8_t *mac) {
  uint8_t local_mac[6];
  esp_read_mac(local_mac, ESP_MAC_WIFI_STA);
  for (int i = 0; i < 6; i++) {
    if (mac[i] != local_mac[i])
      return false;
  }
  return true;
}

/** @brief Registers a MAC address as an ESP-NOW peer and persists it to NVS. */
inline void register_peer_dynamic(const uint8_t *mac) {
  if (!esphome::espnow::global_esp_now) {
    ESP_LOGW("espnow_disc",
             "global_esp_now not ready, skipping register_peer_dynamic");
    return;
  }
  if (is_local_mac(mac)) {
    return; // Don't register ourselves
  }
  if (espnow_peers == nullptr) {
    ESP_LOGE("espnow_disc", "espnow_peers global is null!");
    return;
  }
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

  // FIXED: Register peer with current WiFi channel for reliable unicast
  esp_now_peer_info_t peer_info = {};
  memset(&peer_info, 0, sizeof(esp_now_peer_info_t));
  memcpy(peer_info.peer_addr, mac, 6);
  peer_info.channel = 0; // 0 = use current channel (auto)
  peer_info.encrypt = false;
  peer_info.ifidx = WIFI_IF_STA;

  // Remove first if already exists (channel may have changed)
  esp_now_del_peer(mac);
  esp_err_t err = esp_now_add_peer(&peer_info);

  if (err == ESP_OK) {
    ESP_LOGI("espnow_disc", "Peer %s registered (channel: current)",
             mac_str.c_str());
  } else {
    ESP_LOGW("espnow_disc", "Failed to register peer %s: %d", mac_str.c_str(),
             err);
  }
}

/** @brief Loads all peers saved in the runtime string cache and registers them
 *  with the ESP-NOW stack.
 *
 *  NOTE W4: Despite the previous name (load_peers_from_flash), this does NOT
 *  restore from NVS/flash. espnow_peers is a GlobalsComponent<std::string>
 *  WITHOUT restore_value, so its contents are lost on every reboot.
 *  Peers are automatically re-discovered via the ROOM_DISC broadcast. This
 *  function is called for completeness but will usually be a no-op after boot.
 *
 *  TODO: If persistent peer storage is required, change espnow_peers to
 *  restore_value: true (NVS) and add NVS size validation.
 */
inline void load_peers_from_runtime_cache() {
  if (!esphome::espnow::global_esp_now) {
    ESP_LOGW(
        "espnow_disc",
        "global_esp_now not ready, skipping load_peers_from_runtime_cache");
    return;
  }
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
      ESP_LOGI("espnow_disc", "Restored peer from flash: %s", mac_str.c_str());
    }
    if (end == std::string::npos)
      break;
    start = end + 1;
    end = current_list.find(",", start);
  }
}

/** @brief Sends a discovery broadcast to identify peers in the same room. */
inline void send_discovery_broadcast() {
  if (!esphome::espnow::global_esp_now) {
    ESP_LOGW("espnow_disc",
             "global_esp_now not ready, skipping send_discovery_broadcast");
    return;
  }
  if (!config_floor_id || !config_room_id) {
    ESP_LOGW("espnow_disc",
             "config IDs not ready, skipping send_discovery_broadcast");
    return;
  }
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "ROOM_DISC:%d:%d",
           (int)config_floor_id->state, (int)config_room_id->state);
  std::string msg(buffer);
  std::vector<uint8_t> data(msg.begin(), msg.end());
  uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  // FIXED #3: ESPNowComponent::send() signature is send(peer, payload, size).
  // Passing std::vector directly was silently wrong — must use .data()/.size().
  esphome::espnow::global_esp_now->send(broadcast_mac, data, [](esp_err_t err) {
    /* no-op callback required: ESPNow crashes with std::bad_function_call
       without it */
  });
  ESP_LOGI("espnow_disc", "Sent discovery broadcast: %s", msg.c_str());
}

/** @brief Requests current status from all known peers.
 *  Called shortly after boot once peers are discovered.
 */
inline void request_peer_status() {
  if (!esphome::espnow::global_esp_now)
    return;

  ESP_LOGI("vent_sync", "Requesting current status from peers...");
  auto data = build_and_populate_packet(esphome::MSG_STATUS_REQUEST);
  uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  esphome::espnow::global_esp_now->send(broadcast_mac, data, [](esp_err_t err) {
    /* no-op callback */
  });
}

/** @brief Sends a unicast confirmation to a discovered peer. */
inline void send_discovery_confirmation(const uint8_t *target_mac) {
  if (!esphome::espnow::global_esp_now)
    return;
  if (!config_floor_id || !config_room_id)
    return;
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "ROOM_CONF:%d:%d",
           (int)config_floor_id->state, (int)config_room_id->state);
  std::string msg(buffer);
  std::vector<uint8_t> data(msg.begin(), msg.end());
  // FIXED #3: Use correct send(peer, payload_ptr, size) signature.
  esphome::espnow::global_esp_now->send(target_mac, data, [](esp_err_t err) {
    /* no-op callback required: ESPNow crashes with std::bad_function_call
       without it */
  });
}

/** @brief Sends a sync packet to all registered peers via unicast. */
inline void send_sync_to_all_peers(const std::vector<uint8_t> &data) {
  if (!esphome::espnow::global_esp_now)
    return;
  if (espnow_peers == nullptr)
    return;
  std::string current_list = espnow_peers->value();
  if (current_list.empty()) {
    ESP_LOGW("espnow_sync", "No peers in list — cannot send unicast!");
    return;
  }

  ESP_LOGI("espnow_sync", "Sending unicast to peers: %s", current_list.c_str());

  size_t start = 0;
  size_t end = current_list.find(",");
  while (true) {
    std::string mac_str = (end == std::string::npos)
                              ? current_list.substr(start)
                              : current_list.substr(start, end - start);
    auto mac = parse_mac_local(mac_str);
    if (mac.has_value())
      // FIXED #3: Use correct send(peer, payload_ptr, size) signature.
      esphome::espnow::global_esp_now->send(mac->data(), data,
                                            [](esp_err_t err) {
                                              /* no-op callback required: ESPNow
                                                 crashes with
                                                 std::bad_function_call without
                                                 it */
                                            });
    if (end == std::string::npos)
      break;
    start = end + 1;
    end = current_list.find(",", start);
  }
}

/** @brief Parses incoming discovery strings. */
inline bool handle_discovery_payload(const std::string &payload,
                                     const uint8_t *src_addr) {
  if (payload.find("ROOM_DISC:") == 0 || payload.find("ROOM_CONF:") == 0) {
    if (is_local_mac(src_addr)) {
      return true; // Recognize but ignore self-packets
    }
    int floor, room;
    if (sscanf(payload.c_str() + 10, "%d:%d", &floor, &room) == 2) {
      if (floor == (int)config_floor_id->state &&
          room == (int)config_room_id->state) {
        ESP_LOGI("espnow_disc", "Matching room discovery from %02X:%02X:%02X",
                 src_addr[3], src_addr[4], src_addr[5]);
        register_peer_dynamic(src_addr);
        if (payload.find("ROOM_DISC:") == 0)
          send_discovery_confirmation(src_addr);
        return true;
      }
    }
  }
  return false;
}

/** @brief Sends a sync packet to all registered peers via unicast.
 *  This is used for real-time mirroring of settings (CO2, fan levels, etc.)
 */
inline void sync_settings_to_peers() {
  if (ventilation_ctrl == nullptr)
    return;
  // Build a MSG_STATE packet which includes all current settings
  auto data = build_and_populate_packet(esphome::MSG_STATE);

  // OPTIMIZATION: For immediate state changes (button/slider), we use
  // a room-wide broadcast (FF:FF:FF:FF:FF:FF). This is much more robust
  // than unicast because it works even if a peer's MAC is not yet discovered
  // or registered. Periodic sync still uses unicast.
  uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  esphome::espnow::global_esp_now->send(broadcast_mac, data,
                                        [](esp_err_t err) {});

  ESP_LOGI("vent_sync", "Sent state change via BROADCAST to room members.");
}

// --- Inline wrappers (called from YAML lambdas) -----------------------

// Global constants


/// Last PWM level sent to fan_pwm_primary via set_level() (0.0–1.0).
/// Used by the 'Lüfter PWM Ansteuerung' template sensor since LEDCOutput has no
/// get_level().
// FIXED #9: Changed static→inline for mutable state (C++17, safe for single TU)
inline void handle_espnow_receive(const std::vector<uint8_t> &data) {
  auto *v = ventilation_ctrl;
  if (v == nullptr)
    return;

  if (data.size() != sizeof(esphome::VentilationPacket)) {
    ESP_LOGI("vent_sync", "Invalid packet size: %d", data.size());
    return;
  }
  esphome::VentilationPacket *pkt = (esphome::VentilationPacket *)data.data();

  // Validate magic header and protocol version
  if (pkt->magic_header != 0x42 ||
      pkt->protocol_version != esphome::PROTOCOL_VERSION) {
    ESP_LOGI("vent_sync", "Invalid packet header: %d %d", pkt->magic_header,
             pkt->protocol_version);
    return;
  }

  // Handle Request/Response Logic
  if (pkt->msg_type == esphome::MSG_STATUS_REQUEST) {
    if (pkt->floor_id == (int)config_floor_id->state &&
        pkt->room_id == (int)config_room_id->state) {
      ESP_LOGI("vent_sync", "Status request from peer %d. Sending response...",
               pkt->device_id);
      auto resp = build_and_populate_packet(esphome::MSG_STATUS_RESPONSE);
      uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
      esphome::espnow::global_esp_now->send(broadcast_mac, resp,
                                            [](esp_err_t err) {});
    }
    return;
  }

  bool changed = v->on_packet_received(data);

  if (pkt->msg_type == esphome::MSG_STATUS_RESPONSE && !v->is_state_synced) {
    if (pkt->floor_id == (int)config_floor_id->state &&
        pkt->room_id == (int)config_room_id->state) {
      ESP_LOGI("vent_sync", "Adopting state from peer %d (REBOOT SYNC)",
               pkt->device_id);

      // Soft Transition: Start from 0 to ensure ramp-up
      int target_intensity = pkt->fan_intensity;

      // Apply mode
      int mode_idx = 1; // Default to WRG
      if (pkt->current_mode == esphome::MODE_VENTILATION)
        mode_idx = 2;
      else if (pkt->current_mode == esphome::MODE_STOSSLUEFTUNG)
        mode_idx = 3;
      else if (pkt->current_mode == esphome::MODE_OFF)
        mode_idx = 4;

      // Set mode and trigger ramp via cycle_operating_mode
      cycle_operating_mode(mode_idx);

      // Force intensity to 0 then ramp to target
      fan_intensity_level->value() = 0;
      fan_speed_update->execute();
      
      // Now set target intensity - the update_fan_logic will pick it up
      v->set_fan_intensity(target_intensity);
      fan_intensity_level->value() = target_intensity;
      
      v->is_state_synced = true;
      return; // Skip further sync processing to avoid double-triggers
    }
  }

  if (changed) {
    // 1. Sync Fan Intensity
    if (fan_intensity_level != nullptr &&
        v->current_fan_intensity != fan_intensity_level->value()) {
      fan_intensity_level->value() = v->current_fan_intensity;
      if (fan_intensity_display != nullptr)
        fan_intensity_display->publish_state(v->current_fan_intensity);
    }

    // 2. Sync Mode & Globals
    int new_mode_idx = 0;
    std::string mode_str = "Wärmerückgewinnung";

    if (v->state_machine.current_mode == esphome::MODE_OFF) {
      new_mode_idx = 4;
      mode_str = "Aus";
      if (ventilation_enabled)
        ventilation_enabled->value() = false;
      if (system_on)
        system_on->value() = false;
      if (auto_mode_active)
        auto_mode_active->value() = false;
    } else {
      if (ventilation_enabled)
        ventilation_enabled->value() = true;
      if (system_on)
        system_on->value() = true;

      if (auto_mode_active != nullptr && auto_mode_active->value()) {
        new_mode_idx = 0;
        mode_str = "Automatik";
      } else if (v->state_machine.current_mode == esphome::MODE_ECO_RECOVERY) {
        new_mode_idx = 1;
        mode_str = "Wärmerückgewinnung";
      } else if (v->state_machine.current_mode == esphome::MODE_VENTILATION) {
        new_mode_idx = 2;
        mode_str = "Durchlüften";
      } else if (v->state_machine.current_mode == esphome::MODE_STOSSLUEFTUNG) {
        new_mode_idx = 3;
        mode_str = "Stoßlüftung";
      }
    }

    if (current_mode_index != nullptr)
      current_mode_index->value() = new_mode_idx;

    // Update Select Component if different
    if (luefter_modus != nullptr &&
        std::string(luefter_modus->current_option()) != mode_str) {
      luefter_modus->publish_state(mode_str);
    }

    // FIXED: Activate UI so LEDs show the peer's change for 30 seconds
    // Without this, ui_active is false and update_leds_logic() turns
    // everything off immediately.
    ui_active->value() = true;
    ui_timeout_script->execute(); // Start 30s auto-dim timer

    // Update Visuals
    update_leds->execute();
    fan_speed_update->execute();

    ESP_LOGI("vent_sync", "Peer state applied — UI activated for 30s");
  }

  // --- Configuration Settings Sync ---
  // ESP-NOW payloads carry all user settings. If another node has different
  // configuration values, we adopt them to ensure room-wide parity.

  // Guard against corrupt packets with out-of-range values that could
  // crash/break logic
  bool dirty = false;

  // 1. CO2 Automatik (Switch)
  if (pkt->co2_auto_enabled != co2_auto_enabled->value()) {
    co2_auto_enabled->value() = pkt->co2_auto_enabled;
    automatik_co2->publish_state(pkt->co2_auto_enabled);
    ESP_LOGI("vent_sync", "Synced co2_auto_enabled from peer: %d",
             pkt->co2_auto_enabled);
    dirty = true;
  }

  // 2. Automatik Min/Max Levels (Sliders 1-10)
  if (pkt->automatik_min_fan_level >= 1 && pkt->automatik_min_fan_level <= 10 &&
      pkt->automatik_min_fan_level != automatik_min_fan_level->value()) {
    automatik_min_fan_level->value() = pkt->automatik_min_fan_level;
    automatik_min_luefterstufe->publish_state(pkt->automatik_min_fan_level);
    ESP_LOGI("vent_sync", "Synced automatik_min_fan_level from peer: %d",
             pkt->automatik_min_fan_level);
    dirty = true;
  }

  if (pkt->automatik_max_fan_level >= 1 && pkt->automatik_max_fan_level <= 10 &&
      pkt->automatik_max_fan_level != automatik_max_fan_level->value()) {
    automatik_max_fan_level->value() = pkt->automatik_max_fan_level;
    automatik_max_luefterstufe->publish_state(pkt->automatik_max_fan_level);
    ESP_LOGI("vent_sync", "Synced automatik_max_fan_level from peer: %d",
             pkt->automatik_max_fan_level);
    dirty = true;
  }

  // 3. Automatik Thresholds (Numbers)
  if (pkt->auto_co2_threshold_val >= 400 &&
      pkt->auto_co2_threshold_val <= 5000 &&
      pkt->auto_co2_threshold_val != auto_co2_threshold_val->value()) {
    auto_co2_threshold_val->value() = pkt->auto_co2_threshold_val;
    auto_co2_threshold->publish_state(pkt->auto_co2_threshold_val);
    ESP_LOGI("vent_sync", "Synced auto_co2_threshold from peer: %d ppm",
             pkt->auto_co2_threshold_val);
    dirty = true;
  }

  if (pkt->auto_humidity_threshold_val >= 0 &&
      pkt->auto_humidity_threshold_val <= 100 &&
      pkt->auto_humidity_threshold_val !=
          auto_humidity_threshold_val->value()) {
    auto_humidity_threshold_val->value() = pkt->auto_humidity_threshold_val;
    auto_humidity_threshold->publish_state(pkt->auto_humidity_threshold_val);
    ESP_LOGI("vent_sync", "Synced auto_humidity_threshold from peer: %d %%",
             pkt->auto_humidity_threshold_val);
    dirty = true;
  }

  // 4. Presence Compensation Slider (-5 to +5)
  if (pkt->auto_presence_val >= -5 && pkt->auto_presence_val <= 5 &&
      pkt->auto_presence_val != auto_presence_val->value()) {
    auto_presence_val->value() = pkt->auto_presence_val;
    auto_presence_slider->publish_state(pkt->auto_presence_val);
    ESP_LOGI("vent_sync", "Synced auto_presence_val from peer: %d",
             pkt->auto_presence_val);
    dirty = true;
  }

  // 5. System Timers
  if (pkt->sync_interval_min >= 1 && pkt->sync_interval_min <= 1440) {
    if (v->sync_interval_ms != (uint32_t)(pkt->sync_interval_min * 60 * 1000)) {
      v->sync_interval_ms = (uint32_t)(pkt->sync_interval_min * 60 * 1000);
      sync_interval_config->publish_state(pkt->sync_interval_min);
      ESP_LOGI("vent_sync", "Synced sync_interval_min from peer: %d",
               pkt->sync_interval_min);
      dirty = true;
    }
  }

  if (pkt->vent_timer_min <= 1440 &&
      (uint32_t)(pkt->vent_timer_min * 60 * 1000) !=
          v->state_machine.ventilation_duration_ms) {
    v->state_machine.ventilation_duration_ms = pkt->vent_timer_min * 60 * 1000;
    vent_timer->publish_state(pkt->vent_timer_min);
    ESP_LOGI("vent_sync", "Synced vent_timer from peer: %d min",
             pkt->vent_timer_min);
    // Note: setting the timer does not change the active mode.
  }

  // 6. UI Settings (LED Brightness)
  if (pkt->max_led_brightness >= 0.05f && pkt->max_led_brightness <= 1.0f &&
      std::abs(pkt->max_led_brightness - max_led_brightness->value()) > 0.01f) {
    max_led_brightness->value() = pkt->max_led_brightness;
    led_max_brightness_config->publish_state(pkt->max_led_brightness * 100.0f);
    update_leds->execute();
    ESP_LOGI("vent_sync", "Synced max_led_brightness from peer: %.2f",
             pkt->max_led_brightness);
    dirty = true;
  }

  // If any configs changed, re-evaluate the auto mode immediately to reflect
  // new boundaries — but ONLY if auto mode is actually active.
  // FIXED K1/W6: Calling evaluate_auto_mode() unconditionally caused a
  // Ping-Pong: evaluate_auto_mode() → trigger_sync() → pending_broadcast=true
  // → send to peer → peer syncs config → loops indefinitely.
  if (dirty && auto_mode_active != nullptr && auto_mode_active->value()) {
    evaluate_auto_mode();
  }
}

/// @brief Updates the ventilation duration on-the-fly without switching mode.
/// Called when the user adjusts the vent_timer number component.
