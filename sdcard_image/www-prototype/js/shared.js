// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Shared Utilities
// Vanilla JS · Shoelace Web Components
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
//
// Licensed under the GNU Lesser General Public License (LGPL 3.0).
// https://www.gnu.org/licenses/lgpl-3.0.txt
//
// Part of the dadamachines additions to the CTAG TBD platform.
// See LICENSE in the repository root for full terms.
// ═══════════════════════════════════════════════════════════════
'use strict';

// ─── Constants ───────────────────────────────────────────────
const API_V1 = '/api/v1';

// sun-fill SVG not in our Shoelace bundle — register it as a data URI
// so the theme toggle doesn't trigger a network fetch
const SUN_FILL_SVG = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16"><path d="M8 12a4 4 0 1 0 0-8 4 4 0 0 0 0 8M8 0a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 0m0 13a.5.5 0 0 1 .5.5v2a.5.5 0 0 1-1 0v-2A.5.5 0 0 1 8 13m8-5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2a.5.5 0 0 1 .5.5M3 8a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1 0-1h2A.5.5 0 0 1 3 8m10.657-5.657a.5.5 0 0 1 0 .707l-1.414 1.415a.5.5 0 1 1-.707-.708l1.414-1.414a.5.5 0 0 1 .707 0m-9.193 9.193a.5.5 0 0 1 0 .707L3.05 13.657a.5.5 0 0 1-.707-.707l1.414-1.414a.5.5 0 0 1 .707 0m9.193 2.121a.5.5 0 0 1-.707 0l-1.414-1.414a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707M3.757 4.464a.5.5 0 0 1-.707 0L1.636 3.05a.5.5 0 0 1 .707-.707l1.414 1.414a.5.5 0 0 1 0 .707"/></svg>';

// ═══════════════════════════════════════════════════════════════
//  HTML ESCAPING
// ═══════════════════════════════════════════════════════════════

function esc(s) {
  const d = document.createElement('div');
  d.textContent = s;
  return d.innerHTML;
}

// ═══════════════════════════════════════════════════════════════
//  FORMATTING
// ═══════════════════════════════════════════════════════════════

function formatBytes(bytes) {
  if (bytes < 1024) return `${bytes} B`;
  if (bytes < 1048576) return `${(bytes / 1024).toFixed(1)} KB`;
  if (bytes < 1073741824) return `${(bytes / 1048576).toFixed(1)} MB`;
  return `${(bytes / 1073741824).toFixed(1)} GB`;
}

// ═══════════════════════════════════════════════════════════════
//  API CLIENT
// ═══════════════════════════════════════════════════════════════

/**
 * GET request to /api/v1/<path>
 * @param {string} path - e.g. '/getPlugins' or '/getActivePlugin/0'
 * @returns {Promise<any>} parsed JSON response
 */
// Default timeout for API read requests (ms).
// ESP32 responses are fast (<200ms) but USB NCM can stall;
// 10s matches server-side recv/send_wait_timeout.
var API_TIMEOUT_MS = 10000;
// Longer timeout for mutation operations (loadPreset, savePreset, etc.)
// that trigger SD card I/O on the firmware side.
var API_MUTATION_TIMEOUT_MS = 20000;
// Extra-long timeout for plugin switches — WTOsc, WTOscDuo, Freakwaves, VctrSnt
// trigger ctagSampleRom SD card loading (all wavetable/sample data into PSRAM),
// which can take 15-30+ seconds. The firmware blocks the HTTP response until done.
var API_PLUGIN_SWITCH_TIMEOUT_MS = 45000;

// ─── Circuit Breaker ─────────────────────────────────────────
// Track consecutive API failures.  After _FAILURE_THRESHOLD in a row,
// trigger disconnect + drain the request queue to stop hammering a
// dead device.  Threshold is generous because the ESP32 httpd can
// be temporarily unresponsive during heavy plugin allocation
// (mutex contention with audio task, SD card I/O).
var _consecutiveFailures = 0;
var _FAILURE_THRESHOLD = 4;

async function apiFetch(path, timeoutMs, skipCircuitBreaker) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    const r = await fetch(`${API_V1}${path}`, {
      signal: AbortSignal.timeout(timeoutMs),
    });
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    // Don't count toward circuit breaker if caller opted out.
    // Plugin switch timeouts mean the device is busy (loading sample ROM),
    // NOT offline.  Only genuine network errors should trigger disconnect.
    if (!skipCircuitBreaker) {
      _consecutiveFailures++;
      if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
        setDisconnected();
        apiQueue.drain();
      }
    }
    throw e;
  }
}

