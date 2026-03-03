// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Sound Designer View
// Persona: Sound Designer
//
// Focused on creating machine definitions and mapping DSP params
// to macro controls. Shows the mapping layer: macro param → DSP CC,
// multiplier/divider, and the raw DSP parameter browser.
//
// Workflow:
//   1. Select a track / or start from scratch
//   2. Choose a DSP machine plugin from the browser
//   3. Define output mappings (macro param → DSP CC)
//   4. Set up 6 groups × 4 macro params with labels + ranges
//   5. Save as machine definition JSON
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    machineDefinitions: [],       // Available machine definitions
    selectedMachineDef: null,     // Currently edited definition
    dspPlugins: [],               // Available DSP plugins
    selectedPlugin: null,         // Currently loaded plugin
    dspParams: [],                // Current plugin's DSP params
    mappings: [],                 // Active mapping table rows
    selectedTrack: 0,
    initialized: false,
  };

  // ─── Mock Data ────────────────────────────────────────────

  function generateMockDSPPlugins() {
    return [
      { id: 'DrumBDamp',    category: 'Drums',   desc: 'Bass Drum with damping' },
      { id: 'DrumSD',       category: 'Drums',   desc: 'Snare Drum synthesis' },
      { id: 'DrumHiHat',    category: 'Drums',   desc: 'Hi-Hat metallic synthesis' },
      { id: 'CDelay',       category: 'Effects', desc: 'Configurable delay' },
      { id: 'Reverb',       category: 'Effects', desc: 'Plate reverb' },
      { id: 'WTOsc',        category: 'Synths',  desc: 'Wavetable oscillator' },
      { id: 'SubOsc',       category: 'Synths',  desc: 'Sub-oscillator with filter' },
      { id: 'Rompler',      category: 'Sampler', desc: 'ROM sample playback' },
      { id: 'Granular',     category: 'Sampler', desc: 'Granular sample playback' },
      { id: 'PicoSeqRack',  category: 'System',  desc: '16-track rack sequencer' },
    ];
  }

  function generateMockDSPParams(pluginId) {
    var paramSets = {
      'DrumBDamp': [
        'pitch', 'tune', 'decay', 'noise_level', 'noise_decay', 'fm_amount',
        'fm_decay', 'drive', 'shape', 'attack', 'sustain', 'release',
        'filter_freq', 'filter_reso', 'filter_env', 'volume',
        'pan', 'fx_send_1', 'fx_send_2', 'trigger_mode',
      ],
      'DrumSD': [
        'tune', 'snappy', 'decay', 'noise_color', 'noise_decay', 'drive',
        'tone', 'ring_freq', 'ring_amount', 'filter_freq', 'filter_reso',
        'attack', 'sustain', 'volume', 'pan', 'fx_send_1',
      ],
      'DrumHiHat': [
        'freq_hi', 'freq_lo', 'tone', 'noise_mix', 'decay_closed', 'decay_open',
        'metallic', 'ring', 'saturate', 'bit_reduce', 'filter_freq',
        'attack', 'choke', 'volume', 'pan', 'fx_send_1',
      ],
      'WTOsc': [
        'wave_pos', 'morph', 'detune', 'sub_level', 'sub_octave', 'fm_depth',
        'filter_freq', 'filter_reso', 'filter_type', 'filter_env',
        'env_attack', 'env_decay', 'env_sustain', 'env_release',
        'lfo_rate', 'lfo_depth', 'lfo_dest', 'glide',
        'volume', 'pan', 'fx_send_1', 'fx_send_2',
      ],
    };

    var names = paramSets[pluginId] || [
      'param_0', 'param_1', 'param_2', 'param_3',
      'param_4', 'param_5', 'param_6', 'param_7',
      'param_8', 'param_9', 'param_10', 'param_11',
      'volume', 'pan', 'fx_send_1', 'fx_send_2',
    ];

    return names.map(function(name, i) {
      return {
        index: i,
        name: name,
        value: Math.floor(Math.random() * 4096),
        min: 0,
        max: 4095,
        isCurrent: false,
      };
    });
  }

  function generateMockMachineDefinitions() {
    return [
      {
        name: 'RackDBD',
        plugin: 'DrumBDamp',
        description: 'Bass drum with 6×4 macro controls',
        groups: 6,
        mappedParams: 24,
      },
      {
        name: 'RackDSD',
        plugin: 'DrumSD',
        description: 'Snare drum rack definition',
        groups: 6,
        mappedParams: 16,
      },
      {
        name: 'RackHH1',
        plugin: 'DrumHiHat',
        description: 'Hi-hat with open/close control',
        groups: 6,
        mappedParams: 16,
      },
      {
        name: 'RackTBD03',
        plugin: 'WTOsc',
        description: '303-style acid synth rack',
        groups: 6,
        mappedParams: 22,
      },
    ];
  }

  function generateMockMappings(defName) {
    var mappings;
    if (defName === 'RackDBD') {
      mappings = [
        { group: 0, slot: 0, macroName: 'Frequency', dspParam: 'pitch',       mul: 1.0, div: 1.0 },
        { group: 0, slot: 1, macroName: 'Tone',      dspParam: 'tone',        mul: 1.0, div: 1.0 },
        { group: 0, slot: 2, macroName: 'Decay',     dspParam: 'decay',       mul: 1.0, div: 1.0 },
        { group: 0, slot: 3, macroName: 'Noise',     dspParam: 'noise_level', mul: 0.5, div: 1.0 },
        { group: 1, slot: 0, macroName: 'Drive',     dspParam: 'drive',       mul: 1.0, div: 1.0 },
        { group: 1, slot: 1, macroName: 'Shape',     dspParam: 'shape',       mul: 1.0, div: 1.0 },
        { group: 1, slot: 2, macroName: 'Dirty',     dspParam: 'fm_amount',   mul: 0.7, div: 1.0 },
        { group: 1, slot: 3, macroName: 'FM Decay',  dspParam: 'fm_decay',    mul: 1.0, div: 1.0 },
        { group: 2, slot: 0, macroName: 'Level',     dspParam: 'volume',      mul: 1.0, div: 1.0 },
        { group: 2, slot: 1, macroName: 'Accent',    dspParam: 'attack',      mul: 1.0, div: 2.0 },
        { group: 2, slot: 2, macroName: 'Attack',    dspParam: 'attack',      mul: 1.0, div: 1.0 },
        { group: 2, slot: 3, macroName: 'Sustain',   dspParam: 'sustain',     mul: 1.0, div: 1.0 },
      ];
    } else {
      mappings = [
        { group: 0, slot: 0, macroName: 'Param A', dspParam: 'param_0', mul: 1.0, div: 1.0 },
        { group: 0, slot: 1, macroName: 'Param B', dspParam: 'param_1', mul: 1.0, div: 1.0 },
        { group: 0, slot: 2, macroName: 'Param C', dspParam: 'param_2', mul: 1.0, div: 1.0 },
        { group: 0, slot: 3, macroName: 'Param D', dspParam: 'param_3', mul: 1.0, div: 1.0 },
      ];
    }
    return mappings;
  }

  // ─── Track Select ────────────────────────────────────────

  function renderTrackSelect() {
    var select = document.getElementById('designer-track-select');
    if (!select) return;

    var html = '';
    for (var i = 0; i < 16; i++) {
      html += '<sl-option value="' + i + '">Track ' + (i + 1) + '</sl-option>';
    }
    select.innerHTML = html;
  }

  function setupTrackSelectEvents() {
    var select = document.getElementById('designer-track-select');
    if (!select) return;

    select.addEventListener('sl-change', function() {
      state.selectedTrack = parseInt(select.value, 10);
      // Optionally reload definition / plugin for this track
    });
  }

  // ─── Machine Definition List ─────────────────────────────

  function renderMachineDefList() {
    var container = document.getElementById('machine-def-list');
    if (!container) return;

    var defs = state.machineDefinitions;
    var html = '';

    defs.forEach(function(def) {
      var isActive = state.selectedMachineDef && state.selectedMachineDef.name === def.name;
      html += '<div class="machine-item' + (isActive ? ' active' : '') + '" data-def="' + S.esc(def.name) + '">';
      html += '<div class="machine-item-name">' + S.esc(def.name) + '</div>';
      html += '<div class="machine-item-desc">' + S.esc(def.description) + '</div>';
      html += '<div class="machine-item-meta">';
      html += '<span>Plugin: ' + S.esc(def.plugin) + '</span>';
      html += '<span>' + def.mappedParams + ' mapped</span>';
      html += '</div>';
      html += '</div>';
    });

    if (!html) {
      html = '<div class="empty-state"><p>No machine definitions found</p></div>';
    }

    container.innerHTML = html;
  }

  function setupMachineDefListEvents() {
    var container = document.getElementById('machine-def-list');
    if (!container) return;

    container.addEventListener('click', function(e) {
      var item = e.target.closest('.machine-item');
      if (!item) return;
      var defName = item.getAttribute('data-def');
      selectMachineDefinition(defName);
    });
  }

  function selectMachineDefinition(defName) {
    var def = state.machineDefinitions.find(function(d) { return d.name === defName; });
    if (!def) return;

    state.selectedMachineDef = def;
    state.mappings = generateMockMappings(defName);
    state.dspParams = generateMockDSPParams(def.plugin);
    state.selectedPlugin = def.plugin;

    // Update machine def select
    var select = document.getElementById('designer-machinedef-select');
    if (select) {
      select.value = defName;
    }

    // Update active states
    document.querySelectorAll('.machine-item').forEach(function(item) {
      item.classList.toggle('active', item.getAttribute('data-def') === defName);
    });

    renderMappingEditor();
    renderDSPParams();
  }

  // ─── Machine Definition Select (toolbar) ─────────────────

  function renderMachineDefSelect() {
    var select = document.getElementById('designer-machinedef-select');
    if (!select) return;

    var html = '<sl-option value="">— Select Machine —</sl-option>';
    state.machineDefinitions.forEach(function(def) {
      html += '<sl-option value="' + S.esc(def.name) + '">' + S.esc(def.name) + '</sl-option>';
    });
    select.innerHTML = html;

    select.addEventListener('sl-change', function() {
      if (select.value) {
        selectMachineDefinition(select.value);
      }
    });
  }

  // ─── Mapping Editor ──────────────────────────────────────

  function renderMappingEditor() {
    var container = document.getElementById('mapping-editor-body');
    if (!container) return;

    if (!state.selectedMachineDef) {
      container.innerHTML =
        '<div class="empty-state">' +
        '<sl-icon name="git-branch" style="font-size:2rem;opacity:0.3;"></sl-icon>' +
        '<p>Select a machine definition to edit mappings</p>' +
        '</div>';
      return;
    }

    // Update title
    var title = document.getElementById('mapping-editor-title');
    if (title) {
      title.textContent = state.selectedMachineDef.name + ' — Output Mappings';
    }

    // Group mappings by group index
    var groupedMappings = {};
    state.mappings.forEach(function(m) {
      if (!groupedMappings[m.group]) groupedMappings[m.group] = [];
      groupedMappings[m.group].push(m);
    });

    var html = '';

    // Build DSP param options for dropdowns
    var dspOptions = '<option value="">—</option>';
    state.dspParams.forEach(function(p) {
      dspOptions += '<option value="' + S.esc(p.name) + '">' + S.esc(p.name) + ' [' + p.index + ']</option>';
    });

    Object.keys(groupedMappings).sort().forEach(function(gIdx) {
      var group = groupedMappings[gIdx];
      var groupNames = ['Tone', 'Shape', 'Dynamics', 'Modulation', 'Effects', 'Mix'];
      var groupName = groupNames[gIdx] || 'Group ' + (parseInt(gIdx) + 1);

      html += '<div class="mapping-group">';
      html += '<div class="mapping-group-title">';
      html += '<span>Group ' + (parseInt(gIdx) + 1) + ': ' + S.esc(groupName) + '</span>';
      html += '<button class="mapping-add-btn" data-group="' + gIdx + '" title="Add mapping">';
      html += '<sl-icon name="plus-circle"></sl-icon>';
      html += '</button>';
      html += '</div>';

      html += '<table class="mapping-table">';
      html += '<thead><tr>';
      html += '<th>Slot</th>';
      html += '<th>Macro Name</th>';
      html += '<th>DSP Parameter</th>';
      html += '<th>Mul</th>';
      html += '<th>Div</th>';
      html += '<th></th>';
      html += '</tr></thead>';
      html += '<tbody>';

      group.forEach(function(m, i) {
        html += '<tr class="mapping-row" data-group="' + m.group + '" data-slot="' + m.slot + '">';
        html += '<td class="mapping-slot">' + (m.group * 4 + m.slot + 1) + '</td>';
        html += '<td><input class="mapping-input mapping-name" value="' + S.esc(m.macroName) + '" /></td>';
        html += '<td><select class="mapping-select mapping-dsp-param">';
        // Re-build options with selection
        html += '<option value="">—</option>';
        state.dspParams.forEach(function(p) {
          var sel = (p.name === m.dspParam) ? ' selected' : '';
          html += '<option value="' + S.esc(p.name) + '"' + sel + '>' + S.esc(p.name) + ' [' + p.index + ']</option>';
        });
        html += '</select></td>';
        html += '<td><input class="mapping-input mapping-mul" type="number" step="0.1" min="0" max="10" value="' + m.mul + '" /></td>';
        html += '<td><input class="mapping-input mapping-div" type="number" step="0.1" min="0.1" max="10" value="' + m.div + '" /></td>';
        html += '<td><button class="mapping-remove-btn" title="Remove mapping"><sl-icon name="x-circle"></sl-icon></button></td>';
        html += '</tr>';
      });

      // Empty slots (if group has <4 mappings)
      for (var s = group.length; s < 4; s++) {
        html += '<tr class="mapping-row mapping-row-empty" data-group="' + gIdx + '" data-slot="' + s + '">';
        html += '<td class="mapping-slot">' + (parseInt(gIdx) * 4 + s + 1) + '</td>';
        html += '<td><input class="mapping-input mapping-name" placeholder="(empty)" /></td>';
        html += '<td><select class="mapping-select mapping-dsp-param">' + dspOptions + '</select></td>';
        html += '<td><input class="mapping-input mapping-mul" type="number" step="0.1" min="0" max="10" value="1.0" /></td>';
        html += '<td><input class="mapping-input mapping-div" type="number" step="0.1" min="0.1" max="10" value="1.0" /></td>';
        html += '<td></td>';
        html += '</tr>';
      }

      html += '</tbody></table>';
      html += '</div>';
    });

    // If no groups exist, show empty groups 1-6
    if (Object.keys(groupedMappings).length === 0) {
      for (var g = 0; g < 6; g++) {
        html += '<div class="mapping-group">';
        html += '<div class="mapping-group-title">';
        html += '<span>Group ' + (g + 1) + '</span>';
        html += '</div>';

        html += '<table class="mapping-table">';
        html += '<thead><tr>';
        html += '<th>Slot</th><th>Macro Name</th><th>DSP Parameter</th><th>Mul</th><th>Div</th><th></th>';
        html += '</tr></thead><tbody>';

        for (var s = 0; s < 4; s++) {
          html += '<tr class="mapping-row mapping-row-empty" data-group="' + g + '" data-slot="' + s + '">';
          html += '<td class="mapping-slot">' + (g * 4 + s + 1) + '</td>';
          html += '<td><input class="mapping-input mapping-name" placeholder="(empty)" /></td>';
          html += '<td><select class="mapping-select mapping-dsp-param">' + dspOptions + '</select></td>';
          html += '<td><input class="mapping-input mapping-mul" type="number" step="0.1" min="0" max="10" value="1.0" /></td>';
          html += '<td><input class="mapping-input mapping-div" type="number" step="0.1" min="0.1" max="10" value="1.0" /></td>';
          html += '<td></td>';
          html += '</tr>';
        }

        html += '</tbody></table></div>';
      }
    }

    container.innerHTML = html;
    setupMappingEditorEvents(container);
  }

  function setupMappingEditorEvents(container) {
    // Add mapping button
    container.querySelectorAll('.mapping-add-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var groupIdx = parseInt(btn.getAttribute('data-group'), 10);
        S.toast('Add mapping to group ' + (groupIdx + 1) + ' — coming soon', 'primary', 2000);
      });
    });

    // Remove mapping button
    container.querySelectorAll('.mapping-remove-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var row = btn.closest('.mapping-row');
        if (row) {
          row.classList.add('mapping-row-empty');
          row.querySelector('.mapping-name').value = '';
          row.querySelector('.mapping-dsp-param').value = '';
          row.querySelector('.mapping-mul').value = '1.0';
          row.querySelector('.mapping-div').value = '1.0';
          S.toast('Mapping removed', 'primary', 1500);
        }
      });
    });

    // Mapping value change highlight
    container.querySelectorAll('.mapping-input, .mapping-select').forEach(function(input) {
      input.addEventListener('change', function() {
        var row = input.closest('.mapping-row');
        if (row) {
          row.classList.add('mapping-changed');
          // Highlight matching DSP param
          var dspSelect = row.querySelector('.mapping-dsp-param');
          if (dspSelect && dspSelect.value) {
            highlightDSPParam(dspSelect.value);
          }
        }
      });
    });
  }

  // ─── DSP Parameters Panel ───────────────────────────────

  function renderDSPParams() {
    var container = document.getElementById('dsp-param-list');
    if (!container) return;

    if (!state.dspParams.length) {
      container.innerHTML = '<div class="empty-state"><p>No DSP parameters loaded</p></div>';
      return;
    }

    // Determine which params are mapped
    var mappedParamNames = {};
    state.mappings.forEach(function(m) {
      mappedParamNames[m.dspParam] = true;
    });

    var html = '';
    state.dspParams.forEach(function(p) {
      var isMapped = mappedParamNames[p.name] || false;
      html += '<div class="dsp-param-row' + (isMapped ? ' mapped' : '') + '" data-param-name="' + S.esc(p.name) + '">';
      html += '<span class="dsp-param-index">' + p.index + '</span>';
      html += '<span class="dsp-param-name">' + S.esc(p.name) + '</span>';
      html += '<span class="dsp-param-value">' + p.value + '</span>';
      if (isMapped) {
        html += '<sl-icon name="link" class="dsp-param-mapped-icon" title="Mapped to macro"></sl-icon>';
      }
      html += '</div>';
    });

    container.innerHTML = html;
    setupDSPParamEvents(container);
  }

  function setupDSPParamEvents(container) {
    container.querySelectorAll('.dsp-param-row').forEach(function(row) {
      row.addEventListener('click', function() {
        var name = row.getAttribute('data-param-name');
        // Toggle highlight
        container.querySelectorAll('.dsp-param-row').forEach(function(r) {
          r.classList.remove('highlighted');
        });
        row.classList.add('highlighted');

        // Find and highlight any mapping that references this param
        highlightMappingForParam(name);
      });
    });
  }

  function highlightDSPParam(paramName) {
    var list = document.getElementById('dsp-param-list');
    if (!list) return;

    list.querySelectorAll('.dsp-param-row').forEach(function(row) {
      row.classList.toggle('highlighted', row.getAttribute('data-param-name') === paramName);
    });
  }

  function highlightMappingForParam(paramName) {
    var editor = document.getElementById('mapping-editor-body');
    if (!editor) return;

    editor.querySelectorAll('.mapping-row').forEach(function(row) {
      var select = row.querySelector('.mapping-dsp-param');
      row.classList.toggle('mapping-highlighted', select && select.value === paramName);
    });
  }

  // ─── Toolbar Actions ─────────────────────────────────────

  function setupToolbarActions() {
    var saveBtn = document.getElementById('designer-save-btn');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        if (!state.selectedMachineDef) {
          S.toast('No machine definition selected', 'warning', 2000);
          return;
        }
        S.toast('Saved ' + state.selectedMachineDef.name, 'success', 2000);
      });
    }

    var exportBtn = document.getElementById('designer-export-btn');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        if (!state.selectedMachineDef) {
          S.toast('Nothing to export', 'warning', 2000);
          return;
        }
        // Build export JSON
        var exportData = {
          name: state.selectedMachineDef.name,
          plugin: state.selectedMachineDef.plugin,
          description: state.selectedMachineDef.description,
          mappings: state.mappings.map(function(m) {
            return {
              group: m.group,
              slot: m.slot,
              macroName: m.macroName,
              dspParam: m.dspParam,
              mul: m.mul,
              div: m.div,
            };
          }),
        };

        var blob = new Blob([JSON.stringify(exportData, null, 2)], { type: 'application/json' });
        var url = URL.createObjectURL(blob);
        var a = document.createElement('a');
        a.href = url;
        a.download = state.selectedMachineDef.name + '.json';
        a.click();
        URL.revokeObjectURL(url);
        S.toast('Exported ' + state.selectedMachineDef.name + '.json', 'success', 2000);
      });
    }

    var importBtn = document.getElementById('designer-import-btn');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        var input = document.createElement('input');
        input.type = 'file';
        input.accept = '.json';
        input.addEventListener('change', function() {
          if (input.files.length === 0) return;
          var reader = new FileReader();
          reader.onload = function() {
            try {
              var data = JSON.parse(reader.result);
              S.toast('Imported: ' + (data.name || 'unknown'), 'success', 2000);
              // In production: add to machine definitions list, reload
            } catch (err) {
              S.toast('Invalid JSON file', 'danger', 3000);
            }
          };
          reader.readAsText(input.files[0]);
        });
        input.click();
      });
    }

    var newBtn = document.getElementById('designer-new-btn');
    if (newBtn) {
      newBtn.addEventListener('click', function() {
        S.toast('New machine definition — coming soon', 'primary', 2000);
      });
    }
  }

  // ─── Initialization ─────────────────────────────────────

  function init() {
    state.machineDefinitions = generateMockMachineDefinitions();
    state.dspPlugins = generateMockDSPPlugins();

    renderTrackSelect();
    setupTrackSelectEvents();
    renderMachineDefSelect();
    renderMachineDefList();
    setupMachineDefListEvents();
    setupToolbarActions();

    // Start with empty mapping editor and DSP panel
    renderMappingEditor();
    renderDSPParams();

    // Auto-select first machine definition
    if (state.machineDefinitions.length > 0) {
      selectMachineDefinition(state.machineDefinitions[0].name);
    }

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.designer = {
    init: init,
    state: state,
  };

})();
