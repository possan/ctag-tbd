// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Performer View
// Persona: Performer / Artist
//
// Uses shared data from shared.js (synthdefs, macrodefs, soundpresets).
// Shared track tabs handle track selection — this view handles:
//   - Macro knob display for the active track
//   - Sound preset browser (left sidebar)
//   - Quick actions (right panel)
//
// Data flow:
//   Track Selection → Machine selection → Macro Def → Knobs
//   Sound Presets → Load knob values
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    activeTrack: -1,         // Currently selected track index
    activeMachine: '',       // Active machine id for selected track
    activeMacroDef: null,    // Active macro definition object
    activePreset: null,      // Active sound preset object
    paramValues: [],         // Current parameter values for active macro/preset
    presetSearchTerm: '',
    initialized: false,
  };

  // ─── API Helpers ──────────────────────────────────────────

  function apiGet(url) {
    return fetch(url).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  function apiPost(url, data) {
    return fetch(url, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data),
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      return r.json();
    });
  }

  // ─── Track Selection (driven by shared track tabs) ────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.activePreset = null;

    // Get available machines for this track
    var availMachines = S.getTrackMachines(track);

    // Default to first available machine
    var machineId = availMachines.length > 0 ? availMachines[0] : '';
    state.activeMachine = machineId;

    // Find matching macro definitions for this machine
    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === machineId;
    });

    // Prefer "allparams" definition, fallback to first
    var allParamsDef = matchingDefs.find(function(d) {
      return d.id.indexOf('allparams') !== -1;
    });
    var def = allParamsDef || matchingDefs[0] || null;
    state.activeMacroDef = def;

    // Initialize param values from macro def defaults
    state.paramValues = [];
    if (def && def.groups) {
      def.groups.forEach(function(group) {
        group.parameters.forEach(function(param) {
          state.paramValues[param.idx] = param.def || 0;
        });
      });
    }

    renderMacroControls(track, def, availMachines);
    renderPresetBrowser();
  }

  // ─── Macro Controls Rendering ────────────────────────────

  function renderMacroControls(track, macroDef, availMachines) {
    var container = document.getElementById('macro-controls');
    if (!container) return;

    if (!macroDef) {
      container.innerHTML =
        '<div class="empty-state" id="macro-empty">' +
        '<sl-icon name="sliders"></sl-icon>' +
        '<h3>No macro definition found</h3>' +
        '<p>No macro definition available for machine "' + S.esc(state.activeMachine) + '"</p>' +
        '</div>';
      return;
    }

    var html = '';

    // Track info header with CLEAR labels
    html += '<div class="track-info-header">';
    html += '<span class="track-badge">CH ' + String(track.index + 1).padStart(2, '0') + '</span>';
    html += '<span class="track-title">' + S.esc(track.name) + '</span>';

    // Label: "Machine:" for the machine selector
    html += '<span class="track-info-label">Machine:</span>';
    html += '<sl-select id="performer-machine-select" size="small" value="' + S.esc(state.activeMachine) + '" style="min-width:140px;">';
    availMachines.forEach(function(machId) {
      var info = S.getMachineInfo(machId);
      var label = info ? info.name : machId;
      html += '<sl-option value="' + S.esc(machId) + '">' + S.esc(label) + '</sl-option>';
    });
    html += '</sl-select>';

    // Label: "Knob Set:" for the macro definition selector (if multiple)
    var matchingDefs = S.data.macroDefs.filter(function(d) {
      return d.machine === state.activeMachine;
    });
    if (matchingDefs.length > 1) {
      html += '<span class="track-info-label">Macro Def:</span>';
      html += '<sl-select id="performer-macrodef-select" size="small" value="' + S.esc(macroDef.id) + '" style="min-width:160px;">';
      matchingDefs.forEach(function(d) {
        html += '<sl-option value="' + S.esc(d.id) + '">' + S.esc(d.name) + '</sl-option>';
      });
      html += '</sl-select>';
    } else {
      html += '<span class="track-subtitle">' + S.esc(macroDef.name) + '</span>';
    }
    html += '</div>';

    // Render each macro group (knob pages)
    var mappingInfo = S.analyzeMappings(macroDef);

    if (macroDef.groups) {
      macroDef.groups.forEach(function(group, gi) {
        if (!group.parameters || group.parameters.length === 0) return;

        html += '<div class="macro-group" data-group="' + gi + '">';

        // Group header
        html += '<div class="macro-group-header">';
        html += '<sl-icon name="chevron-down" class="macro-group-chevron"></sl-icon>';
        html += '<span class="macro-group-name">' + S.esc(group.name) + '</span>';
        html += '</div>';

        // Group body with knob grid (4 columns)
        html += '<div class="macro-group-body">';
        group.parameters.forEach(function(param) {
          var value = state.paramValues[param.idx] !== undefined ? state.paramValues[param.idx] : (param.def || 0);
          var min = param.min || 0;
          var max = param.max || 127;
          var isMacro = S.isMacroKnob(mappingInfo, param.idx);
          var knobColor = isMacro ? 'macro' : 'normal';
          var cellClass = 'macro-knob-cell' + (isMacro ? ' is-macro' : '');

          html += '<div class="' + cellClass + '" data-param-idx="' + param.idx + '">';
          html += '<div class="macro-knob" ';
          html += 'data-value="' + value + '" data-min="' + min + '" data-max="' + max + '" data-idx="' + param.idx + '" data-color="' + knobColor + '">';
          html += S.renderKnobSVG({ value: value, min: min, max: max, color: knobColor, size: 52 });
          html += '</div>';
          html += '<span class="macro-knob-label">' + S.esc(param.name) + '</span>';
          html += '<span class="macro-knob-value">' + value + '</span>';

          // Show mapping targets with real-time computed values
          var targets = mappingInfo[param.idx] || [];
          if (targets.length > 0) {
            var outputs = S.computeMappingOutputs(macroDef, param.idx, value);
            html += '<div class="knob-target-panel' + (isMacro ? ' is-macro' : '') + '" data-knob-idx="' + param.idx + '">';
            if (isMacro) {
              html += '<div class="knob-target-badge">MACRO</div>';
            }
            outputs.forEach(function(o) {
              html += '<div class="knob-target-row" data-ctrl="' + o.ctrl + '">';
              html += '<span class="knob-target-name">' + S.esc(o.name) + '</span>';
              html += '<span class="knob-target-bar"><span class="knob-target-fill" style="width:' + o.pct + '%"></span></span>';
              html += '<span class="knob-target-val">' + o.value + '</span>';
              html += '</div>';
            });
            html += '</div>';
          }
          // Show curve indicator if non-linear
          if (param.curve && param.curve !== 'linear') {
            html += '<span class="curve-badge">' + S.esc(param.curve) + '</span>';
          }

          html += '</div>';
        });
        html += '</div>';

        html += '</div>';
      });
    }

    container.innerHTML = html;
    setupMacroKnobEvents(container);
    setupMacroGroupEvents(container);
    setupMachineChangeEvents();
  }

  function setupMachineChangeEvents() {
    var machineSelect = document.getElementById('performer-machine-select');
    if (machineSelect) {
      machineSelect.addEventListener('sl-change', function() {
        var newMachine = machineSelect.value;
        if (newMachine === state.activeMachine) return;
        state.activeMachine = newMachine;

        // Find matching macro definitions for new machine
        var matchingDefs = S.data.macroDefs.filter(function(d) {
          return d.machine === newMachine;
        });
        var allParamsDef = matchingDefs.find(function(d) {
          return d.id.indexOf('allparams') !== -1;
        });
        var def = allParamsDef || matchingDefs[0] || null;
        state.activeMacroDef = def;

        // Reset param values
        state.paramValues = [];
        if (def && def.groups) {
          def.groups.forEach(function(group) {
            group.parameters.forEach(function(param) {
              state.paramValues[param.idx] = param.def || 0;
            });
          });
        }

        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        var availMachines = S.getTrackMachines(track);
        renderMacroControls(track, def, availMachines);
        renderPresetBrowser();

        sendTrackUpdate({ track: state.activeTrack, machine: newMachine });
      });
    }

    var macroDefSelect = document.getElementById('performer-macrodef-select');
    if (macroDefSelect) {
      macroDefSelect.addEventListener('sl-change', function() {
        var defId = macroDefSelect.value;
        var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
        if (!def) return;
        state.activeMacroDef = def;

        state.paramValues = [];
        if (def.groups) {
          def.groups.forEach(function(group) {
            group.parameters.forEach(function(param) {
              state.paramValues[param.idx] = param.def || 0;
            });
          });
        }

        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        var availMachines = S.getTrackMachines(track);
        renderMacroControls(track, def, availMachines);
        renderPresetBrowser();

        sendTrackUpdate({ track: state.activeTrack, macro: def.id });
      });
    }
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
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
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
        var dy = startY - e.clientY;
        var range = max - min;
        var sensitivity = range / 200;
        var newVal = Math.round(startVal + dy * sensitivity);
        newVal = Math.max(min, Math.min(max, newVal));

        knob.setAttribute('data-value', newVal);
        valueEl.textContent = newVal;
        state.paramValues[paramIdx] = newVal;

        // Re-render the SVG knob with correct color
        var knobColor = knob.getAttribute('data-color') || 'normal';
        knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: knobColor, size: 52 });

        // Update target panel real-time values
        if (state.activeMacroDef) {
          var panel = cell.querySelector('.knob-target-panel');
          if (panel) {
            var outputs = S.computeMappingOutputs(state.activeMacroDef, paramIdx, newVal);
            outputs.forEach(function(o) {
              var row = panel.querySelector('.knob-target-row[data-ctrl="' + o.ctrl + '"]');
              if (!row) return;
              var valEl = row.querySelector('.knob-target-val');
              var fillEl = row.querySelector('.knob-target-fill');
              if (valEl) valEl.textContent = o.value;
              if (fillEl) fillEl.style.width = o.pct + '%';
            });
          }
        }
      }

      function onPointerUp() {
        knob.classList.remove('dragging');
        document.removeEventListener('pointermove', onPointerMove);
        document.removeEventListener('pointerup', onPointerUp);

        var value = parseInt(knob.getAttribute('data-value'), 10);
        state.paramValues[paramIdx] = value;
        sendParameterUpdate();
      }

      knob.addEventListener('pointerdown', onPointerDown);
    });
  }

  // ─── API: Send Updates ────────────────────────────────────

  function sendTrackUpdate(body) {
    apiPost('/api/v1/macroapi?action=update_track', body).then(function() {
      console.log('[Performer] Track update sent:', body);
    }).catch(function(err) {
      console.error('[Performer] Track update failed:', err);
    });
  }

  function sendParameterUpdate() {
    if (state.activeTrack < 0 || !state.activeMacroDef) return;

    var body = {
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: state.activeMacroDef.id,
      parameters: state.paramValues.slice(),
    };

    apiPost('/api/v1/macroapi?action=update_track', body).then(function() {
      console.log('[Performer] Parameters sent for track', state.activeTrack);
    }).catch(function(err) {
      console.error('[Performer] Parameter send failed:', err);
    });
  }

  // ─── Preset Browser ─────────────────────────────────────

  function renderPresetBrowser() {
    var container = document.getElementById('preset-list');
    if (!container) return;

    // Filter presets for the active machine
    var matchingDefIds = {};
    S.data.macroDefs.forEach(function(d) {
      if (d.machine === state.activeMachine) {
        matchingDefIds[d.id] = true;
      }
    });

    var presets = S.data.soundPresets.filter(function(p) {
      return matchingDefIds[p.macro] || false;
    });

    // Apply search filter
    var term = state.presetSearchTerm.toLowerCase();
    if (term) {
      presets = presets.filter(function(p) {
        return (p.name || '').toLowerCase().indexOf(term) !== -1 ||
               (p.group || '').toLowerCase().indexOf(term) !== -1;
      });
    }

    // Group by `group` field
    var groups = {};
    presets.forEach(function(p) {
      var g = p.group || 'Uncategorized';
      if (!groups[g]) groups[g] = [];
      groups[g].push(p);
    });

    var html = '';
    Object.keys(groups).sort().forEach(function(groupName) {
      html += '<div class="preset-category">' + S.esc(groupName) + '</div>';
      groups[groupName].forEach(function(p) {
        var isActive = state.activePreset && state.activePreset.id === p.id;
        html += '<div class="preset-item' + (isActive ? ' active' : '') + '" data-preset-id="' + S.esc(p.id) + '">';
        html += '<span class="preset-item-name">' + S.esc(p.name) + '</span>';
        html += '<span class="preset-item-machine">' + S.esc(p.macro) + '</span>';
        html += '</div>';
      });
    });

    if (!html) {
      html = '<div class="empty-state" style="padding:1.5rem;">';
      html += '<p style="font-size:0.78rem;">No presets for ' + S.esc(state.activeMachine || 'this track') + '</p>';
      html += '</div>';
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
        var presetId = item.getAttribute('data-preset-id');
        loadPreset(presetId);
      });
    }
  }

  function loadPreset(presetId) {
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) return;

    state.activePreset = preset;

    var def = S.data.macroDefs.find(function(d) { return d.id === preset.macro; });
    if (def) {
      state.activeMacroDef = def;
      state.activeMachine = def.machine;
    }

    if (preset.values && preset.values.length > 0) {
      state.paramValues = preset.values.slice();
    }

    document.querySelectorAll('.preset-item').forEach(function(p) {
      p.classList.toggle('active', p.getAttribute('data-preset-id') === presetId);
    });

    var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
    if (track) {
      var availMachines = S.getTrackMachines(track);
      renderMacroControls(track, state.activeMacroDef, availMachines);
    }

    sendTrackUpdate({
      track: state.activeTrack,
      machine: state.activeMachine,
      macro: preset.macro,
      parameters: state.paramValues.slice(),
    });

    S.toast('Loaded: ' + preset.name, 'success', 2000);
  }

  // ─── Quick Actions ───────────────────────────────────────

  function setupQuickActions() {
    var randomizeBtn = document.getElementById('qa-randomize');
    if (randomizeBtn) {
      randomizeBtn.addEventListener('click', function() {
        if (state.activeTrack < 0 || !state.activeMacroDef) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        if (state.activeMacroDef.groups) {
          state.activeMacroDef.groups.forEach(function(group) {
            group.parameters.forEach(function(param) {
              var min = param.min || 0;
              var max = param.max || 127;
              state.paramValues[param.idx] = Math.floor(Math.random() * (max - min + 1)) + min;
            });
          });
        }
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        var availMachines = S.getTrackMachines(track);
        renderMacroControls(track, state.activeMacroDef, availMachines);
        sendParameterUpdate();
        S.toast('Randomized ' + track.name, 'success', 1500);
      });
    }

    var initBtn = document.getElementById('qa-init');
    if (initBtn) {
      initBtn.addEventListener('click', function() {
        if (state.activeTrack < 0 || !state.activeMacroDef) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        if (state.activeMacroDef.groups) {
          state.activeMacroDef.groups.forEach(function(group) {
            group.parameters.forEach(function(param) {
              state.paramValues[param.idx] = param.def || 0;
            });
          });
        }
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        var availMachines = S.getTrackMachines(track);
        renderMacroControls(track, state.activeMacroDef, availMachines);
        sendParameterUpdate();
        S.toast('Initialized ' + track.name, 'success', 1500);
      });
    }

    var saveBtn = document.getElementById('qa-save-preset');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        if (state.activeTrack < 0 || !state.activeMacroDef) {
          S.toast('Select a track first', 'warning', 2000);
          return;
        }
        savePresetDialog();
      });
    }

    var muteBtn = document.getElementById('qa-mute-track');
    if (muteBtn) {
      muteBtn.addEventListener('click', function() {
        S.toast('Mute — requires device connection', 'primary', 2000);
      });
    }

    var soloBtn = document.getElementById('qa-solo-track');
    if (soloBtn) {
      soloBtn.addEventListener('click', function() {
        S.toast('Solo — requires device connection', 'primary', 2000);
      });
    }

    var exportBtn = document.getElementById('qa-export');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        exportAllPresets();
      });
    }

    var importBtn = document.getElementById('qa-import');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        importPresetFile();
      });
    }
  }

  // ─── Save Preset Dialog ──────────────────────────────────

  function savePresetDialog() {
    var name = prompt('Preset name:', state.activePreset ? state.activePreset.name : state.activeMacroDef.name);
    if (!name) return;

    var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-');
    var group = prompt('Preset group:', state.activePreset ? state.activePreset.group : state.activeMachine);
    if (group === null) return;

    var preset = {
      id: id,
      name: name,
      group: group || 'User',
      macro: state.activeMacroDef.id,
      values: state.paramValues.slice(),
    };

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = 'macrosoundpresets/' + id + '.json';

    fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved preset: ' + name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Export / Import ──────────────────────────────────────

  function exportAllPresets() {
    var data = {
      macroDefs: S.data.macroDefs,
      soundPresets: S.data.soundPresets,
    };
    var blob = new Blob([JSON.stringify(data, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = 'tbd16-presets-export.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported all presets', 'success', 2000);
  }

  function importPresetFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.id && data.macro) {
            importSinglePreset(data);
          } else if (data.soundPresets) {
            S.toast('Bulk import — coming soon', 'primary', 2000);
          } else {
            S.toast('Unrecognized JSON format', 'warning', 3000);
          }
        } catch (err) {
          S.toast('Invalid JSON: ' + err.message, 'danger', 3000);
        }
      };
      reader.readAsText(input.files[0]);
    });
    input.click();
  }

  function importSinglePreset(preset) {
    var filePath = 'macrosoundpresets/' + preset.id + '.json';
    var jsonStr = JSON.stringify(preset, null, 2);

    fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Imported: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderPresetBrowser();
    }).catch(function(err) {
      S.toast('Import failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    // Register for shared track selection events
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupPresetBrowserEvents();
    setupQuickActions();

    // Auto-select first track if data is already loaded
    if (S.data.loaded && S.data.tracks.length > 0) {
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.performer = {
    init: init,
    state: state,
    reload: function() {
      if (state.activeTrack >= 0) {
        var track = S.data.tracks.find(function(t) { return t.index === state.activeTrack; });
        if (track) onTrackSelected(state.activeTrack, track);
      }
    },
  };

})();