/**
 * POST request to /api/v1/<path> with JSON body
 * @param {string} path - e.g. '/setConfiguration'
 * @param {object} body - JSON-serializable data
 * @returns {Promise<any>} parsed JSON response
 */
async function apiPostJSON(path, body, timeoutMs) {
  timeoutMs = timeoutMs || API_TIMEOUT_MS;
  try {
    const r = await fetch(`${API_V1}${path}`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body),
      signal: AbortSignal.timeout(timeoutMs),
    });
    if (!r.ok) throw new Error(`API ${r.status}`);
    _consecutiveFailures = 0;
    var text = await r.text();
    if (!text || !text.trim()) return {};
    try { return JSON.parse(text); } catch(e) { return {}; }
  } catch(e) {
    _consecutiveFailures++;
    if (_consecutiveFailures >= _FAILURE_THRESHOLD) {
      setDisconnected();
      apiQueue.drain();
    }
    throw e;
  }
}

// ═══════════════════════════════════════════════════════════════
//  FETCH QUEUE — Serialize requests to avoid overwhelming ESP32
// ═══════════════════════════════════════════════════════════════

class FetchQueue {
  constructor() {
    this._queue = [];
    this._running = false;
    this._paused = false;
  }

  enqueue(fn) {
    if (this._paused) {
      return Promise.reject(new Error('Device offline \u2014 request cancelled'));
    }
    return new Promise((resolve, reject) => {
      this._queue.push({ fn, resolve, reject });
      this._process();
    });
  }

  async _process() {
    if (this._running) return;
    this._running = true;
    while (this._queue.length && !this._paused) {
      const { fn, resolve, reject } = this._queue.shift();
      try { resolve(await fn()); } catch (e) { reject(e); }
    }
    this._running = false;
  }

  /** Reject all pending items and pause the queue. */
  drain() {
    this._paused = true;
    while (this._queue.length) {
      var item = this._queue.shift();
      item.reject(new Error('Device offline \u2014 request cancelled'));
    }
  }

  /** Resume accepting new items after reconnection. */
  resume() {
    this._paused = false;
  }
}

// Shared queue for serializing parameter SET calls across views
const paramQueue = new FetchQueue();

// Global API queue — ALL API calls route through this to keep
// max 1 in-flight request at a time (ESP32 has only ~4 usable sockets).
const apiQueue = new FetchQueue();

/**
 * Queue-wrapped apiFetch — serializes all GET requests.
 * @param {string} path - e.g. '/getPlugins'
 * @returns {Promise<any>} parsed JSON response
 */
function queuedFetch(path, timeoutMs, skipCircuitBreaker) {
  return apiQueue.enqueue(function() { return apiFetch(path, timeoutMs, skipCircuitBreaker); });
}

/**
 * Queue-wrapped apiPostJSON — serializes all POST requests.
 * @param {string} path - e.g. '/favorites/store/0'
 * @param {object} body
 * @returns {Promise<any>} parsed JSON response
 */
function queuedPost(path, body, timeoutMs) {
  return apiQueue.enqueue(function() { return apiPostJSON(path, body, timeoutMs); });
}

// ═══════════════════════════════════════════════════════════════
//  TOAST NOTIFICATIONS
// ═══════════════════════════════════════════════════════════════

var _toastBusy = false;
var _toastCount = 0;           // throttle: max toasts in flight
var _TOAST_MAX_PENDING = 5;    // discard beyond this to avoid DOM leak

function toast(message, variant, duration) {
  if (_toastBusy) return;          // prevent synchronous recursion
  if (_toastCount >= _TOAST_MAX_PENDING) return;  // throttle
  _toastBusy = true;
  _toastCount++;
  try {
    variant = variant || 'primary';
    duration = duration || 4000;
    var stack = document.getElementById('toast-stack');
    if (!stack) { _toastBusy = false; _toastCount--; return; }
    var alert = document.createElement('sl-alert');
    alert.variant = variant;
    alert.closable = true;
    alert.duration = duration;
    alert.innerHTML = '<sl-icon slot="icon" name="' + iconForVariant(variant) + '">' + '</sl-icon>' + esc(message);
    // Decrement counter when alert is removed from DOM (auto-hide or close)
    alert.addEventListener('sl-after-hide', function() { _toastCount = Math.max(0, _toastCount - 1); });
    stack.appendChild(alert);
    // Only call .toast() if sl-alert is already defined; skip silently otherwise
    if (customElements.get('sl-alert')) {
      try { alert.toast(); } catch(e) { console.warn('toast render failed:', e); }
    }
    // If sl-alert not defined yet, just leave it in DOM (don't queue .whenDefined
    // which causes async re-entrancy problems)
  } catch(e) {
    console.warn('toast() error:', e);
    _toastCount = Math.max(0, _toastCount - 1);
  } finally {
    _toastBusy = false;
  }
}

