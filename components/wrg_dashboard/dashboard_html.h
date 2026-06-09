// ==========================================================================
// VentoSync HRV – ESPHome Custom Component
// https://github.com/thomasengeroff-dotcom/VentoSync
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
// File:        dashboard_html.h
// Description: HTML/CSS strings for the WRG web dashboard.
// Author:      Thomas Engeroff
// Created:     2026-03-09
// Modified:    2026-06-09
// ==========================================================================
#pragma once

const char DASHBOARD_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="de" class="dark">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Wohnraumlüftung Dashboard</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
  <script src="https://cdn.tailwindcss.com"></script>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    tailwind.config = {
      darkMode: 'class',
      theme: {
        extend: {
          fontFamily: {
            sans: ['Inter', 'system-ui', '-apple-system', 'sans-serif'],
          },
          colors: {
            bg: '#0f172a',
            card: '#1e293b',
            cardHover: '#253347',
            surface: '#334155',
            accent: '#22d3ee',
            accentHover: '#06b6d4',
            accentDim: 'rgba(34, 211, 238, 0.12)',
            danger: '#f87171',
            dangerDim: 'rgba(248, 113, 113, 0.12)',
            warning: '#fbbf24',
            warningDim: 'rgba(251, 191, 36, 0.12)',
            success: '#34d399',
            successDim: 'rgba(52, 211, 153, 0.12)',
            muted: '#64748b',
            subtle: '#475569',
          }
        }
      }
    }
  </script>
  <style>
    body { font-family: 'Inter', system-ui, -apple-system, sans-serif; }

    /* Custom scrollbar */
    ::-webkit-scrollbar { width: 6px; }
    ::-webkit-scrollbar-track { background: #0f172a; }
    ::-webkit-scrollbar-thumb { background: #334155; border-radius: 3px; }
    ::-webkit-scrollbar-thumb:hover { background: #475569; }

    /* Slider styling */
    input[type=range] {
      -webkit-appearance: none;
      appearance: none;
      height: 6px;
      background: linear-gradient(90deg, #334155 0%, #334155 100%);
      border-radius: 3px;
      outline: none;
    }
    input[type=range]::-webkit-slider-thumb {
      -webkit-appearance: none;
      height: 20px;
      width: 20px;
      border-radius: 50%;
      background: #22d3ee;
      cursor: pointer;
      box-shadow: 0 0 8px rgba(34, 211, 238, 0.4);
      transition: box-shadow 0.2s ease;
    }
    input[type=range]::-webkit-slider-thumb:hover {
      box-shadow: 0 0 14px rgba(34, 211, 238, 0.6);
    }
    input[type=range]::-moz-range-thumb {
      height: 20px;
      width: 20px;
      border-radius: 50%;
      background: #22d3ee;
      cursor: pointer;
      border: none;
      box-shadow: 0 0 8px rgba(34, 211, 238, 0.4);
    }
    input[type=range]::-moz-range-track {
      height: 6px;
      background: #334155;
      border-radius: 3px;
    }

    /* Number input spinner styling */
    input[type=number]::-webkit-inner-spin-button,
    input[type=number]::-webkit-outer-spin-button {
      opacity: 1;
      height: 28px;
    }

    /* Card hover effect */
    .dash-card {
      transition: border-color 0.25s ease, box-shadow 0.25s ease;
    }
    .dash-card:hover {
      border-color: #334155;
      box-shadow: 0 4px 24px rgba(0, 0, 0, 0.25);
    }

    /* Sensor row hover */
    .sensor-row {
      padding: 6px 8px;
      border-radius: 6px;
      transition: background-color 0.15s ease;
    }
    .sensor-row:hover {
      background-color: rgba(51, 65, 85, 0.3);
    }

    /* Pulsing indicator */
    @keyframes pulse-dot {
      0%, 100% { opacity: 1; box-shadow: 0 0 0 0 rgba(34, 211, 238, 0.5); }
      50% { opacity: 0.85; box-shadow: 0 0 0 4px rgba(34, 211, 238, 0); }
    }
    .pulse-online {
      animation: pulse-dot 2s ease-in-out infinite;
    }

    /* Focus ring for inputs */
    input:focus, select:focus {
      outline: none;
      border-color: #22d3ee !important;
      box-shadow: 0 0 0 2px rgba(34, 211, 238, 0.2) !important;
    }

    /* Value emphasis */
    .val-text {
      color: #f1f5f9;
      font-weight: 600;
      font-variant-numeric: tabular-nums;
    }
    .val-unit {
      color: #64748b;
      font-weight: 400;
      font-size: 0.75em;
      margin-left: 2px;
    }

    /* Section header */
    .section-header {
      display: flex;
      align-items: center;
      gap: 10px;
      padding-bottom: 10px;
      margin-bottom: 12px;
      border-bottom: 1px solid #1e293b;
    }
    .section-header h2 {
      font-size: 1.05rem;
      font-weight: 600;
      color: #f1f5f9;
      letter-spacing: -0.01em;
    }
    .section-header .icon {
      width: 28px;
      height: 28px;
      border-radius: 8px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-size: 14px;
      flex-shrink: 0;
    }

    /* Badge pill */
    .badge {
      display: inline-flex;
      align-items: center;
      padding: 2px 10px;
      border-radius: 9999px;
      font-size: 0.7rem;
      font-weight: 700;
      letter-spacing: 0.04em;
      text-transform: uppercase;
    }

    /* Setting group */
    .setting-group-title {
      font-size: 0.7rem;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      color: #64748b;
      margin-bottom: 10px;
      padding-bottom: 6px;
      border-bottom: 1px solid rgba(51, 65, 85, 0.5);
    }
  </style>
</head>
<body class="bg-bg text-gray-300 min-h-screen p-4 sm:p-6 lg:p-8 flex flex-col items-center">
  <div class="w-full max-w-5xl space-y-5">
    
    <header class="text-center mb-6">
      <h1 class="text-2xl sm:text-3xl font-bold tracking-tight text-white">
        WRG Lüftung Dashboard
      </h1>
      <span class="text-xs font-medium text-muted mt-1 inline-block">(v2.0)</span>
    </header>
    
    <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
      
      <!-- General Settings / Device Info -->
      <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col">
        <div class="section-header">
          <div class="icon bg-blue-500/15 text-blue-400">⚙</div>
          <h2>Grundeinstellungen</h2>
        </div>
        <div class="space-y-1 flex-1">
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Geräte-ID:</span> <span class="val-text text-lg" id="val_device_id">--</span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Aktuelle Floor ID:</span> <span class="val-text text-lg" id="val_floor_id">--</span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Raum (Room ID):</span> <span class="val-text text-lg" id="val_room_id">--</span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Geräte-Phase (A/B):</span> <span class="val-text text-lg" id="val_phase">--</span></div>
        </div>
      </div>

      <!-- Status & Sensors -->
      <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col">
        <div class="section-header">
          <div class="icon bg-cyan-500/15 text-cyan-400">📡</div>
          <h2>Aktuelle Sensordaten</h2>
        </div>
        <div class="space-y-1 flex-1">
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Board-Temperatur:</span> <span class="val-text text-lg" id="val_temperature">--<span class="val-unit">°C</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Luftdruck:</span> <span class="val-text text-lg" id="val_pressure">--<span class="val-unit">hPa</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Feuchtigkeit (Außen):</span> <span class="val-text text-lg" id="val_outdoor_humidity">--<span class="val-unit">%</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Temperatur Zuluft:</span> <span class="val-text text-lg" id="val_temp_zuluft">--<span class="val-unit">°C</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Temperatur Abluft:</span> <span class="val-text text-lg" id="val_temp_abluft">--<span class="val-unit">°C</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Effizienz WRG:</span> <span class="val-text text-accent text-lg" id="val_heat_recovery_efficiency">--<span class="val-unit">%</span></span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Lüfter RPM:</span> <span class="val-text text-lg" id="val_fan_rpm">--</span></div>
          <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Luftrichtung:</span> <span class="val-text text-lg" id="val_direction_display">--</span></div>
        </div>
      </div>

      <!-- Air Quality & Maintenance combined column -->
      <div class="flex flex-col gap-4">
        <!-- Air Quality -->
        <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col flex-1">
          <div class="section-header">
            <div class="icon bg-emerald-500/15 text-emerald-400">🌿</div>
            <h2>Luftqualität</h2>
          </div>
          <div class="space-y-1 flex-1">
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">CO2:</span> <span class="val-text text-lg" id="val_room_co2">--<span class="val-unit">ppm</span></span></div>
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Bewertung:</span> <span class="val-text text-lg" id="val_room_co2_bewertung">--</span></div>
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Temperatur:</span> <span class="val-text text-lg" id="val_room_temperature">--<span class="val-unit">°C</span></span></div>
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Luftfeuchtigkeit:</span> <span class="val-text text-lg" id="val_room_humidity">--<span class="val-unit">%</span></span></div>
          </div>
        </div>

        <!-- Maintenance -->
        <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col">
          <div class="section-header">
            <div class="icon bg-amber-500/15 text-amber-400">🔧</div>
            <h2>Wartung</h2>
          </div>
          <div class="space-y-1">
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Filter Betriebstage:</span> <span class="val-text text-lg" id="val_filter_operating_days">--</span></div>
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Filterwechsel Alarm:</span> <span class="val-text text-lg" id="val_filter_change_alarm">--</span></div>
            <div class="sensor-row flex justify-between items-center"><span class="text-muted text-sm">Radar Präsenz:</span> <span class="val-text text-lg" id="val_radar_presence">--</span></div>
          </div>
        </div>
      </div>

      <!-- Controls -->
      <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col space-y-5 md:col-span-2 lg:col-span-3">
        <div class="section-header">
          <div class="icon bg-violet-500/15 text-violet-400">🎛</div>
          <h2>Einstellungen</h2>
        </div>
        
        <div class="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <!-- Left group: Main controls -->
          <div>
            <div class="setting-group-title">Steuerung</div>
            <div class="space-y-4">
              <div class="flex flex-col space-y-2">
                <span class="text-sm text-muted font-medium">Lüfter Modus</span>
                <select id="luefter_modus" onchange="sendSet('luefter_modus', this.value)" class="bg-surface/50 border border-subtle/50 text-white text-sm rounded-lg block w-full p-2.5 transition-all duration-200 cursor-pointer">
                  <option value="Smart-Automatik">Smart-Automatik</option>
                  <option value="Wärmerückgewinnung">Wärmerückgewinnung</option>
                  <option value="Durchlüften">Durchlüften</option>
                  <option value="Stoßlüftung">Stoßlüftung</option>
                  <option value="Aus">Aus</option>
                </select>
              </div>

              <div class="flex flex-col space-y-2">
                <div class="flex justify-between items-center"><span class="text-sm text-muted font-medium">Lüfter Intensität</span><span id="label_fan_intensity" class="font-bold text-accent text-lg tabular-nums">--</span></div>
                <input type="range" id="fan_intensity_display" min="1" max="10" step="1" onchange="sendSet('fan_intensity_display', this.value)" oninput="document.getElementById('label_fan_intensity').innerText = this.value" class="w-full cursor-pointer mt-1">
              </div>

              <div class="grid grid-cols-2 gap-3">
                <div class="flex justify-between items-center bg-surface/30 p-3 rounded-lg border border-subtle/30 transition-colors duration-200 hover:border-subtle/60">
                  <span class="text-sm text-gray-300">Autom. Min Stufe</span>
                  <input type="number" id="automatik_min_luefterstufe" onchange="sendSet('automatik_min_luefterstufe', this.value)" class="bg-surface/60 border border-subtle/50 text-white text-sm rounded-lg w-20 p-1.5 text-center transition-all duration-200">
                </div>
                <div class="flex justify-between items-center bg-surface/30 p-3 rounded-lg border border-subtle/30 transition-colors duration-200 hover:border-subtle/60">
                  <span class="text-sm text-gray-300">Autom. Max Stufe</span>
                  <input type="number" id="automatik_max_luefterstufe" onchange="sendSet('automatik_max_luefterstufe', this.value)" class="bg-surface/60 border border-subtle/50 text-white text-sm rounded-lg w-20 p-1.5 text-center transition-all duration-200">
                </div>
              </div>
            </div>
          </div>

          <!-- Right group: Thresholds -->
          <div>
            <div class="setting-group-title">Schwellwerte & Automatik</div>
            <div class="space-y-3">
              <div class="flex justify-between items-center bg-surface/30 p-3 rounded-lg border border-subtle/30 transition-colors duration-200 hover:border-subtle/60">
                <span class="text-sm text-gray-300">CO2 Schwellwert</span>
                <div class="flex items-center gap-2">
                  <input type="number" id="auto_co2_threshold" onchange="sendSet('auto_co2_threshold', this.value)" class="bg-surface/60 border border-subtle/50 text-white text-sm rounded-lg w-20 p-1.5 text-center transition-all duration-200">
                  <span class="text-xs text-muted">ppm</span>
                </div>
              </div>
              
              <div class="flex justify-between items-center bg-surface/30 p-3 rounded-lg border border-subtle/30 transition-colors duration-200 hover:border-subtle/60">
                <span class="text-sm text-gray-300">Feuchte Schwellwert</span>
                <div class="flex items-center gap-2">
                  <input type="number" id="auto_humidity_threshold" onchange="sendSet('auto_humidity_threshold', this.value)" class="bg-surface/60 border border-subtle/50 text-white text-sm rounded-lg w-20 p-1.5 text-center transition-all duration-200">
                  <span class="text-xs text-muted">%</span>
                </div>
              </div>

              <div class="flex justify-between items-center bg-surface/30 p-3 rounded-lg border border-subtle/30 transition-colors duration-200 hover:border-subtle/60">
                <span class="text-sm text-gray-300">Anwesenheit Anpassung</span>
                <input type="number" id="auto_presence_slider" onchange="sendSet('auto_presence_slider', this.value)" class="bg-surface/60 border border-subtle/50 text-white text-sm rounded-lg w-20 p-1.5 text-center transition-all duration-200">
              </div>
            </div>
          </div>
        </div>
      </div>

      <!-- Verbundene Geräte (ESP-NOW) -->
      <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col space-y-4 md:col-span-2 lg:col-span-3 hidden" id="peers_card">
        <div class="section-header">
          <div class="icon bg-sky-500/15 text-sky-400">📶</div>
          <h2>Verbundene Geräte (ESP-NOW)</h2>
        </div>
        <div id="peers_container" class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4"></div>
      </div>

      <!-- Chart -->
      <div class="dash-card bg-card rounded-xl p-5 shadow-lg border border-gray-800/60 flex flex-col md:col-span-2 lg:col-span-3">
        <div class="section-header">
          <div class="icon bg-pink-500/15 text-pink-400">📈</div>
          <h2>Graphen & Verlauf</h2>
        </div>
        <div class="relative w-full h-[400px]">
          <canvas id="historyChart"></canvas>
        </div>
      </div>
      
    </div>
  </div>

  <script>
    function sanitizeHTML(str) {
        if (typeof str !== 'string') return str;
        return str.replace(/[<>&"]/g, function(c) {
            return {'<': '&lt;', '>': '&gt;', '&': '&amp;', '"': '&quot;'}[c];
        });
    }

    // Chart Setup
    const maxHistoryPoints = 150; // approx 5 minutes at 2s interval
    const chartData = {
      labels: ["", "", "", "", ""],
      datasets: [
        {
          label: 'Lüfter RPM',
          borderColor: '#22d3ee',
          backgroundColor: 'rgba(34, 211, 238, 0.08)',
          data: [null, null, null, null, null],
          yAxisID: 'y',
          tension: 0.4,
          borderWidth: 2,
          pointRadius: 0,
          fill: true
        },
        {
          label: 'Raumtemp °C',
          borderColor: '#f87171',
          backgroundColor: 'rgba(248, 113, 113, 0.06)',
          data: [null, null, null, null, null],
          yAxisID: 'y1',
          tension: 0.4,
          borderWidth: 2,
          pointRadius: 0,
          fill: false
        },
        {
          label: 'CO2 ppm',
          borderColor: '#a78bfa',
          backgroundColor: 'rgba(167, 139, 250, 0.06)',
          data: [null, null, null, null, null],
          yAxisID: 'y2',
          tension: 0.4,
          borderWidth: 2,
          pointRadius: 0,
          fill: false
        },
        {
          label: 'Luftfeuchte %',
          borderColor: '#38bdf8',
          backgroundColor: 'rgba(56, 189, 248, 0.06)',
          data: [null, null, null, null, null],
          yAxisID: 'y1',
          tension: 0.4,
          borderWidth: 2,
          pointRadius: 0,
          fill: false
        }
      ]
    };

    const ctx = document.getElementById('historyChart').getContext('2d');
    const historyChart = new Chart(ctx, {
      type: 'line',
      data: chartData,
      options: {
        responsive: true,
        maintainAspectRatio: false,
        animation: false,
        interaction: { mode: 'index', intersect: false },
        plugins: {
          legend: { 
            labels: { 
              color: '#e2e8f0', 
              usePointStyle: true, 
              boxWidth: 8,
              padding: 16,
              font: { family: 'Inter', size: 12, weight: '500' }
            },
            position: 'top'
          },
          tooltip: {
            backgroundColor: 'rgba(15, 23, 42, 0.95)',
            titleColor: '#f1f5f9',
            bodyColor: '#cbd5e1',
            borderColor: '#334155',
            borderWidth: 1,
            padding: 12,
            cornerRadius: 8,
            titleFont: { family: 'Inter', weight: '600' },
            bodyFont: { family: 'Inter' }
          }
        },
        scales: {
          x: { 
            ticks: { color: '#64748b', maxRotation: 0, autoSkipPadding: 20, font: { size: 11 } },
            grid: { color: 'rgba(51, 65, 85, 0.3)', drawBorder: false }
          },
          y: {
            type: 'linear',
            display: true,
            position: 'left',
            title: { display: true, text: 'RPM', color: '#22d3ee', font: { family: 'Inter', size: 11, weight: '600' } },
            ticks: { color: '#64748b', font: { size: 11 } },
            grid: { color: 'rgba(51, 65, 85, 0.15)', drawBorder: false }
          },
          y1: {
            type: 'linear',
            display: true,
            position: 'right',
            title: { display: true, text: '°C / %', color: '#f87171', font: { family: 'Inter', size: 11, weight: '600' } },
            ticks: { color: '#64748b', font: { size: 11 } },
            grid: { drawOnChartArea: false }
          },
          y2: {
            type: 'linear',
            display: true,
            position: 'right',
            title: { display: true, text: 'CO2 ppm', color: '#a78bfa', font: { family: 'Inter', size: 11, weight: '600' } },
            ticks: { color: '#64748b', font: { size: 11 } },
            grid: { drawOnChartArea: false }
          }
        }
      }
    });

    // Direction display helper – uses semantic colors
    function formatDirection(val) {
      if (!val || val === '--') return '<span class="val-text">--</span>';
      const s = String(val);
      if (s.includes('Zuluft')) return `<span class="badge" style="background:rgba(34,211,238,0.12);color:#22d3ee;">${sanitizeHTML(s)}</span>`;
      if (s.includes('Abluft')) return `<span class="badge" style="background:rgba(248,113,113,0.12);color:#f87171;">${sanitizeHTML(s)}</span>`;
      if (s.includes('Stillstand')) return `<span class="badge" style="background:rgba(100,116,139,0.2);color:#94a3b8;">${sanitizeHTML(s)}</span>`;
      return `<span class="val-text">${sanitizeHTML(s)}</span>`;
    }

    // CO2 rating with color
    function formatCO2Rating(val) {
      if (!val || val === '--') return '<span class="val-text">--</span>';
      const s = String(val);
      if (s === 'Gut' || s === 'Sehr gut') return `<span class="badge" style="background:rgba(52,211,153,0.12);color:#34d399;">${sanitizeHTML(s)}</span>`;
      if (s === 'Mäßig' || s === 'Mittel') return `<span class="badge" style="background:rgba(251,191,36,0.12);color:#fbbf24;">${sanitizeHTML(s)}</span>`;
      if (s === 'Schlecht' || s === 'Sehr schlecht') return `<span class="badge" style="background:rgba(248,113,113,0.12);color:#f87171;">${sanitizeHTML(s)}</span>`;
      return `<span class="val-text">${sanitizeHTML(s)}</span>`;
    }

    async function updateData() {
      try {
        const res = await fetch('/state');
        if (!res.ok) return;
        const data = await res.json();
        
        // Update direct text elements
        const ids = ["device_id", "floor_id", "room_id", "phase", 
                     "temperature", "pressure", "outdoor_humidity", "temp_zuluft", "temp_abluft", 
                     "heat_recovery_efficiency", "fan_rpm", "room_co2", 
                     "room_temperature", "room_humidity", "filter_operating_days"];
        
        ids.forEach(id => {
          const el = document.getElementById("val_" + id);
          if (el && data[id] !== null) {
            let num = parseFloat(data[id]);
            let strVal = typeof data[id] === 'string' ? data[id] : String(data[id]);
            el.childNodes[0].textContent = isNaN(num) ? sanitizeHTML(strVal) : (Number.isInteger(num) ? num : num.toFixed(1));
          }
        });

        // Direction display – uses semantic badges
        const dirEl = document.getElementById("val_direction_display");
        if (dirEl && data.direction_display !== null) {
          dirEl.innerHTML = formatDirection(data.direction_display);
        }

        // CO2 Bewertung – colored badge
        const co2BewEl = document.getElementById("val_room_co2_bewertung");
        if (co2BewEl && data.room_co2_bewertung !== null) {
          co2BewEl.innerHTML = formatCO2Rating(data.room_co2_bewertung);
        }

        // Binary sensors
        const alarmEl = document.getElementById("val_filter_change_alarm");
        if (alarmEl) {
          if (data.filter_change_alarm) {
            alarmEl.innerHTML = '<span class="badge" style="background:rgba(248,113,113,0.15);color:#f87171;">ALARM</span>';
          } else {
            alarmEl.innerHTML = '<span class="badge" style="background:rgba(52,211,153,0.12);color:#34d399;">OK</span>';
          }
        }
        
        const radarEl = document.getElementById("val_radar_presence");
        if (radarEl) {
          if (data.radar_presence) {
            radarEl.innerHTML = '<span class="badge" style="background:rgba(34,211,238,0.12);color:#22d3ee;">Ja</span>';
          } else {
            radarEl.innerHTML = '<span class="badge" style="background:rgba(100,116,139,0.15);color:#94a3b8;">Nein</span>';
          }
        }

        // Inputs / Selects (only update if not currently focused)
        if (document.activeElement.id !== "luefter_modus" && data.luefter_modus) {
          document.getElementById("luefter_modus").value = data.luefter_modus;
        }
        
        const uiControls = ['fan_intensity_display', 'automatik_min_luefterstufe', 'automatik_max_luefterstufe', 'auto_co2_threshold', 'auto_humidity_threshold', 'auto_presence_slider'];
        uiControls.forEach(ctrl => {
           if (document.activeElement.id !== ctrl && data[ctrl] !== null) {
              document.getElementById(ctrl).value = data[ctrl];
           }
        });

        document.getElementById("label_fan_intensity").innerText = document.getElementById("fan_intensity_display").value;
        
        // Render ESP-NOW Peers
        document.getElementById('peers_card').classList.remove('hidden');
        const container = document.getElementById('peers_container');
        
        let localPhaseBadge = "<span class='badge' style='background:rgba(100,116,139,0.2);color:#94a3b8;'>--</span>";
        if (data.direction_display && data.direction_display.includes("Zuluft")) {
            localPhaseBadge = "<span class='badge' style='background:rgba(34,211,238,0.12);color:#22d3ee;'>IN</span>";
        } else if (data.direction_display && data.direction_display.includes("Abluft")) {
            localPhaseBadge = "<span class='badge' style='background:rgba(248,113,113,0.12);color:#f87171;'>OUT</span>";
        }
        
        const localRPM = (data.fan_rpm !== null && data.fan_rpm !== undefined && !isNaN(data.fan_rpm)) ? Number(data.fan_rpm).toFixed(0) : "--";
        const localBoardT = (data.temperature !== null && data.temperature !== undefined && !isNaN(data.temperature)) ? Number(data.temperature).toFixed(1) + " °C" : "--";
        const localRoomT = (data.room_temp !== null && data.room_temp !== undefined && !isNaN(data.room_temp)) ? Number(data.room_temp).toFixed(1) + " °C" : "--";
        const localPID = (data.pid_demand !== null && data.pid_demand !== undefined && !isNaN(data.pid_demand)) ? (Math.round(data.pid_demand * 100) + "%") : "--";
        const localMode = data.luefter_modus === 'Wärmerückgewinnung' ? 'WRG' : (data.luefter_modus || '--');

        let html = `<div class="bg-surface/30 rounded-xl p-4 border border-subtle/40 transition-all duration-200 hover:border-subtle/70 hover:shadow-lg">
            <div class="font-semibold text-gray-300 mb-3 pb-2 border-b border-subtle/40 flex justify-between items-center">
              <span>Gerät ${sanitizeHTML(String(data.device_id || "--"))} <span class="text-xs font-normal text-muted">(lokal)</span></span>
              <span class="w-2.5 h-2.5 rounded-full bg-muted"></span>
            </div>
            <div class="flex justify-between text-sm mb-3">
                <span class="text-muted">Modus: <strong class="text-gray-200">${sanitizeHTML(localMode)}</strong></span>
                <span class="flex items-center gap-2">Stufe: <strong class="text-gray-200">${sanitizeHTML(String(data.fan_intensity_display || "--"))}</strong> ${localPhaseBadge}</span>
            </div>
            <div class="grid grid-cols-2 gap-x-4 gap-y-2 text-xs bg-bg/40 p-3 rounded-lg">
                <div class="flex justify-between"><span class="text-muted">Lüfter RPM</span> <strong class="val-text text-sm">${localRPM}</strong></div>
                <div class="flex justify-between"><span class="text-muted">Board-temp</span> <strong class="val-text text-sm">${localBoardT}</strong></div>
                <div class="flex justify-between"><span class="text-muted">Raum-temp</span> <strong class="val-text text-sm">${localRoomT}</strong></div>
                <div class="flex justify-between"><span class="text-muted">PID</span> <strong class="text-accent text-sm font-semibold">${localPID}</strong></div>
            </div>
        </div>`;

        if (data.peers && data.peers.length > 0) {
          data.peers.forEach(peer => {
            const modeNames = ["Aus", "WRG", "Durchlüften", "Stoßlüftung"];
            const mode = peer.mode >= 0 && peer.mode <= 3 ? modeNames[peer.mode] : "Unbekannt";
            const phase = peer.phase ? "<span class='badge' style='background:rgba(34,211,238,0.12);color:#22d3ee;'>IN</span>" : "<span class='badge' style='background:rgba(248,113,113,0.12);color:#f87171;'>OUT</span>";
            
            const rpm = (peer.rpm !== undefined && peer.rpm !== null && !isNaN(peer.rpm)) ? Number(peer.rpm).toFixed(0) : "--";
            const boardT = (peer.board_t !== undefined && peer.board_t !== null && !isNaN(peer.board_t)) ? Number(peer.board_t).toFixed(1) + " °C" : "--";
            const roomT = (peer.room_t !== undefined && peer.room_t !== null && !isNaN(peer.room_t)) ? Number(peer.room_t).toFixed(1) + " °C" : "--";
            const pid = (peer.pid_demand !== undefined && peer.pid_demand !== null && !isNaN(peer.pid_demand)) ? (Math.round(peer.pid_demand * 100) + "%") : "--";
            
            html += `<div class="bg-card rounded-xl p-4 border border-gray-800/60 transition-all duration-200 hover:border-subtle hover:shadow-lg">
                <div class="font-semibold text-white mb-3 pb-2 border-b border-subtle/40 flex justify-between items-center">
                  <span>Gerät ${sanitizeHTML(String(peer.device_id))}</span>
                  <span class="w-2.5 h-2.5 rounded-full bg-accent pulse-online"></span>
                </div>
                <div class="flex justify-between text-sm mb-3">
                    <span class="text-muted">Modus: <strong class="text-gray-200">${sanitizeHTML(mode)}</strong></span>
                    <span class="flex items-center gap-2">Stufe: <strong class="text-gray-200">${sanitizeHTML(String(peer.speed))}</strong> ${phase}</span>
                </div>
                <div class="grid grid-cols-2 gap-x-4 gap-y-2 text-xs bg-bg/40 p-3 rounded-lg">
                    <div class="flex justify-between"><span class="text-muted">Lüfter RPM</span> <strong class="val-text text-sm">${rpm}</strong></div>
                    <div class="flex justify-between"><span class="text-muted">Board-temp</span> <strong class="val-text text-sm">${boardT}</strong></div>
                    <div class="flex justify-between"><span class="text-muted">Raum-temp</span> <strong class="val-text text-sm">${roomT}</strong></div>
                    <div class="flex justify-between"><span class="text-muted">PID</span> <strong class="text-accent text-sm font-semibold">${pid}</strong></div>
                </div>
            </div>`;
          });
        }
        container.innerHTML = html;
        
        // Update Chart
        const now = new Date();
        const timeStr = now.getHours().toString().padStart(2, '0') + ':' + 
                        now.getMinutes().toString().padStart(2, '0') + ':' + 
                        now.getSeconds().toString().padStart(2, '0');

        chartData.labels.push(timeStr);
        chartData.datasets[0].data.push(data.fan_rpm === null ? null : parseFloat(data.fan_rpm));
        chartData.datasets[1].data.push(data.room_temperature === null ? null : parseFloat(data.room_temperature));
        chartData.datasets[2].data.push(data.room_co2 === null ? null : parseFloat(data.room_co2));
        chartData.datasets[3].data.push(data.room_humidity === null ? null : parseFloat(data.room_humidity));

        if (chartData.labels.length > maxHistoryPoints) {
            chartData.labels.shift();
            chartData.datasets.forEach(dataset => dataset.data.shift());
        }
        historyChart.update();

      } catch (e) {
        console.error('Error fetching state:', e);
      }
    }

    async function sendSet(id, val) {
      const allowedKeys = ['luefter_modus', 'fan_intensity_display', 'automatik_min_luefterstufe', 
                          'automatik_max_luefterstufe', 'auto_co2_threshold', 'auto_humidity_threshold',
                          'auto_presence_slider', 'vent_timer', 'sync_interval_config'];

      if (!allowedKeys.includes(id)) {
          console.error('Client validation failed: Invalid parameter =', id);
          return;
      }
      if (id === 'fan_intensity_display' && (val < 1 || val > 10)) {
          console.error('Client validation failed: Invalid fan intensity =', val);
          return;
      }

      if (id === 'fan_intensity_display') document.getElementById("label_fan_intensity").innerText = val;

      try {
        await fetch(`/set?id=${encodeURIComponent(id)}&val=${encodeURIComponent(val)}`);
      } catch (e) {
        console.error('Error setting value:', e);
      }
      setTimeout(updateData, 500); // refresh after delay
    }

    setInterval(updateData, 2000);
    updateData(); // initial call
  </script>
</body>
</html>
)=====";
