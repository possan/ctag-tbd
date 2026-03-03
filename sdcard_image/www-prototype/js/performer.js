// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Performer View
// Persona: Performer / Artist
//
// Focused on performance: macro knobs, preset recall, track overview.
// Shows only the high-level macro parameters (6 groups × 4 params),
// NOT the raw DSP parameters.
//
// Data flow:
//   Macro Sound Presets → Macro Definitions → DSP CC values
//   (All resolved by MacroTranslator on the device.)
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    tracks: [],             // Track metadata (16 tracks)
    activeTrack: -1,        // Currently selected track index (0-15)
    macroDefinitions: {},   // Loaded macro definitions keyed by filename
    soundPresets: {},       // Available sound presets
    currentPreset: null,    // Active preset for selected track
    currentMacroDef: null,  // Active macro definition for selected track
    presetSearchTerm: '',
    initialized: false,
  };

  // ─── Mock Data (for prototype — replaced by API calls in production) ──

  /**
   * Generate mock track data for 16-track PicoSeqRack.
   * In production, this comes from the device via API.
   */
  function generateMockTracks() {
    var trackNames = [
      'Kick',     'Snare',    'Hi-Hat',   'Clap',
      'Perc 1',   'Perc 2',   'Bass',     'Lead',
      'Pad',      'FX 1',     'FX 2',     'Chord',
      'Arp',      'Stab',     'Sub',      'Master'
    ];
    var machines = [
      'RackDBD',   'RackDSD',   'RackHH1',  'RackDSD',
      'RackDBD',   'RackHH1',   'RackTBD03','RackSynth',
      'RackSynth', 'RackFxDelay','RackFxDelay','RackSynth',
      'RackSynth', 'RackRompler','RackDBD',  'Mixer'
    ];
    var tracks = [];
    for (var i = 0; i < 16; i++) {
      tracks.push({
        index: i,
        name: trackNames[i],
        machine: machines[i],
        muted: false,
        solo: false,
      });
    }
    return tracks;
  }

  /**
   * Generate mock macro definition for a track.
   * Structure: 6 parameter groups × 4 parameters each
   */
  function generateMockMacroDefinition(trackName, machine) {
    var groups = [];
    // Create meaningful parameter groups based on machine type
    var groupDefs;
    if (machine === 'RackDBD' || machine === 'RackDSD') {
      groupDefs = [
        { name: 'Tone',      params: ['Frequency', 'Tone', 'Decay', 'Noise'] },
        { name: 'Shape',     params: ['Drive', 'Shape', 'Dirty', 'FM Decay'] },
        { name: 'Dynamics',  params: ['Level', 'Accent', 'Attack', 'Sustain'] },
        { name: 'Modulation',params: ['LFO Rate', 'LFO Depth', 'Env Mod', 'Pitch'] },
        { name: 'Effects',   params: ['FX Send 1', 'FX Send 2', 'Pan', 'Width'] },
        { name: 'Mix',       params: ['Volume', 'Mute', 'Solo', 'Comp'] },
      ];
    } else if (machine === 'RackTBD03') {
      groupDefs = [
        { name: 'Oscillator', params: ['Shape', 'Tune', 'Detune', 'Sub Level'] },
        { name: 'Filter',     params: ['Cutoff', 'Resonance', 'Env Mod', 'Key Track'] },
        { name: 'Envelope',   params: ['Attack', 'Decay', 'Sustain', 'Release'] },
        { name: 'Accent',     params: ['Accent', 'Slide', 'Overdrive', 'Dist Type'] },
        { name: 'Effects',    params: ['FX Send 1', 'FX Send 2', 'Pan', 'Width'] },
        { name: 'Mix',        params: ['Volume', 'Mute', 'Solo', 'Comp'] },
      ];
    } else if (machine === 'RackHH1') {
      groupDefs = [
        { name: 'Tone',      params: ['Freq High', 'Freq Low', 'Tone', 'Noise'] },
        { name: 'Envelope',  params: ['Decay', 'Attack', 'Hold', 'Release'] },
        { name: 'Character', params: ['Metallic', 'Ring', 'Saturate', 'Bit Red.'] },
        { name: 'Open/Close',params: ['Open Level', 'Closed Level', 'Choke', 'Threshold'] },
        { name: 'Effects',   params: ['FX Send 1', 'FX Send 2', 'Pan', 'Width'] },
        { name: 'Mix',       params: ['Volume', 'Mute', 'Solo', 'Comp'] },
      ];
    } else {
      groupDefs = [
        { name: 'Group 1', params: ['Param A', 'Param B', 'Param C', 'Param D'] },
        { name: 'Group 2', params: ['Param E', 'Param F', 'Param G', 'Param H'] },
        { name: 'Group 3', params: ['Param I', 'Param J', 'Param K', 'Param L'] },
        { name: 'Group 4', params: ['Param M', 'Param N', 'Param O', 'Param P'] },
        { name: 'Group 5', params: ['Param Q', 'Param R', 'Param S', 'Param T'] },
        { name: 'Group 6', params: ['Param U', 'Param V', 'Param W', 'Param X'] },
      ];
    }

    groupDefs.forEach(function(gd, gi) {
      var params = [];
      gd.params.forEach(function(pname, pi) {
        params.push({
          name: pname,
          value: Math.floor(Math.random() * 128),
          min: 0,
          max: 127,
        });
      });
      groups.push({
        name: gd.name,
        index: gi,
        params: params,
      });
    });

    return {
      name: trackName + ' Macro',
      groups: groups,
    };
  }

  /**
   * Generate mock sound presets.
   */
  function generateMockPresets() {
    var categories = {
      'KICKS': [
        { name: '808 Deep Kick', machine: 'RackDBD' },
        { name: '909 Tight Kick', machine: 'RackDBD' },
        { name: 'Sub Boom', machine: 'RackDBD' },
        { name: 'Punchy Analog', machine: 'RackDBD' },
        { name: 'Lo-Fi Thump', machine: 'RackDBD' },
      ],
      'SNARES': [
        { name: 'Snappy Snare', machine: 'RackDSD' },
        { name: 'Fat Clap', machine: 'RackDSD' },
        { name: 'Tight Rim', machine: 'RackDSD' },
        { name: 'Layered Crack', machine: 'RackDSD' },
      ],
      'HI-HATS': [
        { name: 'Crispy Closed', machine: 'RackHH1' },
        { name: 'Sizzle Open', machine: 'RackHH1' },
        { name: 'Dark Hat', machine: 'RackHH1' },
      ],
      'BASS': [
        { name: 'Acid Squelch', machine: 'RackTBD03' },
        { name: 'Rubber Bass', machine: 'RackTBD03' },
        { name: 'Deep Sub', machine: 'RackTBD03' },
        { name: 'Growl Bass', machine: 'RackTBD03' },
      ],
      'SYNTHS': [
        { name: 'Warm Pad', machine: 'RackSynth' },
        { name: 'Bright Lead', machine: 'RackSynth' },
        { name: 'Pluck Stab', machine: 'RackSynth' },
      ],
      'EFFECTS': [
        { name: 'Tape Delay', machine: 'RackFxDelay' },
        { name: 'Shimmer Verb', machine: 'RackFxDelay' },
      ],
    };

    return categories;
  }

  // ─── Track Overview ──────────────────────────────────────

  function renderTrackOverview() {
    var container = document.getElementById('track-overview');
    if (!container) return;

    var html = '';
    state.tracks.forEach(function(track) {
      var classes = 'track-strip';
      if (track.index === state.activeTrack) classes += ' active';
      if (track.muted) classes += ' muted';

      html += '<div class="' + classes + '" data-track="' + track.index + '">';
      html += '<span class="track-num">' + String(track.index + 1).padStart(2, '0') + '</span>';
      html += '<span class="track-name">' + S.esc(track.name) + '</span>';
      html += '<span class="track-machine">' + S.esc(track.machine.replace('Rack', '')) + '</span>';
      html += '</div>';
    });
    container.innerHTML = html;
  }

  function setupTrackOverviewEvents() {
    var container = document.getElementById('track-overview');
    if (!container) return;

    container.addEventListener('click', function(e) {
      var strip = e.target.closest('.track-strip');
      if (!strip) return;
      var trackIdx = parseInt(strip.getAttribute('data-track'), 10);
      selectTrack(trackIdx);
    });
  }

  function selectTrack(idx) {
    if (idx < 0 || idx >= state.tracks.length) return;
    state.activeTrack = idx;

    // Update track strip active state
    document.querySelectorAll('.track-strip').forEach(function(s) {
      s.classList.toggle('active', parseInt(s.getAttribute('data-track'), 10) === idx);
    });

    // Load macro controls for selected track
    var track = state.tracks[idx];
    var macroDef = generateMockMacroDefinition(track.name, track.machine);
    state.currentMacroDef = macroDef;

    renderMacroControls(track, macroDef);
  }

  // ─── Macro Controls Rendering ────────────────────────────

  function renderMacroControls(track, macroDef) {
    var container = document.getElementById('macro-controls');
    if (!container) return;

    var html = '';

    // Track info header
    html += '<div class="track-info-header">';
    html += '<span class="track-badge">CH ' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-title">' + S.esc(track.name) + '</span>';
    html += '<span class="track-subtitle">' + S.esc(track.machine) + '</span>';
    html += '</div>';

    // Render each macro group
    macroDef.groups.forEach(function(group) {
      html += '<div class="macro-group" data-group="' + group.index + '">';

      // Group header
      html += '<div class="macro-group-header">';
      html += '<sl-icon name="chevron-down" class="macro-group-chevron"></sl-icon>';
      html += '<span class="macro-group-name">' + S.esc(group.name) + '</span>';
      html += '</div>';

      // Group body with knob grid (4 columns)
      html += '<div class="macro-group-body">';
      group.params.forEach(function(param, pi) {
        var pct = Math.round((param.value / param.max) * 100);
        html += '<div class="macro-knob-cell" data-group="' + group.index + '" data-param="' + pi + '">';
        html += '<div class="macro-knob" style="--knob-pct:' + pct + '" ';
        html += 'data-value="' + param.value + '" data-min="' + param.min + '" data-max="' + param.max + '">';
        html += '<span class="knob-indicator" style="' + knobIndicatorStyle(pct) + '"></span>';
        html += '</div>';
        html += '<span class="macro-knob-label">' + S.esc(param.name) + '</span>';
        html += '<span class="macro-knob-value">' + param.value + '</span>';
        html += '</div>';
      });
      html += '</div>';

      html += '</div>';
    });

    container.innerHTML = html;
    setupMacroKnobEvents(container);
    setupMacroGroupEvents(container);
  }

  /**
   * Calculate CSS position for the knob indicator dot.
   * Maps 0-100% to a position on the knob's perimeter.
   */
  function knobIndicatorStyle(pct) {
    // Map 0-100 to angle: 225° (start) → 315° (end) going clockwise (full range = 270°)
    var angle = 225 + (pct / 100) * 270;
    var rad = (angle * Math.PI) / 180;
    var radius = 27; // half of 54px (inside the 68px knob, accounting for border + inner circle)
    var cx = 34 + Math.cos(rad) * radius - 3; // center x - half dot width
    var cy = 34 + Math.sin(rad) * radius - 3; // center y - half dot height
    return 'left:' + cx.toFixed(1) + 'px;top:' + cy.toFixed(1) + 'px;';
  }

  function setupMacroGroupEvents(container) {
    container.querySelectorAll('.macro-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.parentElement.classList.toggle('collapsed');
      });
    });
  }

  function setupMacroKnobEvents(container) {
    container.querySelectorAll('.macro-knob').forEach(function(knob) {
      var cell = knob.closest('.macro-knob-cell');
      var valueEl = cell.querySelector('.macro-knob-value');
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var startY = 0;
      var startVal = 0;

      function onPointerDown(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;
        document.addEventListener('pointermove', onPointerMove);
        document.addEventListener('pointerup', onPointerUp);
      }

      function onPointerMove(e) {
        var dy = startY - e.clientY; // up = increase
        var range = max - min;
        var sensitivity = range / 200; // 200px drag = full range
        var newVal = Math.round(startVal + dy * sensitivity);
        newVal = Math.max(min, Math.min(max, newVal));

        knob.setAttribute('data-value', newVal);
        var pct = Math.round(((newVal - min) / (max - min)) * 100);
        knob.style.setProperty('--knob-pct', pct);
        valueEl.textContent = newVal;

        // Update indicator position
        var indicator = knob.querySelector('.knob-indicator');
        if (indicator) indicator.style.cssText = knobIndicatorStyle(pct);
      }

      function onPointerUp(e) {
        knob.classList.remove('dragging');
        document.removeEventListener('pointermove', onPointerMove);
        document.removeEventListener('pointerup', onPointerUp);

        // Send value to device
        var groupIdx = parseInt(cell.getAttribute('data-group'), 10);
        var paramIdx = parseInt(cell.getAttribute('data-param'), 10);
        var value = parseInt(knob.getAttribute('data-value'), 10);
        sendMacroValue(state.activeTrack, groupIdx, paramIdx, value);
      }

      knob.addEventListener('pointerdown', onPointerDown);
    });
  }

  function sendMacroValue(trackIdx, groupIdx, paramIdx, value) {
    // In production, this sends to /api/v1/macroapi
    // For prototype, just log it
    console.log('Macro value: track=' + trackIdx + ' group=' + groupIdx +
                ' param=' + paramIdx + ' value=' + value);

    // TODO: POST to /api/v1/macroapi/tracks/{track}/parameters
    // Body: { group: groupIdx, param: paramIdx, value: value }
  }

  // ─── Preset Browser ─────────────────────────────────────

  function renderPresetBrowser() {
    var container = document.getElementById('preset-list');
    if (!container) return;

    var presets = generateMockPresets();
    var term = state.presetSearchTerm.toLowerCase();
    var html = '';

    Object.keys(presets).forEach(function(category) {
      var items = presets[category];
      if (term) {
        items = items.filter(function(p) {
          return p.name.toLowerCase().indexOf(term) !== -1;
        });
      }
      if (items.length === 0) return;

      html += '<div class="preset-category">' + S.esc(category) + '</div>';
      items.forEach(function(p) {
        html += '<div class="preset-item" data-preset="' + S.esc(p.name) + '">';
        html += '<span class="preset-item-name">' + S.esc(p.name) + '</span>';
        html += '<span class="preset-item-machine">' + S.esc(p.machine.replace('Rack', '')) + '</span>';
        html += '</div>';
      });
    });

    if (!html) {
      html = '<div class="empty-state" style="padding:1.5rem;"><p style="font-size:0.78rem;">No presets found</p></div>';
    }

    container.innerHTML = html;
  }

  function setupPresetBrowserEvents() {
    var search = document.getElementById('preset-search');
    if (search) {
      search.addEventListener('sl-input', function() {
        state.presetSearchTerm = search.value || '';
        renderPresetBrowser();
      });
      search.addEventListener('sl-clear', function() {
        state.presetSearchTerm = '';
        renderPresetBrowser();
      });
    }

    var list = document.getElementById('preset-list');
    if (list) {
      list.addEventListener('click', function(e) {
        var item = e.target.closest('.preset-item');
        if (!item) return;
        var presetName = item.getAttribute('data-preset');

        // Mark active
        list.querySelectorAll('.preset-item').forEach(function(p) {
          p.classList.remove('active');
        });
        item.classList.add('active');

        loadPreset(presetName);
      });
    }
  }

  function loadPreset(presetName) {
    console.log('Loading preset:', presetName);
    S.toast('Loaded preset: ' + presetName, 'success', 2000);

    // In production: POST to /api/v1/macroapi to load the sound preset
    // Then refresh the macro controls with new values
    if (state.activeTrack >= 0) {
      var track = state.tracks[state.activeTrack];
      var macroDef = generateMockMacroDefinition(track.name, track.machine);
      // Randomize values to simulate preset loading
      macroDef.groups.forEach(function(g) {
        g.params.forEach(function(p) {
          p.value = Math.floor(Math.random() * 128);
        });
      });
      state.currentMacroDef = macroDef;
      renderMacroControls(track, macroDef);
    }
  }

  // ─── Favorites ───────────────────────────────────────────

  function renderFavorites() {
    var container = document.getElementById('fav-row');
    if (!container) return;

    var html = '';
    for (var i = 0; i < 10; i++) {
      html += '<button class="fav-btn" data-fav="' + i + '" title="Favorite ' + (i + 1) + '">';
      html += (i + 1);
      html += '</button>';
    }
    container.innerHTML = html;
  }

  function setupFavoritesEvents() {
    var container = document.getElementById('fav-row');
    if (!container) return;

    container.addEventListener('click', function(e) {
      var btn = e.target.closest('.fav-btn');
      if (!btn) return;
      var idx = parseInt(btn.getAttribute('data-fav'), 10);
      S.toast('Favorite ' + (idx + 1) + ' — recall/store not yet wired', 'primary', 2000);
    });
  }

  // ─── Quick Actions ───────────────────────────────────────

  function setupQuickActions() {
    var muteBtn = document.getElementById('qa-mute-track');
    if (muteBtn) {
      muteBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) return;
        var track = state.tracks[state.activeTrack];
        track.muted = !track.muted;
        renderTrackOverview();
        S.toast(track.name + (track.muted ? ' muted' : ' unmuted'), 'primary', 1500);
      });
    }

    var soloBtn = document.getElementById('qa-solo-track');
    if (soloBtn) {
      soloBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) return;
        var track = state.tracks[state.activeTrack];
        track.solo = !track.solo;
        S.toast(track.name + (track.solo ? ' solo ON' : ' solo OFF'), 'primary', 1500);
      });
    }

    var randomizeBtn = document.getElementById('qa-randomize');
    if (randomizeBtn) {
      randomizeBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        var track = state.tracks[state.activeTrack];
        var macroDef = generateMockMacroDefinition(track.name, track.machine);
        state.currentMacroDef = macroDef;
        renderMacroControls(track, macroDef);
        S.toast('Randomized ' + track.name, 'success', 1500);
      });
    }

    var initBtn = document.getElementById('qa-init');
    if (initBtn) {
      initBtn.addEventListener('click', function() {
        if (state.activeTrack < 0) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        var track = state.tracks[state.activeTrack];
        var macroDef = generateMockMacroDefinition(track.name, track.machine);
        // Reset all values to midpoint
        macroDef.groups.forEach(function(g) {
          g.params.forEach(function(p) {
            p.value = Math.floor((p.max - p.min) / 2);
          });
        });
        state.currentMacroDef = macroDef;
        renderMacroControls(track, macroDef);
        S.toast('Initialized ' + track.name, 'success', 1500);
      });
    }

    var saveBtn = document.getElementById('qa-save-preset');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        S.toast('Save preset — coming soon', 'primary', 2000);
      });
    }

    var exportBtn = document.getElementById('qa-export');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        S.toast('Export all — coming soon', 'primary', 2000);
      });
    }

    var importBtn = document.getElementById('qa-import');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        S.toast('Import all — coming soon', 'primary', 2000);
      });
    }
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    state.tracks = generateMockTracks();
    renderTrackOverview();
    setupTrackOverviewEvents();
    renderPresetBrowser();
    setupPresetBrowserEvents();
    renderFavorites();
    setupFavoritesEvents();
    setupQuickActions();

    // Auto-select first track
    selectTrack(0);

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.performer = {
    init: init,
    state: state,
  };

})();