function iconForVariant(v) {
  var map = {
    success: 'check2-circle',
    warning: 'exclamation-triangle',
    danger: 'exclamation-octagon',
    primary: 'info-circle',
    neutral: 'info-circle'
  };
  return map[v] || 'info-circle';
}

// ═══════════════════════════════════════════════════════════════
//  THEME MANAGEMENT
// ═══════════════════════════════════════════════════════════════

function setupThemeToggle(btnId) {
  var btn = document.getElementById(btnId || 'theme-toggle');
  if (!btn) return;
  var saved = localStorage.getItem('tbd-theme');
  if (saved === 'light') applyTheme('light');
  btn.addEventListener('click', function() {
    var isDark = document.documentElement.classList.contains('sl-theme-dark');
    applyTheme(isDark ? 'light' : 'dark');
  });
}

function applyTheme(theme) {
  var html = document.documentElement;
  var btn  = document.getElementById('theme-toggle');
  if (theme === 'light') {
    html.classList.remove('sl-theme-dark');
    html.classList.add('sl-theme-light');
    if (btn) btn.name = 'sun-fill';
  } else {
    html.classList.remove('sl-theme-light');
    html.classList.add('sl-theme-dark');
    if (btn) btn.name = 'moon-fill';
  }
  // Both themes are loaded in HTML; only the class toggles which one is active
  localStorage.setItem('tbd-theme', theme);
}

// ═══════════════════════════════════════════════════════════════
//  CONNECTION MONITOR
// ═══════════════════════════════════════════════════════════════

var connectionState = {
  status: 'connecting',   // 'connecting' | 'connected' | 'disconnected'
  retries: 0,
  maxRetries: 60,
  pollIntervalMs: 5000,   // WLED uses 5s for HTTP fallback reconnect
  _timer: null,
  _onConnect: null,
  _onDisconnect: null,
};

function startConnectionMonitor(onConnect, onDisconnect) {
  connectionState._onConnect = onConnect;
  connectionState._onDisconnect = onDisconnect;
}

function setConnected() {
  if (connectionState.status === 'connected') return;
  connectionState.status = 'connected';
  connectionState.retries = 0;
  _consecutiveFailures = 0;
  apiQueue.resume();
  updateConnectionUI();
  if (connectionState._onConnect) connectionState._onConnect();
}

function setDisconnected() {
  if (connectionState.status === 'disconnected') return;
  connectionState.status = 'disconnected';
  updateConnectionUI();
  if (connectionState._onDisconnect) connectionState._onDisconnect();
  scheduleReconnect();
}

function scheduleReconnect() {
  if (connectionState._timer) return;
  connectionState._timer = setInterval(async function() {
    if (connectionState.retries >= connectionState.maxRetries) {
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      return;
    }
    // Skip poll if the API queue is busy (user-initiated request in-flight)
    if (apiQueue._running) return;
    connectionState.retries++;
    try {
      await apiFetch('/getIOCaps');
      clearInterval(connectionState._timer);
      connectionState._timer = null;
      setConnected();
    } catch (e) {
      // still disconnected
    }
  }, connectionState.pollIntervalMs);
}

