import os
import re

input_file = "components/automation_helpers.h"
output_dir = "components/helpers"
os.makedirs(output_dir, exist_ok=True)

with open(input_file, 'r') as f:
    lines = f.readlines()

def get_file(line, current):
    m = re.match(r'^(?:inline|static|extern)\s+[\w\:\<\>]+(?:\s+[\*\&]?\s*|\s+)(\w+)\s*\(', line)
    if m:
        func = m.group(1)
        if func in ['filter_ntc_stable', 'get_co2_classification', 'get_co2_fan_level', 'calculate_heat_recovery_efficiency']: return 'climate.h'
        if func in ['evaluate_auto_mode']: return 'auto_mode.h'
        if func in ['notify_fan_direction_changed', 'set_fan_logic', 'level_to_speed', 'get_current_target_speed', 'calculate_virtual_fan_rpm', 'update_fan_logic', 'is_fan_slider_off', 'calculate_ramp_up', 'calculate_ramp_down']: return 'fan_control.h'
        if func in ['cycle_operating_mode', 'update_filter_analytics', 'sync_config_to_controller', 'run_led_self_test']: return 'system_lifecycle.h'
        if func in ['update_leds_logic', 'check_master_led_error']: return 'led_feedback.h'
        if func in ['is_local_mac', 'register_peer_dynamic', 'load_peers_from_runtime_cache', 'send_discovery_broadcast', 'request_peer_status', 'send_discovery_confirmation', 'send_sync_to_all_peers', 'handle_discovery_payload', 'sync_settings_to_peers', 'handle_espnow_receive', 'build_and_populate_packet']: return 'network_sync.h'
        if func in ['set_ventilation_timer', 'set_sync_interval_handler', 'set_fan_intensity_slider', 'set_operating_mode_select', 'handle_button_mode_click', 'handle_button_power_short_click', 'handle_button_power_long_click', 'handle_button_level_click', 'handle_intensity_bounce']: return 'user_input.h'
    return current

outputs = {
    'globals.h': [], 'climate.h': [], 'auto_mode.h': [], 'fan_control.h': [],
    'system_lifecycle.h': [], 'led_feedback.h': [], 'network_sync.h': [], 'user_input.h': []
}

current_file = 'globals.h'
brace_level = 0

for line in lines:
    if brace_level == 0:
        new_file = get_file(line, current_file)
        if new_file != current_file and not line.startswith('}'):
            current_file = new_file
            
        if 'inline float last_fan_pwm_level' in line: current_file = 'globals.h'
        if 'inline std::deque<float> ntc_history' in line: current_file = 'globals.h'
        if 'inline uint32_t last_direction_change_time' in line: current_file = 'globals.h'

    outputs[current_file].append(line)
    brace_level += line.count('{') - line.count('}')

print(f"Split completed. File sizes:")
for f, content in outputs.items():
    with open(os.path.join(output_dir, f), 'w') as out:
        if f != 'globals.h':
            out.write('#pragma once\n#include "globals.h"\n\n')
        out.writelines(content)
    print(f" - {f}: {len(content)} lines")

with open(input_file, 'w') as f:
    f.write('''#pragma once
// ==========================================================================
// WRG Wohnraumlüftung – ESPHome Custom Component
// ==========================================================================

#include "helpers/globals.h"
#include "helpers/network_sync.h"
#include "helpers/climate.h"
#include "helpers/auto_mode.h"
#include "helpers/fan_control.h"
#include "helpers/system_lifecycle.h"
#include "helpers/led_feedback.h"
#include "helpers/user_input.h"
''')
