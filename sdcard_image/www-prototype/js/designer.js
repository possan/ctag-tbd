// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Sound Designer View (Real Data)
// Persona: Sound Designer
//
// Loads and edits real macro definitions and sound presets:
//   - synthdefinitions.json → machine list + DSP CC parameters
//   - macrodefinitions/*.json → parameter groups + output mappings
//   - macrosoundpresets/*.json → sound preset values
//
// Workflow:
//   1. Select a machine (DSP plugin) or existing macro definition
//   2. Edit parameter groups (6 groups × 4 params)
//   3. Edit output mappings (macro param → DSP CC, mul/div)
//   4. Save macro definition → create/load sound presets
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    synthDefs: null,           // From synthdefinitions.json
    tracks: [],                // All tracks
    machines: [],              // All machine definitions (id, name, type, parameters[])
    macroDefs: [],             // All loaded macro definitions
    soundPresets: [],          // All loaded sound presets
    selectedDefId: null,       // Currently selected/edited macro definition id
    editDef: null,             // Working copy of the definition being edited
    selectedTrack: 0,          // Track select in toolbar
    dirty: false,              // Has unsaved changes
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

  // ─── Data Loading ─────────────────────────────────────────

  function loadAllData() {
    S.showLoading('Loading definitions…');

    return Promise.all([
      apiGet('/api/v1/samples?getconfig=synthdefinitions.json'),
      apiGet('/api/v1/macroapi/definitions'),
      apiGet('/api/v1/macroapi/soundpresets'),
    ]).then(function(results) {
      state.synthDefs = results[0];
      state.tracks = results[0].tracks || [];
      state.machines = results[0].machines || [];
      state.macroDefs = results[1] || [];
      state.soundPresets = results[2] || [];
      S.hideLoading();
      console.log('[Designer] Loaded:', state.machines.length, 'machines,',
                  state.macroDefs.length, 'macro defs,',
                  state.soundPresets.length, 'sound presets');
    }).catch(function(err) {
      S.hideLoading();
      console.error('[Designer] Load error:', err);
      S.toast('Failed to load data: ' + err.message, 'danger', 4000);
    });
  }

  // ─── Track Select (toolbar) ──────────────────────────────

  function renderTrackSelect() {
    var select = document.getElementById('designer-track-select');
    if (!select) return;

    var html = '';
    state.tracks.forEach(function(t) {
      html += '<sl-option value="' + t.index + '">Ch ' + (t.index + 1) + ': ' + S.esc(t.name) + ' (' + S.esc(t.type) + ')</sl-option>';
    });
    select.innerHTML = html;
  }

  function setupTrackSelectEvents() {
    var select = document.getElementById('designer-track-select');
    if (!select) return;
    select.addEventListener('sl-change', function() {
      state.selectedTrack = parseInt(select.value, 10);
      // Update DSP panel to show this track's available machines
      renderDSPPanel();
    });
  }

  // ─── Machine Definition Select (toolbar) ─────────────────

  function renderMachineDefSelect() {
    var select = document.getElementById('designer-machine-select');
    if (!select) return;

    var html = '<sl-option value="">— New Definition —</sl-option>';
    state.macroDefs.forEach(function(def) {
      html += '<sl-option value="' + S.esc(def.id) + '">' +
              S.esc(def.name || def.id) + ' [' + S.esc(def.machine) + ']</sl-option>';
    });
    select.innerHTML = html;
  }

  function setupMachineDefSelectEvents() {
    var select = document.getElementById('designer-machine-select');
    if (!select) return;

    select.addEventListener('sl-change', function() {
      var defId = select.value;
      if (defId) {
        selectMacroDefinition(defId);
      } else {
        createNewDefinition();
      }
    });
  }

  // ─── Machine List (left panel) ───────────────────────────

  function renderMachineList() {
    var container = document.getElementById('machine-list');
    if (!container) return;

    // Group macro definitions by machine
    var byMachine = {};
    state.macroDefs.forEach(function(def) {
      var m = def.machine || 'unknown';
      if (!byMachine[m]) byMachine[m] = [];
      byMachine[m].push(def);
    });

    var html = '';

    // Show each machine group
    Object.keys(byMachine).sort().forEach(function(machineId) {
      var machineInfo = state.machines.find(function(m) { return m.id === machineId; });
      var machineName = machineInfo ? machineInfo.name : machineId;

      html += '<div class="machine-group">';
      html += '<div class="machine-group-header">' + S.esc(machineName) + ' <span style="opacity:0.5;">(' + S.esc(machineId) + ')</span></div>';

      byMachine[machineId].forEach(function(def) {
        var isActive = state.selectedDefId === def.id;
        html += '<div class="machine-item' + (isActive ? ' active' : '') + '" data-def-id="' + S.esc(def.id) + '">';
        html += '<div class="machine-item-name">' + S.esc(def.name || def.id) + '</div>';
        html += '<div class="machine-item-meta">';

        // Count mapped params
        var paramCount = 0;
        if (def.groups) {
          def.groups.forEach(function(g) { paramCount += (g.parameters || []).length; });
        }
        var mappingCount = (def.mapping || []).length;
        html += '<span>' + paramCount + ' params</span>';
        html += '<span>' + mappingCount + ' mappings</span>';
        html += '</div>';
        html += '</div>';
      });

      html += '</div>';
    });

    if (!html) {
      html = '<div class="empty-state" style="padding:1rem;"><p style="font-size:0.78rem;">No macro definitions found</p></div>';
    }

    container.innerHTML = html;
  }

  function setupMachineListEvents() {
    var container = document.getElementById('machine-list');
    if (!container) return;

    container.addEventListener('click', function(e) {
      var item = e.target.closest('.machine-item');
      if (!item) return;
      var defId = item.getAttribute('data-def-id');
      selectMacroDefinition(defId);

      // Also update the toolbar select
      var select = document.getElementById('designer-machine-select');
      if (select) select.value = defId;
    });
  }

  // ─── Select / Edit a Macro Definition ────────────────────

  function selectMacroDefinition(defId) {
    var def = state.macroDefs.find(function(d) { return d.id === defId; });
    if (!def) return;

    state.selectedDefId = defId;
    state.editDef = JSON.parse(JSON.stringify(def)); // Deep clone for editing
    state.dirty = false;

    // Ensure groups structure (6 groups × 4 params)
    ensureGroupStructure(state.editDef);

    // Update machine list active state
    document.querySelectorAll('.machine-item').forEach(function(item) {
      item.classList.toggle('active', item.getAttribute('data-def-id') === defId);
    });

    renderMappingEditor();
    renderDSPPanel();
  }

  function createNewDefinition() {
    state.selectedDefId = null;
    state.editDef = {
      id: '',
      name: '',
      machine: '',
      groups: [],
      mapping: [],
    };
    ensureGroupStructure(state.editDef);
    state.dirty = true;

    document.querySelectorAll('.machine-item').forEach(function(item) {
      item.classList.remove('active');
    });

    renderMappingEditor();
    renderDSPPanel();
  }

  /** Ensure definition has 6 groups with 4 param slots each */
  function ensureGroupStructure(def) {
    if (!def.groups) def.groups = [];
    while (def.groups.length < 6) {
      def.groups.push({
        name: 'Page ' + (def.groups.length + 1),
        parameters: [],
      });
    }
    var runningIdx = 0;
    def.groups.forEach(function(group) {
      if (!group.parameters) group.parameters = [];
      // Don't auto-fill — show actual params (may be 0-4 per group)
      group.parameters.forEach(function(p) {
        if (p.idx === undefined) {
          p.idx = runningIdx;
        }
        runningIdx = Math.max(runningIdx, p.idx + 1);
      });
    });
    if (!def.mapping) def.mapping = [];
  }

  // ─── Mapping Editor (center panel) ───────────────────────

  function renderMappingEditor() {
    var container = document.getElementById('mapping-editor');
    if (!container) return;

    if (!state.editDef) {
      container.innerHTML =
        '<div class="empty-state" id="mapping-empty">' +
        '<sl-icon name="diagram-3"></sl-icon>' +
        '<h3>No Machine Selected</h3>' +
        '<p>Select a macro definition from the left panel to view & edit parameter mappings</p>' +
        '</div>';
      return;
    }

    var def = state.editDef;

    // Get DSP params for the linked machine
    var machineInfo = state.machines.find(function(m) { return m.id === def.machine; });
    var machineParams = machineInfo ? (machineInfo.parameters || []) : [];

    // Build DSP param CC options for mapping dropdowns
    var ccOptions = '<option value="">—</option>';
    machineParams.forEach(function(p) {
      ccOptions += '<option value="' + p.ctrl + '">' + S.esc(p.name) + ' (CC ' + p.ctrl + ')</option>';
    });

    var html = '';

    // Definition header
    html += '<div class="mapping-def-header">';
    html += '<div class="mapping-def-fields">';
    html += '<label>ID:</label>';
    html += '<input class="mapping-input def-id-input" value="' + S.esc(def.id) + '" placeholder="e.g. db-mypatch" />';
    html += '<label>Name:</label>';
    html += '<input class="mapping-input def-name-input" value="' + S.esc(def.name) + '" placeholder="e.g. My Kick Patch" />';
    html += '<label>Machine:</label>';
    html += '<select class="mapping-select def-machine-select">';
    html += '<option value="">— Select —</option>';
    state.machines.forEach(function(m) {
      if (m.id === 'nodrum' || m.id === 'nosynth' || m.id === 'nofx') return;
      var sel = (m.id === def.machine) ? ' selected' : '';
      html += '<option value="' + S.esc(m.id) + '"' + sel + '>' + S.esc(m.name) + ' (' + S.esc(m.id) + ')</option>';
    });
    html += '</select>';
    html += '</div>';

    // Action buttons
    html += '<div class="mapping-def-actions">';
    html += '<button class="mapping-btn btn-1to1" title="Create 1:1 mapping from all machine CCs">1:1 Map</button>';
    html += '</div>';
    html += '</div>';

    // ── Tabs: Parameters | Output Mappings | Sound Presets ──
    html += '<div class="mapping-tabs">';
    html += '<button class="mapping-tab active" data-tab="params">Parameter Groups</button>';
    html += '<button class="mapping-tab" data-tab="mappings">Output Mappings (' + (def.mapping || []).length + ')</button>';
    html += '<button class="mapping-tab" data-tab="presets">Sound Presets</button>';
    html += '</div>';

    // ── TAB: Parameter Groups ──
    html += '<div class="mapping-tab-content active" data-tab="params">';
    html += renderParameterGroups(def, ccOptions);
    html += '</div>';

    // ── TAB: Output Mappings ──
    html += '<div class="mapping-tab-content" data-tab="mappings">';
    html += renderOutputMappings(def, machineParams);
    html += '</div>';

    // ── TAB: Sound Presets ──
    html += '<div class="mapping-tab-content" data-tab="presets">';
    html += renderSoundPresetsForDef(def);
    html += '</div>';

    container.innerHTML = html;
    setupMappingEditorEvents(container);
  }

  // ── Render: Parameter Groups ──

  function renderParameterGroups(def, ccOptions) {
    var html = '';

    def.groups.forEach(function(group, gi) {
      html += '<div class="mapping-group" data-group-idx="' + gi + '">';
      html += '<div class="mapping-group-title">';
      html += '<input class="mapping-input group-name-input" value="' + S.esc(group.name) + '" data-group="' + gi + '" placeholder="Group name" />';
      html += '<button class="mapping-add-btn add-param-btn" data-group="' + gi + '" title="Add parameter">+ Param</button>';
      html += '</div>';

      html += '<table class="mapping-table">';
      html += '<thead><tr>';
      html += '<th>#</th>';
      html += '<th>Name</th>';
      html += '<th>Default</th>';
      html += '<th>Min</th>';
      html += '<th>Max</th>';
      html += '<th>Res</th>';
      html += '<th>UI</th>';
      html += '<th></th>';
      html += '</tr></thead>';
      html += '<tbody>';

      (group.parameters || []).forEach(function(param, pi) {
        html += '<tr class="mapping-row" data-group="' + gi + '" data-param="' + pi + '">';
        html += '<td class="mapping-slot">' + param.idx + '</td>';
        html += '<td><input class="mapping-input param-name" value="' + S.esc(param.name) + '" /></td>';
        html += '<td><input class="mapping-input param-def" type="number" value="' + (param.def || 0) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-min" type="number" value="' + (param.min || 0) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-max" type="number" value="' + (param.max || 127) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-res" type="number" value="' + (param.res || 64) + '" style="width:50px;" /></td>';
        html += '<td>';
        html += '<select class="mapping-select param-ui">';
        ['bignum', 'slider', 'toggle', 'selector'].forEach(function(ui) {
          var sel = (param.ui === ui) ? ' selected' : '';
          html += '<option value="' + ui + '"' + sel + '>' + ui + '</option>';
        });
        html += '</select>';
        html += '</td>';
        html += '<td><button class="mapping-remove-btn remove-param-btn" data-group="' + gi + '" data-param="' + pi + '" title="Remove"><sl-icon name="x-circle"></sl-icon></button></td>';
        html += '</tr>';
      });

      // Empty row hint if no params
      if (!group.parameters || group.parameters.length === 0) {
        html += '<tr class="mapping-row-empty"><td colspan="8" style="text-align:center;opacity:0.4;padding:0.5rem;">No parameters — click "+ Param" to add</td></tr>';
      }

      html += '</tbody></table>';
      html += '</div>';
    });

    return html;
  }

  // ── Render: Output Mappings ──

  function renderOutputMappings(def, machineParams) {
    var mappings = def.mapping || [];
    var html = '';

    html += '<div class="mapping-output-header">';
    html += '<span>Output Mappings: macro parameter values → DSP CC values via formula</span>';
    html += '<button class="mapping-add-btn add-mapping-btn" title="Add output mapping">+ Mapping</button>';
    html += '</div>';

    html += '<div style="font-size:0.72rem;color:var(--sl-color-neutral-400);margin-bottom:0.5rem;">Formula: finalValue = start + Σ(paramValue × mul ÷ div)</div>';

    // Build param index→name map
    var paramNames = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramNames[p.idx] = p.name || ('Param ' + p.idx);
      });
    });

    // CC name map
    var ccNames = {};
    machineParams.forEach(function(p) {
      ccNames[p.ctrl] = p.name;
    });

    html += '<table class="mapping-table mapping-output-table">';
    html += '<thead><tr>';
    html += '<th>CC #</th>';
    html += '<th>CC Name</th>';
    html += '<th>Start</th>';
    html += '<th>Sources (param × mul ÷ div)</th>';
    html += '<th></th>';
    html += '</tr></thead>';
    html += '<tbody>';

    mappings.forEach(function(m, mi) {
      html += '<tr class="mapping-row" data-mapping-idx="' + mi + '">';
      html += '<td><input class="mapping-input mapping-ctrl" type="number" value="' + (m.ctrl || 0) + '" style="width:50px;" /></td>';
      html += '<td class="mapping-cc-name">' + S.esc(ccNames[m.ctrl] || '?') + '</td>';
      html += '<td><input class="mapping-input mapping-start" type="number" value="' + (m.start || 0) + '" style="width:50px;" /></td>';
      html += '<td class="mapping-sources">';

      // Render each source in the add[] array
      (m.add || []).forEach(function(add, ai) {
        html += '<span class="mapping-source" data-mapping="' + mi + '" data-add="' + ai + '">';
        html += S.esc(paramNames[add.src] || ('P' + add.src));
        html += ' × ' + add.mul + ' ÷ ' + add.div;
        html += ' <button class="mapping-remove-src-btn" data-mapping="' + mi + '" data-add="' + ai + '">×</button>';
        html += '</span>';
      });
      html += '<button class="mapping-add-src-btn" data-mapping="' + mi + '" title="Add source">+src</button>';

      html += '</td>';
      html += '<td><button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove mapping"><sl-icon name="x-circle"></sl-icon></button></td>';
      html += '</tr>';
    });

    if (mappings.length === 0) {
      html += '<tr class="mapping-row-empty"><td colspan="5" style="text-align:center;opacity:0.4;padding:0.5rem;">No output mappings</td></tr>';
    }

    html += '</tbody></table>';

    return html;
  }

  // ── Render: Sound Presets for this definition ──

  function renderSoundPresetsForDef(def) {
    var matching = state.soundPresets.filter(function(p) {
      return p.macro === def.id;
    });

    var html = '';
    html += '<div class="mapping-output-header">';
    html += '<span>Sound Presets using "' + S.esc(def.id) + '" (' + matching.length + ')</span>';
    html += '<button class="mapping-add-btn create-preset-btn" title="Create new sound preset">+ New Preset</button>';
    html += '</div>';

    if (matching.length === 0) {
      html += '<div style="text-align:center;opacity:0.4;padding:1rem;">No sound presets reference this definition</div>';
      return html;
    }

    // Build param index→name map
    var paramNames = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramNames[p.idx] = p.name || ('Param ' + p.idx);
      });
    });

    html += '<table class="mapping-table">';
    html += '<thead><tr>';
    html += '<th>Preset</th>';
    html += '<th>Group</th>';
    html += '<th>Values</th>';
    html += '<th></th>';
    html += '</tr></thead>';
    html += '<tbody>';

    matching.forEach(function(preset) {
      html += '<tr class="mapping-row">';
      html += '<td>' + S.esc(preset.name || preset.id) + '</td>';
      html += '<td>' + S.esc(preset.group || '—') + '</td>';
      html += '<td class="preset-values-cell">';
      if (preset.values && preset.values.length > 0) {
        preset.values.forEach(function(v, vi) {
          var pName = paramNames[vi] || vi;
          html += '<span class="preset-value-chip" title="' + S.esc(String(pName)) + '">' + v + '</span>';
        });
      } else {
        html += '<span style="opacity:0.4;">empty</span>';
      }
      html += '</td>';
      html += '<td>';
      html += '<button class="mapping-remove-btn delete-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Delete"><sl-icon name="trash"></sl-icon></button>';
      html += '</td>';
      html += '</tr>';
    });

    html += '</tbody></table>';

    return html;
  }

  // ─── Mapping Editor Events ───────────────────────────────

  function setupMappingEditorEvents(container) {
    // Tab switching
    container.querySelectorAll('.mapping-tab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        var tabId = tab.getAttribute('data-tab');
        container.querySelectorAll('.mapping-tab').forEach(function(t) { t.classList.toggle('active', t.getAttribute('data-tab') === tabId); });
        container.querySelectorAll('.mapping-tab-content').forEach(function(c) { c.classList.toggle('active', c.getAttribute('data-tab') === tabId); });
      });
    });

    // Definition header fields
    var defIdInput = container.querySelector('.def-id-input');
    var defNameInput = container.querySelector('.def-name-input');
    var defMachineSelect = container.querySelector('.def-machine-select');

    if (defIdInput) {
      defIdInput.addEventListener('change', function() {
        if (state.editDef) { state.editDef.id = defIdInput.value; state.dirty = true; }
      });
    }
    if (defNameInput) {
      defNameInput.addEventListener('change', function() {
        if (state.editDef) { state.editDef.name = defNameInput.value; state.dirty = true; }
      });
    }
    if (defMachineSelect) {
      defMachineSelect.addEventListener('change', function() {
        if (state.editDef) {
          state.editDef.machine = defMachineSelect.value;
          state.dirty = true;
          renderMappingEditor(); // Re-render to update CC options
          renderDSPPanel();
        }
      });
    }

    // 1:1 mapping button
    var btn1to1 = container.querySelector('.btn-1to1');
    if (btn1to1) {
      btn1to1.addEventListener('click', function() {
        createOneToOneMapping();
      });
    }

    // Group name changes
    container.querySelectorAll('.group-name-input').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        if (state.editDef && state.editDef.groups[gi]) {
          state.editDef.groups[gi].name = input.value;
          state.dirty = true;
        }
      });
    });

    // Parameter field changes
    container.querySelectorAll('.mapping-row[data-group][data-param]').forEach(function(row) {
      var gi = parseInt(row.getAttribute('data-group'), 10);
      var pi = parseInt(row.getAttribute('data-param'), 10);

      row.querySelectorAll('input, select').forEach(function(input) {
        input.addEventListener('change', function() {
          if (!state.editDef) return;
          var param = state.editDef.groups[gi].parameters[pi];
          if (!param) return;
          if (input.classList.contains('param-name')) param.name = input.value;
          if (input.classList.contains('param-def')) param.def = parseInt(input.value, 10) || 0;
          if (input.classList.contains('param-min')) param.min = parseInt(input.value, 10) || 0;
          if (input.classList.contains('param-max')) param.max = parseInt(input.value, 10) || 127;
          if (input.classList.contains('param-res')) param.res = parseInt(input.value, 10) || 64;
          if (input.classList.contains('param-ui')) param.ui = input.value;
          state.dirty = true;
        });
      });
    });

    // Add parameter button
    container.querySelectorAll('.add-param-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;

        // Find next available idx
        var maxIdx = -1;
        state.editDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx > maxIdx) maxIdx = p.idx;
          });
        });

        state.editDef.groups[gi].parameters.push({
          idx: maxIdx + 1,
          name: 'New Param',
          def: 0,
          min: 0,
          max: 127,
          res: 64,
          ui: 'bignum',
        });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Remove parameter button
    container.querySelectorAll('.remove-param-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        var pi = parseInt(btn.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        state.editDef.groups[gi].parameters.splice(pi, 1);
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Output mapping field changes
    container.querySelectorAll('.mapping-row[data-mapping-idx]').forEach(function(row) {
      var mi = parseInt(row.getAttribute('data-mapping-idx'), 10);
      row.querySelectorAll('input').forEach(function(input) {
        input.addEventListener('change', function() {
          if (!state.editDef || !state.editDef.mapping[mi]) return;
          if (input.classList.contains('mapping-ctrl')) {
            state.editDef.mapping[mi].ctrl = parseInt(input.value, 10) || 0;
          }
          if (input.classList.contains('mapping-start')) {
            state.editDef.mapping[mi].start = parseInt(input.value, 10) || 0;
          }
          state.dirty = true;
          renderMappingEditor(); // Re-render to update CC name
        });
      });
    });

    // Add output mapping
    var addMappingBtn = container.querySelector('.add-mapping-btn');
    if (addMappingBtn) {
      addMappingBtn.addEventListener('click', function() {
        if (!state.editDef) return;
        state.editDef.mapping.push({ ctrl: 0, start: 0, add: [] });
        state.dirty = true;
        renderMappingEditor();
      });
    }

    // Remove output mapping
    container.querySelectorAll('.remove-mapping-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        if (!state.editDef) return;
        state.editDef.mapping.splice(mi, 1);
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Add source to mapping
    container.querySelectorAll('.mapping-add-src-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;

        var srcStr = prompt('Source param index (0-23):');
        if (srcStr === null) return;
        var src = parseInt(srcStr, 10);
        if (isNaN(src)) return;

        state.editDef.mapping[mi].add.push({ src: src, mul: 1, div: 1 });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Remove source from mapping
    container.querySelectorAll('.mapping-remove-src-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var mi = parseInt(btn.getAttribute('data-mapping'), 10);
        var ai = parseInt(btn.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        state.editDef.mapping[mi].add.splice(ai, 1);
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Create preset button
    var createPresetBtn = container.querySelector('.create-preset-btn');
    if (createPresetBtn) {
      createPresetBtn.addEventListener('click', function() {
        createSoundPresetForDef();
      });
    }

    // Delete preset buttons
    container.querySelectorAll('.delete-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var presetId = btn.getAttribute('data-preset-id');
        deleteSoundPreset(presetId);
      });
    });
  }

  // ─── Create 1:1 Mapping ──────────────────────────────────

  function createOneToOneMapping() {
    if (!state.editDef || !state.editDef.machine) {
      S.toast('Select a machine first', 'warning', 2000);
      return;
    }

    var machineInfo = state.machines.find(function(m) { return m.id === state.editDef.machine; });
    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      S.toast('Machine has no CC parameters', 'warning', 2000);
      return;
    }

    if (!confirm('This will replace all parameter groups and output mappings with a 1:1 mapping from all ' + machineInfo.parameters.length + ' CCs. Continue?')) {
      return;
    }

    var params = machineInfo.parameters;

    // Set ID and name if empty
    if (!state.editDef.id) {
      state.editDef.id = state.editDef.machine + '-allparams';
    }
    if (!state.editDef.name) {
      state.editDef.name = (machineInfo.name || state.editDef.machine) + ' All params';
    }

    // Create groups: 4 params per group, across 6 groups max
    state.editDef.groups = [];
    var paramIdx = 0;
    for (var g = 0; g < 6 && paramIdx < params.length; g++) {
      var groupParams = [];
      for (var p = 0; p < 4 && paramIdx < params.length; p++) {
        var cc = params[paramIdx];
        groupParams.push({
          idx: paramIdx,
          name: cc.name || ('CC' + cc.ctrl),
          def: cc.def || 0,
          min: 0,
          max: 127,
          res: 64,
          ui: 'bignum',
        });
        paramIdx++;
      }
      state.editDef.groups.push({
        name: 'Page ' + (g + 1),
        parameters: groupParams,
      });
    }

    // Create 1:1 output mappings
    state.editDef.mapping = [];
    var allParams = [];
    state.editDef.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        allParams.push(p);
      });
    });

    params.forEach(function(cc, i) {
      if (i < allParams.length) {
        state.editDef.mapping.push({
          ctrl: cc.ctrl,
          start: 0,
          add: [{ src: allParams[i].idx, mul: 1, div: 1 }],
        });
      }
    });

    state.dirty = true;
    renderMappingEditor();
    S.toast('Created 1:1 mapping with ' + params.length + ' parameters', 'success', 2000);
  }

  // ─── Sound Preset CRUD ──────────────────────────────────

  function createSoundPresetForDef() {
    if (!state.editDef || !state.editDef.id) {
      S.toast('Save the definition first', 'warning', 2000);
      return;
    }

    var name = prompt('Sound preset name:');
    if (!name) return;

    var id = name.toLowerCase().replace(/[^a-z0-9]+/g, '-');
    var group = prompt('Preset group:', state.editDef.machine);
    if (group === null) return;

    // Collect default values from the definition
    var values = [];
    state.editDef.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        values[p.idx] = p.def || 0;
      });
    });

    var preset = {
      id: id,
      name: name,
      group: group || 'User',
      macro: state.editDef.id,
      values: values,
    };

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = 'macrosoundpresets/' + id + '.json';

    fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Created preset: ' + name, 'success', 2000);
      return apiGet('/api/v1/macroapi/soundpresets');
    }).then(function(presets) {
      state.soundPresets = presets || [];
      renderMappingEditor();
    }).catch(function(err) {
      S.toast('Create failed: ' + err.message, 'danger', 3000);
    });
  }

  function deleteSoundPreset(presetId) {
    if (!confirm('Delete sound preset "' + presetId + '"?')) return;

    var filePath = 'macrosoundpresets/' + presetId + '.json';
    apiPost('/api/v1/samples?action=manage', {
      action: 'deleteconfig',
      path: filePath,
    }).then(function() {
      S.toast('Deleted preset: ' + presetId, 'success', 2000);
      return apiGet('/api/v1/macroapi/soundpresets');
    }).then(function(presets) {
      state.soundPresets = presets || [];
      renderMappingEditor();
    }).catch(function(err) {
      S.toast('Delete failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── DSP Panel (right) ──────────────────────────────────

  function renderDSPPanel() {
    var container = document.getElementById('dsp-param-list');
    if (!container) return;

    // Show DSP params for the selected definition's machine
    var machineId = state.editDef ? state.editDef.machine : '';
    var machineInfo = state.machines.find(function(m) { return m.id === machineId; });

    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      container.innerHTML =
        '<div class="empty-state" style="padding:1.5rem;">' +
        '<sl-icon name="cpu" style="font-size:1.5rem;"></sl-icon>' +
        '<p style="font-size:0.78rem;">Select a definition with a machine to see DSP CC parameters</p>' +
        '</div>';
      return;
    }

    // Build a set of mapped ctrl numbers
    var mappedCtrls = {};
    if (state.editDef && state.editDef.mapping) {
      state.editDef.mapping.forEach(function(m) {
        mappedCtrls[m.ctrl] = true;
      });
    }

    var html = '<div class="dsp-panel-title">' + S.esc(machineInfo.name) + ' (' + S.esc(machineId) + ')</div>';

    machineInfo.parameters.forEach(function(p) {
      var isMapped = mappedCtrls[p.ctrl] || false;
      html += '<div class="dsp-param-row' + (isMapped ? ' mapped' : '') + '" data-ctrl="' + p.ctrl + '">';
      html += '<span class="dsp-param-index">CC ' + p.ctrl + '</span>';
      html += '<span class="dsp-param-name">' + S.esc(p.name) + '</span>';
      html += '<span class="dsp-param-value">def: ' + (p.def || 0) + '</span>';
      if (isMapped) {
        html += '<sl-icon name="link" class="dsp-param-mapped-icon" title="Mapped"></sl-icon>';
      }
      html += '</div>';
    });

    container.innerHTML = html;
  }

  // ─── Toolbar Actions ─────────────────────────────────────

  function setupToolbarActions() {
    var saveBtn = document.getElementById('designer-save-btn');
    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        saveDefinition();
      });
    }

    var exportBtn = document.getElementById('designer-export-btn');
    if (exportBtn) {
      exportBtn.addEventListener('click', function() {
        exportDefinition();
      });
    }

    var importBtn = document.getElementById('designer-import-btn');
    if (importBtn) {
      importBtn.addEventListener('click', function() {
        importDefinitionFile();
      });
    }
  }

  function saveDefinition() {
    if (!state.editDef) {
      S.toast('Nothing to save', 'warning', 2000);
      return;
    }
    if (!state.editDef.id) {
      S.toast('Definition ID is required', 'warning', 2000);
      return;
    }
    if (!state.editDef.machine) {
      S.toast('Select a machine for this definition', 'warning', 2000);
      return;
    }

    var jsonStr = JSON.stringify(state.editDef, null, 2);
    var filePath = 'macrodefinitions/' + state.editDef.id + '.json';

    S.showLoading('Saving definition…');

    fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved: ' + state.editDef.name, 'success', 2000);
      state.dirty = false;

      // Reload definitions
      return apiGet('/api/v1/macroapi/definitions');
    }).then(function(defs) {
      state.macroDefs = defs || [];
      renderMachineList();
      renderMachineDefSelect();
      S.hideLoading();
    }).catch(function(err) {
      S.hideLoading();
      S.toast('Save failed: ' + err.message, 'danger', 3000);
    });
  }

  function exportDefinition() {
    if (!state.editDef) {
      S.toast('Nothing to export', 'warning', 2000);
      return;
    }

    var blob = new Blob([JSON.stringify(state.editDef, null, 2)], { type: 'application/json' });
    var a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = (state.editDef.id || 'definition') + '.json';
    a.click();
    URL.revokeObjectURL(a.href);
    S.toast('Exported: ' + (state.editDef.name || state.editDef.id), 'success', 2000);
  }

  function importDefinitionFile() {
    var input = document.createElement('input');
    input.type = 'file';
    input.accept = '.json';
    input.addEventListener('change', function() {
      if (!input.files.length) return;
      var reader = new FileReader();
      reader.onload = function() {
        try {
          var data = JSON.parse(reader.result);
          if (data.groups && data.mapping) {
            // It's a macro definition
            state.editDef = data;
            state.selectedDefId = data.id || null;
            ensureGroupStructure(state.editDef);
            state.dirty = true;
            renderMappingEditor();
            renderDSPPanel();
            S.toast('Imported definition: ' + (data.name || data.id || 'unknown'), 'success', 2000);
          } else if (data.id && data.macro && data.values) {
            // It's a sound preset — upload directly
            var filePath = 'macrosoundpresets/' + data.id + '.json';
            fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify(data, null, 2),
            }).then(function(r) {
              if (!r.ok) throw new Error('HTTP ' + r.status);
              S.toast('Imported preset: ' + data.name, 'success', 2000);
              return apiGet('/api/v1/macroapi/soundpresets');
            }).then(function(presets) {
              state.soundPresets = presets || [];
              renderMappingEditor();
            }).catch(function(err) {
              S.toast('Import failed: ' + err.message, 'danger', 3000);
            });
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

  // ─── Initialization ─────────────────────────────────────

  function init() {
    loadAllData().then(function() {
      renderTrackSelect();
      setupTrackSelectEvents();
      renderMachineDefSelect();
      setupMachineDefSelectEvents();
      renderMachineList();
      setupMachineListEvents();
      setupToolbarActions();

      // Start with empty editor
      renderMappingEditor();
      renderDSPPanel();

      // Auto-select first definition if available
      if (state.macroDefs.length > 0) {
        selectMacroDefinition(state.macroDefs[0].id);
        var select = document.getElementById('designer-machine-select');
        if (select) select.value = state.macroDefs[0].id;
      }

      state.initialized = true;
    });
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.designer = {
    init: init,
    state: state,
    reload: function() {
      loadAllData().then(function() {
        renderMachineList();
        renderMachineDefSelect();
        if (state.selectedDefId) selectMacroDefinition(state.selectedDefId);
      });
    },
  };

})();