function updateConnectionUI() {
  var el = document.getElementById('status-text');
  if (el) {
    if (connectionState.status === 'connected') {
      el.textContent = 'Connected';
      el.style.color = 'var(--sl-color-success-600)';
    } else if (connectionState.status === 'disconnected') {
      el.textContent = 'Offline';
      el.style.color = 'var(--sl-color-danger-600)';
    } else {
      el.textContent = 'Connecting\u2026';
      el.style.color = 'var(--sl-color-neutral-500)';
    }
  }
  // Update footer connection dot
  var dot = document.getElementById('footer-conn-dot');
  var txt = document.getElementById('footer-conn-text');
  if (dot) {
    dot.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (txt) {
    txt.textContent = connectionState.status === 'connected' ? 'Connected' :
                      connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
  // Update header connection pill
  var pill = document.getElementById('conn-pill');
  var pillText = document.getElementById('conn-pill-text');
  if (pill) {
    pill.classList.toggle('offline', connectionState.status !== 'connected');
  }
  if (pillText) {
    pillText.textContent = connectionState.status === 'connected' ? 'Connected' :
                           connectionState.status === 'disconnected' ? 'Offline' : 'Connecting\u2026';
  }
}

// ═══════════════════════════════════════════════════════════════
//  DIALOG HELPERS
// ═══════════════════════════════════════════════════════════════

/**
 * Wrap a Shoelace sl-dialog in a Promise for await-friendly confirmation.
 * @param {string} dialogId - DOM id of the sl-dialog
 * @param {string} okBtnId - DOM id of the confirm button
 * @param {string} cancelBtnId - DOM id of the cancel button
 * @returns {Promise<boolean>} true if confirmed, false if cancelled
 */
function confirmDialog(dialogId, okBtnId, cancelBtnId) {
  return new Promise(function(resolve) {
    var dlg = document.getElementById(dialogId);
    var ok  = document.getElementById(okBtnId);
    var cancel = document.getElementById(cancelBtnId);
    if (!dlg) { resolve(false); return; }

    function cleanup() {
      ok.removeEventListener('click', onOk);
      if (cancel) cancel.removeEventListener('click', onCancel);
      dlg.removeEventListener('sl-request-close', onCancel);
    }
    function onOk() { cleanup(); dlg.hide(); resolve(true); }
    function onCancel() { cleanup(); dlg.hide(); resolve(false); }

    ok.addEventListener('click', onOk);
    if (cancel) cancel.addEventListener('click', onCancel);
    dlg.addEventListener('sl-request-close', onCancel);
    dlg.show();
  });
}

// ═══════════════════════════════════════════════════════════════
//  LOADING OVERLAY — visual feedback during heavy operations
// ═══════════════════════════════════════════════════════════════

function showLoading(message) {
  var overlay = document.getElementById('loading-overlay');
  var text = document.getElementById('loading-text');
  if (overlay) {
    if (text) text.textContent = message || 'Loading\u2026';
    overlay.classList.remove('hidden');
  }
}

function hideLoading() {
  var overlay = document.getElementById('loading-overlay');
  if (overlay) overlay.classList.add('hidden');
}

// ═══════════════════════════════════════════════════════════════
//  EXPORTS — attach to window for non-module scripts
// ═══════════════════════════════════════════════════════════════

// ─── Control Mode ──────────────────────────────────────────
var _waLoaded = false;

function isControlMode() {
  return localStorage.getItem('tbd-control-mode') === '1';
}

function setControlMode(on) {
  localStorage.setItem('tbd-control-mode', on ? '1' : '0');
  if (on && !_waLoaded) loadWebAudioControls();
}

function loadWebAudioControls() {
  // webaudio-controls.js is now included in the app-bundle.js
  // so it's always available — no dynamic loading needed
  _waLoaded = true;
  return Promise.resolve();
}

// ═══════════════════════════════════════════════════════════════
//  SVG KNOB RENDERER — shared between Performer & Designer
// ═══════════════════════════════════════════════════════════════

/**
 * Render an SVG arc knob.
 * @param {object} opts
 * @param {number} opts.value - Current value
 * @param {number} opts.min - Min value (default 0)
 * @param {number} opts.max - Max value (default 127)
 * @param {string} opts.color - Arc color: 'blue' (default, performer) or 'amber' (designer preview)
 * @param {number} opts.size - SVG size in px (default 68)
 * @returns {string} SVG markup string
 */
function renderKnobSVG(opts) {
  var value = opts.value || 0;
  var min = opts.min || 0;
  var max = opts.max || 127;
  var size = opts.size || 68;
  var color = opts.color || 'blue';

  var pct = max > min ? ((value - min) / (max - min)) : 0;
  pct = Math.max(0, Math.min(1, pct));

  // Arc geometry: 270° sweep from 225° (7:30 position) to 135° (4:30 position)
  var cx = size / 2;
  var cy = size / 2;
  var r = (size / 2) - 6;           // arc radius
  var rInner = r - 10;              // inner circle radius
  var strokeWidth = 4;

  var startAngle = 225;
  var endAngle = startAngle + 270;  // 495° = 135°

  // Value angle
  var valueAngle = startAngle + (pct * 270);

  function polarToCartesian(angle) {
    var rad = (angle - 90) * Math.PI / 180;
    return {
      x: cx + r * Math.cos(rad),
      y: cy + r * Math.sin(rad)
    };
  }

  function describeArc(start, end) {
    var sP = polarToCartesian(start);
    var eP = polarToCartesian(end);
    var sweep = (end - start) <= 180 ? 0 : 1;
    return 'M ' + sP.x + ' ' + sP.y + ' A ' + r + ' ' + r + ' 0 ' + sweep + ' 1 ' + eP.x + ' ' + eP.y;
  }

  // Indicator dot position
  var dotPos = polarToCartesian(valueAngle);

  // Color palette
  var trackColor, valueColor, dotColor;
  if (color === 'amber') {
    trackColor = 'var(--sl-color-neutral-300)';
    valueColor = 'var(--sl-color-amber-500, #f59e0b)';
    dotColor = 'var(--sl-color-amber-600, #d97706)';
  } else {
    trackColor = 'var(--sl-color-neutral-300)';
    valueColor = 'var(--sl-color-primary-500)';
    dotColor = 'var(--sl-color-primary-700)';
  }

  var svg = '<svg width="' + size + '" height="' + size + '" viewBox="0 0 ' + size + ' ' + size + '" class="knob-svg">';

  // Background track arc (full 270°)
  svg += '<path d="' + describeArc(startAngle, endAngle) + '" fill="none" stroke="' + trackColor + '" stroke-width="' + strokeWidth + '" stroke-linecap="round" />';

  // Value arc
  if (pct > 0.005) {
    svg += '<path d="' + describeArc(startAngle, valueAngle) + '" fill="none" stroke="' + valueColor + '" stroke-width="' + strokeWidth + '" stroke-linecap="round" />';
  }

  // Inner circle (knob body)
  svg += '<circle cx="' + cx + '" cy="' + cy + '" r="' + rInner + '" fill="var(--sl-color-neutral-50)" stroke="var(--sl-color-neutral-200)" stroke-width="1" />';

  // Indicator dot
  svg += '<circle cx="' + dotPos.x.toFixed(1) + '" cy="' + dotPos.y.toFixed(1) + '" r="3.5" fill="' + dotColor + '" />';

  svg += '</svg>';
  return svg;
}

// ═══════════════════════════════════════════════════════════════
//  SHARED DATA STORE — both Performer and Designer use this
// ═══════════════════════════════════════════════════════════════

var sharedData = {
  synthDefs: null,
  tracks: [],
  machines: [],
  macroDefs: [],
  soundPresets: [],
  activeTrack: -1,
  loaded: false,
};

var _trackChangeCallbacks = [];

/**
 * Load all data from the device (synthdefs, macrodefs, soundpresets).
 * Called once at boot; both views read from sharedData.
 */
function loadSharedData() {
  showLoading('Loading tracks & definitions…');
  return Promise.all([
    fetch('/api/v1/samples?getconfig=synthdefinitions.json').then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    }),
    fetch('/api/v1/macroapi/definitions').then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    }),
    fetch('/api/v1/macroapi/soundpresets').then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    }),
  ]).then(function(results) {
    sharedData.synthDefs = results[0];
    sharedData.tracks = results[0].tracks || [];
    sharedData.machines = results[0].machines || [];
    sharedData.macroDefs = results[1] || [];
    sharedData.soundPresets = results[2] || [];
    sharedData.loaded = true;
    hideLoading();
    console.log('[Shared] Loaded:', sharedData.tracks.length, 'tracks,',
                sharedData.machines.length, 'machines,',
                sharedData.macroDefs.length, 'macro defs,',
                sharedData.soundPresets.length, 'sound presets');
    return sharedData;
  }).catch(function(err) {
    hideLoading();
    console.error('[Shared] Load error:', err);
    toast('Failed to load data: ' + err.message, 'danger', 4000);
    throw err;
  });
}

