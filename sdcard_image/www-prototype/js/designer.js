// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Sound Designer View
// Persona: Sound Designer / Expert
//
// Uses shared data and shared track tabs from shared.js.
// The left panel shows macro definitions FILTERED by the active
// track's available machines — not all definitions globally.
//
// Workflow:
//   1. Select a track via the shared track tabs
//   2. Left panel shows macro definitions for that track's machines
//   3. Pick a definition to edit (or create new)
//   4. Edit parameter groups, output mappings, preview knobs
//   5. Save
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var state = {
    selectedDefId: null,       // Currently selected/edited macro definition id
    editDef: null,             // Working copy of the definition being edited
    activeTrack: -1,           // Active track index (from shared track tabs)
    trackMachines: [],         // Machine IDs available for the active track
    activeMachine: '',         // Currently filtered machine id (or '' for all)
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

  // ─── Track Selection (from shared track tabs) ────────────

  function onTrackSelected(idx, track) {
    state.activeTrack = idx;
    state.trackMachines = S.getTrackMachines(track);

    // Clear current selection
    state.selectedDefId = null;
    state.editDef = null;
    state.dirty = false;

    // Auto-select first machine for this track
    state.activeMachine = state.trackMachines.length > 0 ? state.trackMachines[0] : '';

    renderMachineSelectToolbar();
    updateFilterHelp();
    renderMachineList();
    renderMappingEditor();
    renderDSPPanel();

    // Auto-select first matching definition
    var filteredDefs = getFilteredDefs();
    if (filteredDefs.length > 0) {
      selectMacroDefinition(filteredDefs[0].id);
    }
  }

  function updateFilterHelp() {
    var filterHelp = document.getElementById('designer-filter-help');
    if (!filterHelp) return;
    var p = filterHelp.querySelector('p');
    if (!p) return;

    if (!state.activeMachine) {
      p.textContent = 'Select a machine above to see and edit its macro definitions.';
    } else {
      var info = S.getMachineInfo(state.activeMachine);
      var machineName = info ? info.name : state.activeMachine;
      p.textContent = 'Macro definitions for ' + machineName + '.';
    }
  }

  // ─── Machine Select (toolbar) ────────────────────────────

  function renderMachineSelectToolbar() {
    var select = document.getElementById('designer-machine-select');
    if (!select) return;

    var html = '';
    state.trackMachines.forEach(function(mid) {
      var info = S.getMachineInfo(mid);
      var name = info ? info.name : mid;
      html += '<sl-option value="' + S.esc(mid) + '">' + S.esc(name) + '</sl-option>';
    });
    select.innerHTML = html;
    select.value = state.activeMachine || '';
  }

  function setupMachineSelectToolbarEvents() {
    var select = document.getElementById('designer-machine-select');
    if (!select) return;

    select.addEventListener('sl-change', function() {
      state.activeMachine = select.value || '';
      state.selectedDefId = null;
      state.editDef = null;
      state.dirty = false;

      updateFilterHelp();
      renderMachineList();
      renderMappingEditor();
      renderDSPPanel();

      // Auto-select first matching definition
      var filteredDefs = getFilteredDefs();
      if (filteredDefs.length > 0) {
        selectMacroDefinition(filteredDefs[0].id);
      }
    });
  }

  /**
   * Get macro definitions filtered by active machine (or all track machines).
   */
  function getFilteredDefs() {
    if (!state.trackMachines || state.trackMachines.length === 0) return [];
    if (state.activeMachine) {
      // Filter to specific machine
      return S.data.macroDefs.filter(function(d) {
        return d.machine === state.activeMachine;
      });
    }
    // Show all for this track's machines
    return S.data.macroDefs.filter(function(d) {
      return state.trackMachines.indexOf(d.machine) !== -1;
    });
  }



  // ─── Machine List (left panel — FILTERED by machine) ───────

  function renderMachineList() {
    var container = document.getElementById('machine-list');
    if (!container) return;

    var filteredDefs = getFilteredDefs();

    if (state.trackMachines.length === 0) {
      container.innerHTML =
        '<div class="empty-state" style="padding:1.5rem;">' +
        '<sl-icon name="cpu" style="font-size:1.5rem;"></sl-icon>' +
        '<p style="font-size:0.78rem;">Select a track above to see its macro definitions</p>' +
        '</div>';
      return;
    }

    var html = '';

    if (state.activeMachine) {
      // Single machine selected — show defs with "Create New" button
      html += '<button class="machine-list-add-btn" id="create-def-btn" style="width:calc(100% - 1.7rem);margin:0.4rem 0.85rem;padding:0.4rem;border:1px solid var(--sl-color-neutral-300);background:var(--sl-color-neutral-100);border-radius:var(--sl-border-radius-small);cursor:pointer;font-size:0.75rem;font-weight:600;color:var(--sl-color-primary-700);transition:all 0.12s;">+ Create New Definition</button>';
      
      if (filteredDefs.length === 0) {
        html += '<div class="machine-item" style="opacity:0.4;cursor:default;padding:0.3rem 0.85rem;font-size:0.75rem;">No definitions yet</div>';
      }
      filteredDefs.forEach(function(def) {
        var isActive = state.selectedDefId === def.id;
        html += '<div class="machine-item' + (isActive ? ' active' : '') + '" data-def-id="' + S.esc(def.id) + '">';
        html += '<div class="machine-item-name">' + S.esc(def.name || def.id) + '</div>';
        html += '<div class="machine-item-meta">';
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
    } else {
      // "Show All" — group by machine
      var byMachine = {};
      filteredDefs.forEach(function(def) {
        var m = def.machine || 'unknown';
        if (!byMachine[m]) byMachine[m] = [];
        byMachine[m].push(def);
      });

      state.trackMachines.forEach(function(machineId) {
        var machineInfo = S.getMachineInfo(machineId);
        var machineName = machineInfo ? machineInfo.name : machineId;
        var defs = byMachine[machineId] || [];

        html += '<div class="machine-group">';
        html += '<div class="machine-group-header">' + S.esc(machineName) + ' <span style="opacity:0.5;">(' + S.esc(machineId) + ')</span></div>';

        if (defs.length === 0) {
          html += '<div class="machine-item" style="opacity:0.4;cursor:default;padding:0.3rem 0.85rem;font-size:0.75rem;">No definitions yet</div>';
        }

        defs.forEach(function(def) {
          var isActive = state.selectedDefId === def.id;
          html += '<div class="machine-item' + (isActive ? ' active' : '') + '" data-def-id="' + S.esc(def.id) + '">';
          html += '<div class="machine-item-name">' + S.esc(def.name || def.id) + '</div>';
          html += '<div class="machine-item-meta">';
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
    }

    if (!html) {
      html = '<div class="empty-state" style="padding:1rem;"><p style="font-size:0.78rem;">No machines on this track</p></div>';
    }

    container.innerHTML = html;
  }

  function setupMachineListEvents() {
    var container = document.getElementById('machine-list');
    if (!container) return;

    // Handle "Create New Definition" button
    container.addEventListener('click', function(e) {
      if (e.target.id === 'create-def-btn') {
        createNewDefinition();
        return;
      }
      
      var item = e.target.closest('.machine-item');
      if (!item) return;
      var defId = item.getAttribute('data-def-id');
      if (!defId) return;
      selectMacroDefinition(defId);
    });
  }

  // ─── Select / Edit a Macro Definition ────────────────────

  function selectMacroDefinition(defId) {
    var def = S.data.macroDefs.find(function(d) { return d.id === defId; });
    if (!def) return;

    state.selectedDefId = defId;
    state.editDef = JSON.parse(JSON.stringify(def)); // Deep clone
    state.dirty = false;

    ensureGroupStructure(state.editDef);

    // Update machine list active state
    document.querySelectorAll('.machine-item').forEach(function(item) {
      item.classList.toggle('active', item.getAttribute('data-def-id') === defId);
    });

    renderMappingEditor();
    renderDSPPanel();
  }

  function createNewDefinition() {
    // Default machine to the currently filtered machine, or first on track
    var defaultMachine = state.activeMachine || (state.trackMachines.length > 0 ? state.trackMachines[0] : '');

    state.selectedDefId = null;
    state.editDef = {
      id: '',
      name: '',
      machine: defaultMachine,
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
        '<h3>Select a Macro Definition</h3>' +
        '<p>Pick a definition from the left panel to edit how DSP parameters are exposed as performer knobs.</p>' +
        '<div class="info-callout" style="margin-top:1rem;max-width:450px;">' +
        '<sl-icon name="lightbulb"></sl-icon>' +
        '<p><strong>What is a Macro Definition?</strong><br>' +
        'It maps a machine\'s raw DSP control-change (CC) parameters ' +
        'to user-friendly knob pages that a performer sees. You can combine, ' +
        'scale, and offset multiple CC values from a single knob.</p>' +
        '</div>' +
        '</div>';
      return;
    }

    var def = state.editDef;

    // Get DSP params for the linked machine
    var machineInfo = S.getMachineInfo(def.machine);
    var machineParams = machineInfo ? (machineInfo.parameters || []) : [];

    // Build DSP param CC options
    var ccOptions = '<option value="">—</option>';
    machineParams.forEach(function(p) {
      ccOptions += '<option value="' + p.ctrl + '">' + S.esc(p.name) + ' (CC ' + p.ctrl + ')</option>';
    });

    var html = '';

    // Definition header (simplified — Machine is already in toolbar)
    html += '<div class="mapping-def-header">';
    html += '<div class="mapping-def-fields">';
    html += '<label>ID:</label>';
    html += '<input class="mapping-input def-id-input" value="' + S.esc(def.id) + '" placeholder="e.g. db-mypatch" />';
    html += '<label>Name:</label>';
    html += '<input class="mapping-input def-name-input" value="' + S.esc(def.name) + '" placeholder="e.g. My Patch" />';
    // Machine is now maintained in toolbar, keep hidden input for data binding
    html += '<input type="hidden" class="def-machine-select" value="' + S.esc(def.machine) + '" />';
    html += '</div>';

    // Action buttons
    html += '<div class="mapping-def-actions">';
    html += '<button class="mapping-btn btn-1to1" title="Auto-create a 1:1 mapping from all machine CCs">1:1 Map</button>';
    html += '</div>';
    html += '</div>';

    // ── Tabs ──
    html += '<div class="mapping-tabs">';
    html += '<button class="mapping-tab active" data-tab="preview"><sl-icon name="sliders" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Knob Preview</button>';
    html += '<button class="mapping-tab" data-tab="params"><sl-icon name="table" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Parameter Groups</button>';
    html += '<button class="mapping-tab" data-tab="mappings"><sl-icon name="arrow-left-right" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Output Mappings (' + (def.mapping || []).length + ')</button>';
    html += '<button class="mapping-tab" data-tab="presets"><sl-icon name="collection" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Sound Presets</button>';
    html += '</div>';

    // ── TAB: Knob Preview ──
    html += '<div class="mapping-tab-content active" data-tab="preview">';
    html += renderKnobPreview(def);
    html += '</div>';

    // ── TAB: Parameter Groups ──
    html += '<div class="mapping-tab-content" data-tab="params">';
    html += '<div class="tab-description">';
    html += '<sl-icon name="info-circle"></sl-icon>';
    html += 'Define up to 6 pages of 4 knobs each. These are the controls a Performer sees.';
    html += '</div>';
    html += renderParameterGroups(def, ccOptions);
    html += '</div>';

    // ── TAB: Output Mappings ──
    html += '<div class="mapping-tab-content" data-tab="mappings">';
    html += '<div class="tab-description">';
    html += '<sl-icon name="info-circle"></sl-icon>';
    html += 'Each output mapping connects knob values to a DSP CC channel. Formula: <code>finalValue = start + &Sigma;(paramValue &times; mul &divide; div)</code>.';
    html += '</div>';
    html += renderOutputMappings(def, machineParams);
    html += '</div>';

    // ── TAB: Sound Presets ──
    html += '<div class="mapping-tab-content" data-tab="presets">';
    html += '<div class="tab-description">';
    html += '<sl-icon name="info-circle"></sl-icon>';
    html += 'Sound presets store specific knob values for this definition. A Performer can quickly recall these.';
    html += '</div>';
    html += renderSoundPresetsForDef(def);
    html += '</div>';

    container.innerHTML = html;
    setupMappingEditorEvents(container);
  }

  // ── Render: Knob Preview ──

  function renderKnobPreview(def) {
    var html = '';
    var mappingInfo = S.analyzeMappings(def);

    html += '<div class="designer-knob-preview">';
    html += '<div class="preview-title"><sl-icon name="eye"></sl-icon> Performer Knob Preview</div>';
    html += '<div class="info-callout">';
    html += '<sl-icon name="lightbulb"></sl-icon>';
    html += '<p>This shows how the macro definition will appear to a Performer. Each page becomes a collapsible group of knobs.</p>';
    html += '</div>';

    var hasParams = false;
    def.groups.forEach(function(group, gi) {
      if (!group.parameters || group.parameters.length === 0) return;
      hasParams = true;

      html += '<div class="macro-group" data-group="' + gi + '">';
      html += '<div class="macro-group-header">';
      html += '<sl-icon name="chevron-down" class="macro-group-chevron"></sl-icon>';
      html += '<span class="macro-group-name">' + S.esc(group.name || ('Page ' + (gi + 1))) + '</span>';
      html += '</div>';
      html += '<div class="macro-group-body">';

      group.parameters.forEach(function(param) {
        var value = param.def || 0;
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
        html += '<span class="macro-knob-label">' + S.esc(param.name || ('P' + param.idx)) + '</span>';
        html += '<span class="macro-knob-value">' + value + '</span>';
        // Show curve indicator if non-linear
        if (param.curve && param.curve !== 'linear') {
          html += '<span class="curve-badge">' + S.esc(param.curve) + '</span>';
        }

        // Show mapping targets with real-time computed values
        var targets = mappingInfo[param.idx] || [];
        if (targets.length > 0) {
          var outputs = S.computeMappingOutputs(def, param.idx, value);
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

        html += '</div>';
      });

      html += '</div>';
      html += '</div>';
    });

    if (!hasParams) {
      html += '<div class="empty-state" style="padding:2rem;">';
      html += '<sl-icon name="sliders" style="font-size:2rem;"></sl-icon>';
      html += '<h3>No Parameters Defined</h3>';
      html += '<p>Add parameters in the "Parameter Groups" tab to see a knob preview here.</p>';
      html += '</div>';
    }

    html += '</div>';
    return html;
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
      html += '<th>#</th><th>Name</th><th>Default</th><th>Min</th><th>Max</th><th>Res</th><th>Curve</th><th>UI</th><th></th>';
      html += '</tr></thead>';
      html += '<tbody>';

      (group.parameters || []).forEach(function(param, pi) {
        html += '<tr class="mapping-row" data-group="' + gi + '" data-param="' + pi + '">';
        html += '<td class="mapping-slot">' + param.idx + '</td>';
        html += '<td><input class="mapping-input param-name" value="' + S.esc(param.name) + '" style="width:120px;text-align:left;" /></td>';
        html += '<td><input class="mapping-input param-def" type="number" value="' + (param.def || 0) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-min" type="number" value="' + (param.min || 0) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-max" type="number" value="' + (param.max || 127) + '" style="width:50px;" /></td>';
        html += '<td><input class="mapping-input param-res" type="number" value="' + (param.res || 64) + '" style="width:50px;" /></td>';
        html += '<td>';
        html += '<select class="mapping-select param-curve">';
        ['linear', 'log', 'exp', 'scurve'].forEach(function(curve) {
          var sel = (param.curve === curve) ? ' selected' : '';
          html += '<option value="' + curve + '"' + sel + '>' + curve + '</option>';
        });
        html += '</select>';
        html += '</td>';
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

      if (!group.parameters || group.parameters.length === 0) {
        html += '<tr class="mapping-row-empty"><td colspan="9" style="text-align:center;opacity:0.4;padding:0.5rem;">No parameters — click "+ Param" to add</td></tr>';
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
    html += '<span>' + mappings.length + ' Output Mapping' + (mappings.length !== 1 ? 's' : '') + '</span>';
    html += '<button class="mapping-add-btn add-mapping-btn" title="Add output mapping">+ Mapping</button>';
    html += '</div>';

    var paramNames = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramNames[p.idx] = p.name || ('Param ' + p.idx);
      });
    });

    var ccNames = {};
    machineParams.forEach(function(p) {
      ccNames[p.ctrl] = p.name;
    });

    html += '<table class="mapping-table mapping-output-table">';
    html += '<thead><tr>';
    html += '<th>CC #</th><th>CC Name</th><th>Start</th><th>Sources (param × mul ÷ div)</th><th></th>';
    html += '</tr></thead>';
    html += '<tbody>';

    mappings.forEach(function(m, mi) {
      html += '<tr class="mapping-row" data-mapping-idx="' + mi + '">';
      html += '<td><input class="mapping-input mapping-ctrl" type="number" value="' + (m.ctrl || 0) + '" style="width:50px;" /></td>';
      html += '<td class="mapping-cc-name">' + S.esc(ccNames[m.ctrl] || '?') + '</td>';
      html += '<td><input class="mapping-input mapping-start" type="number" value="' + (m.start || 0) + '" style="width:50px;" /></td>';
      html += '<td class="mapping-sources">';

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
      html += '<tr class="mapping-row-empty"><td colspan="5" style="text-align:center;opacity:0.4;padding:0.75rem;">No output mappings yet. Click "+ Mapping" or use "1:1 Map" to auto-generate.</td></tr>';
    }

    html += '</tbody></table>';
    return html;
  }

  // ── Render: Sound Presets ──

  function renderSoundPresetsForDef(def) {
    var matching = S.data.soundPresets.filter(function(p) {
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

    var paramNames = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramNames[p.idx] = p.name || ('Param ' + p.idx);
      });
    });

    html += '<table class="mapping-table">';
    html += '<thead><tr><th>Preset</th><th>Group</th><th>Values</th><th></th></tr></thead>';
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

    // Knob preview group collapse/expand
    container.querySelectorAll('.designer-knob-preview .macro-group-header').forEach(function(header) {
      header.addEventListener('click', function() {
        header.closest('.macro-group').classList.toggle('collapsed');
      });
    });

    // ── Helper: update target panel values in-place ──
    function updateTargetPanel(cell, def, paramIdx, knobValue) {
      var panel = cell.querySelector('.knob-target-panel');
      if (!panel) return;
      var outputs = S.computeMappingOutputs(def, paramIdx, knobValue);
      outputs.forEach(function(o) {
        var row = panel.querySelector('.knob-target-row[data-ctrl="' + o.ctrl + '"]');
        if (!row) return;
        var valEl = row.querySelector('.knob-target-val');
        var fillEl = row.querySelector('.knob-target-fill');
        if (valEl) valEl.textContent = o.value;
        if (fillEl) fillEl.style.width = o.pct + '%';
      });
    }

    // ── Interactive knobs in designer preview ──
    container.querySelectorAll('.designer-knob-preview .macro-knob').forEach(function(knob) {
      var cell = knob.closest('.macro-knob-cell');
      if (!cell) return;
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
        if (valueEl) valueEl.textContent = newVal;

        var knobColor = knob.getAttribute('data-color') || 'normal';
        knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: knobColor, size: 52 });

        // Update target panel real-time values
        if (state.editDef) {
          updateTargetPanel(cell, state.editDef, paramIdx, newVal);
        }
      }

      function onPointerUp() {
        knob.classList.remove('dragging');
        document.removeEventListener('pointermove', onPointerMove);
        document.removeEventListener('pointerup', onPointerUp);

        // Update the editDef's default value to reflect the change
        var value = parseInt(knob.getAttribute('data-value'), 10);
        if (state.editDef) {
          state.editDef.groups.forEach(function(g) {
            (g.parameters || []).forEach(function(p) {
              if (p.idx === paramIdx) p.def = value;
            });
          });
          state.dirty = true;
        }
      }

      knob.addEventListener('pointerdown', onPointerDown);
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
    // Machine is now controlled from toolbar, no need for change listener
    if (defMachineSelect) {
      defMachineSelect.value = state.editDef ? state.editDef.machine : '';
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
          if (input.classList.contains('param-curve')) param.curve = input.value;
          if (input.classList.contains('param-ui')) param.ui = input.value;
          state.dirty = true;
        });
      });
    });

    // Add parameter
    container.querySelectorAll('.add-param-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;

        var maxIdx = -1;
        state.editDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx > maxIdx) maxIdx = p.idx;
          });
        });

        state.editDef.groups[gi].parameters.push({
          idx: maxIdx + 1,
          name: 'New Param',
          def: 0, min: 0, max: 127, res: 64, curve: 'linear', ui: 'bignum',
        });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Remove parameter
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
          renderMappingEditor();
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

    // Create preset
    var createPresetBtn = container.querySelector('.create-preset-btn');
    if (createPresetBtn) {
      createPresetBtn.addEventListener('click', function() {
        createSoundPresetForDef();
      });
    }

    // Delete preset
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

    var machineInfo = S.getMachineInfo(state.editDef.machine);
    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      S.toast('Machine has no CC parameters', 'warning', 2000);
      return;
    }

    if (!confirm('This will replace all parameter groups and output mappings with a 1:1 mapping from all ' + machineInfo.parameters.length + ' CCs. Continue?')) {
      return;
    }

    var params = machineInfo.parameters;

    if (!state.editDef.id) {
      state.editDef.id = state.editDef.machine + '-allparams';
    }
    if (!state.editDef.name) {
      state.editDef.name = (machineInfo.name || state.editDef.machine) + ' All params';
    }

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
          min: 0, max: 127, res: 64, curve: 'linear', ui: 'bignum',
        });
        paramIdx++;
      }
      state.editDef.groups.push({
        name: 'Page ' + (g + 1),
        parameters: groupParams,
      });
    }

    state.editDef.mapping = [];
    var allParams = [];
    state.editDef.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) { allParams.push(p); });
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
      return S.reloadMacroData();
    }).then(function() {
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
      return S.reloadMacroData();
    }).then(function() {
      renderMappingEditor();
    }).catch(function(err) {
      S.toast('Delete failed: ' + err.message, 'danger', 3000);
    });
  }

  // ─── DSP Panel (right) ──────────────────────────────────

  function renderDSPPanel() {
    var container = document.getElementById('dsp-param-list');
    if (!container) return;

    var machineId = state.editDef ? state.editDef.machine : '';
    var machineInfo = S.getMachineInfo(machineId);

    if (!machineInfo || !machineInfo.parameters || machineInfo.parameters.length === 0) {
      container.innerHTML =
        '<div class="empty-state" style="padding:1.5rem;">' +
        '<sl-icon name="cpu" style="font-size:1.5rem;"></sl-icon>' +
        '<p style="font-size:0.78rem;">Select a definition with a machine to see DSP CC parameters</p>' +
        '</div>';
      return;
    }

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
      return S.reloadMacroData();
    }).then(function() {
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
            state.editDef = data;
            state.selectedDefId = data.id || null;
            ensureGroupStructure(state.editDef);
            state.dirty = true;
            renderMappingEditor();
            renderDSPPanel();
            S.toast('Imported definition: ' + (data.name || data.id || 'unknown'), 'success', 2000);
          } else if (data.id && data.macro && data.values) {
            var filePath = 'macrosoundpresets/' + data.id + '.json';
            fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
              method: 'POST',
              headers: { 'Content-Type': 'application/json' },
              body: JSON.stringify(data, null, 2),
            }).then(function(r) {
              if (!r.ok) throw new Error('HTTP ' + r.status);
              S.toast('Imported preset: ' + data.name, 'success', 2000);
              return S.reloadMacroData();
            }).then(function() {
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
    // Register for shared track selection events
    S.onTrackChange(function(idx, track) {
      onTrackSelected(idx, track);
    });

    setupMachineSelectToolbarEvents();
    setupMachineListEvents();
    setupToolbarActions();

    // If a track is already selected (user was in Performer first), use it
    if (S.data.activeTrack >= 0) {
      var track = S.data.tracks.find(function(t) { return t.index === S.data.activeTrack; });
      if (track) onTrackSelected(S.data.activeTrack, track);
    } else if (S.data.tracks.length > 0) {
      // Auto-select first track
      S.selectTrack(S.data.tracks[0].index);
    }

    state.initialized = true;
  }

  // ─── Exports ─────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.designer = {
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