/**
 * Reload macro definitions and sound presets (after save).
 */
function reloadMacroData() {
  return Promise.all([
    fetch('/api/v1/macroapi/definitions').then(function(r) { return r.ok ? r.json() : []; }),
    fetch('/api/v1/macroapi/soundpresets').then(function(r) { return r.ok ? r.json() : []; }),
  ]).then(function(results) {
    sharedData.macroDefs = results[0] || [];
    sharedData.soundPresets = results[1] || [];
    return sharedData;
  });
}

/**
 * Register a callback for track changes.
 * Callback receives (trackIndex, track).
 */
function onTrackChange(callback) {
  _trackChangeCallbacks.push(callback);
}

/**
 * Select a track. Updates shared state and notifies all listeners.
 */
function selectSharedTrack(idx) {
  var track = sharedData.tracks.find(function(t) { return t.index === idx; });
  if (!track) return;

  sharedData.activeTrack = idx;

  // Update track strip visuals
  document.querySelectorAll('.track-strip').forEach(function(s) {
    s.classList.toggle('active', parseInt(s.getAttribute('data-track'), 10) === idx);
  });

  // Notify all registered listeners
  _trackChangeCallbacks.forEach(function(cb) {
    try { cb(idx, track); } catch(e) { console.error('Track change callback error:', e); }
  });
}

/**
 * Get machine info by id from shared data.
 */
function getMachineInfo(machineId) {
  return sharedData.machines.find(function(m) { return m.id === machineId; }) || null;
}

/**
 * Get available (non-empty) machines for a track.
 */
function getTrackMachines(track) {
  return (track.machines || []).filter(function(m) {
    return m !== 'nodrum' && m !== 'nosynth' && m !== 'nofx';
  });
}

/**
 * Render the shared track overview strip.
 */
function renderTrackOverview() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  var html = '';
  sharedData.tracks.forEach(function(track) {
    var classes = 'track-strip';
    if (track.index === sharedData.activeTrack) classes += ' active';

    var avail = getTrackMachines(track);
    var defaultMachine = avail.length > 0 ? avail[0] : '—';

    html += '<div class="' + classes + '" data-track="' + track.index + '">';
    html += '<span class="track-num">' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-name">' + esc(track.name) + '</span>';
    html += '<span class="track-machine">' + esc(defaultMachine) + '</span>';
    html += '</div>';
  });

  container.innerHTML = html;
}

/**
 * Set up click events on the shared track overview strip.
 */
function setupTrackOverviewEvents() {
  var container = document.getElementById('track-overview');
  if (!container) return;

  container.addEventListener('click', function(e) {
    var strip = e.target.closest('.track-strip');
    if (!strip) return;
    var trackIdx = parseInt(strip.getAttribute('data-track'), 10);
    selectSharedTrack(trackIdx);
  });
}

window.TBD = window.TBD || {};
window.TBD.shared = {
  API_V1: API_V1,
  API_TIMEOUT_MS: API_TIMEOUT_MS,
  API_MUTATION_TIMEOUT_MS: API_MUTATION_TIMEOUT_MS,
  API_PLUGIN_SWITCH_TIMEOUT_MS: API_PLUGIN_SWITCH_TIMEOUT_MS,
  esc: esc,
  formatBytes: formatBytes,
  apiFetch: apiFetch,
  apiPostJSON: apiPostJSON,
  queuedFetch: queuedFetch,
  queuedPost: queuedPost,
  FetchQueue: FetchQueue,
  paramQueue: paramQueue,
  apiQueue: apiQueue,
  toast: toast,
  iconForVariant: iconForVariant,
  setupThemeToggle: setupThemeToggle,
  applyTheme: applyTheme,
  connectionState: connectionState,
  startConnectionMonitor: startConnectionMonitor,
  setConnected: setConnected,
  setDisconnected: setDisconnected,
  updateConnectionUI: updateConnectionUI,
  confirmDialog: confirmDialog,
  isControlMode: isControlMode,
  setControlMode: setControlMode,
  loadWebAudioControls: loadWebAudioControls,
  showLoading: showLoading,
  hideLoading: hideLoading,
  // SVG knob renderer
  renderKnobSVG: renderKnobSVG,
  // Shared data & track management
  data: sharedData,
  loadSharedData: loadSharedData,
  reloadMacroData: reloadMacroData,
  onTrackChange: onTrackChange,
  selectTrack: selectSharedTrack,
  getMachineInfo: getMachineInfo,
  getTrackMachines: getTrackMachines,
  renderTrackOverview: renderTrackOverview,
  setupTrackOverviewEvents: setupTrackOverviewEvents,
};
