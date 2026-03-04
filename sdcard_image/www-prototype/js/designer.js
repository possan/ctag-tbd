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
    html += '<label>NAME:</label>';
    html += '<input class="mapping-input def-name-input" value="' + S.esc(def.name) + '" placeholder="e.g. My Patch" />';
    html += '<label>ID:</label>';
    html += '<input class="mapping-input def-id-input" value="' + S.esc(def.id) + '" placeholder="auto-generated from name" ' + (state.selectedDefId ? '' : 'readonly') + ' />';
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
    html += '<button class="mapping-tab" data-tab="builder"><sl-icon name="wrench" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Macro Builder</button>';
    html += '<button class="mapping-tab" data-tab="presets"><sl-icon name="collection" style="font-size:0.7rem;margin-right:0.2rem;"></sl-icon> Sound Presets</button>';
    html += '</div>';

    // ── TAB: Knob Preview ──
    html += '<div class="mapping-tab-content active" data-tab="preview">';
    html += renderKnobPreview(def);
    html += '</div>';

    // ── TAB: Macro Builder (merged Parameter Groups + Output Mappings) ──
    html += '<div class="mapping-tab-content" data-tab="builder">';
    html += '<div class="tab-description">';
    html += '<sl-icon name="info-circle"></sl-icon>';
    html += 'Define knobs (up to 6 pages \u00d7 4 knobs) and map them to DSP parameters. Drag knobs to preview. The colored dot on each range track shows the current computed CC value.';
    html += '</div>';
    html += renderMacroBuilder(def, machineParams);
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
      html += '<span class="macro-group-page-label">Page ' + (gi + 1) + '</span>';
      html += '<span class="macro-group-name">' + S.esc(group.name || '') + '</span>';
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
        html += '<span class="macro-knob-label">' + S.esc(param.name || ('P' + param.idx)) + '</span>';
        html += '<div class="macro-knob" ';
        html += 'data-value="' + value + '" data-min="' + min + '" data-max="' + max + '" data-idx="' + param.idx + '" data-color="' + knobColor + '">';
        html += S.renderKnobSVG({ value: value, min: min, max: max, color: knobColor, size: 64 });
        html += '</div>';
        html += '<span class="macro-knob-value' + (isMacro ? ' is-macro' : '') + '">' + value + '</span>';

        // Show mapping targets with real-time computed values and range bars
        var targets = mappingInfo[param.idx] || [];
        if (targets.length > 0) {
          var outputs = S.computeMappingOutputs(def, param.idx, value);
          var panelClass = isMacro ? 'knob-target-panel is-macro' : 'knob-target-panel';
          html += '<div class="' + panelClass + '" data-knob-idx="' + param.idx + '">';
          if (isMacro) html += '<div class="knob-target-badge">MACRO</div>';

          outputs.forEach(function(o) {
            // Compute the range for this target
            var mapping = def.mapping.find(function(mm) { return mm.ctrl === o.ctrl; });
            var rangeLow = 0, rangeHigh = 127;
            var sourceCurve = '';
            if (mapping && mapping.add) {
              var singleSrc = mapping.add.length === 1;
              if (singleSrc) {
                rangeLow = mapping.start || 0;
                var a = mapping.add[0];
                rangeHigh = rangeLow + Math.round(127 * (a.mul || 1) / (a.div || 1));
                rangeHigh = Math.min(127, rangeHigh);
                sourceCurve = a.curve || '';
              } else {
                // Multi-source: show total range
                rangeLow = mapping.start || 0;
                rangeHigh = rangeLow;
                mapping.add.forEach(function(a) {
                  rangeHigh += Math.round(127 * (a.mul || 1) / (a.div || 1));
                  // Find curve for this specific source
                  if (a.src === param.idx) sourceCurve = a.curve || '';
                });
                rangeHigh = Math.min(127, rangeHigh);
              }
            }
            var rangeLowPct = rangeLow / 127 * 100;
            var rangeWidthPct = (rangeHigh - rangeLow) / 127 * 100;
            var valuePct = o.value / 127 * 100;

            html += '<div class="knob-target-row" data-ctrl="' + o.ctrl + '">';
            html += '<span class="knob-target-name">' + S.esc(o.name) + '</span>';
            html += '<span class="knob-target-bar">';
            html += '<span class="knob-target-range" style="left:' + rangeLowPct + '%;width:' + rangeWidthPct + '%"></span>';
            html += '<span class="knob-target-dot" style="left:' + valuePct + '%"></span>';
            html += '</span>';
            // Format value with display hints if available
            var targetDH = window.TBD && window.TBD.displayHints;
            var targetFmt = String(o.value);
            if (targetDH && def.machine) {
              var targetParamId = def.machine + '_' + S.esc(o.name).replace(/[- ]/g, '_');
              var targetHint = targetDH.resolveHint(targetParamId, o.name);
              if (targetHint) {
                var physVal = targetDH.rawToDisplay(o.value, 0, 127, targetHint);
                targetFmt = targetDH.formatDisplayValue(physVal, targetHint);
              }
            }
            html += '<span class="knob-target-val">' + targetFmt + '</span>';
            // Show 14-bit badge if applicable
            if (mapping && mapping.bits === 14) {
              html += '<span class="knob-target-14bit">14-bit</span>';
            }
            html += '</div>';

            // Show curve badge if non-linear (from mapping source, not parameter)
            if (sourceCurve && sourceCurve !== 'linear') {
              html += '<span class="curve-badge">' + S.esc(sourceCurve) + '</span>';
            }
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
      html += '<p>Add parameters in the "Macro Builder" tab to see a knob preview here.</p>';
      html += '</div>';
    }

    html += '</div>';
    return html;
  }

  // ── Render: Macro Builder (merged Parameter Groups + Output Mappings) ──

  function renderMacroBuilder(def, machineParams) {
    var mappings = def.mapping || [];
    var DH = window.TBD && window.TBD.displayHints;
    var html = '';

    // Build lookup maps
    var paramsByIdx = {};
    def.groups.forEach(function(g) {
      (g.parameters || []).forEach(function(p) {
        paramsByIdx[p.idx] = p;
      });
    });

    var ccLookup = {};
    machineParams.forEach(function(p) {
      ccLookup[p.ctrl] = p;
    });

    // ── Group mappings by source knob ──
    var paramMappings = {};  // paramIdx → [{mi, ai, mapping}]
    var constants = [];
    var multiSourceMappings = [];

    mappings.forEach(function(m, mi) {
      var sources = m.add || [];
      if (sources.length === 0) {
        constants.push({ mi: mi, mapping: m });
      } else if (sources.length === 1) {
        var src = sources[0].src;
        if (!paramMappings[src]) paramMappings[src] = [];
        paramMappings[src].push({ mi: mi, ai: 0, mapping: m });
      } else {
        multiSourceMappings.push({ mi: mi, mapping: m });
        // Also index multi-source entries per param for display in knob cards
        sources.forEach(function(a, ai) {
          if (!paramMappings[a.src]) paramMappings[a.src] = [];
          paramMappings[a.src].push({ mi: mi, ai: ai, mapping: m });
        });
      }
    });

    // Helper: format CC number with zero-padding
    function fmtCC(ctrl) {
      return 'CC\u2009' + String(ctrl).padStart(2, '0');
    }

    // Helper: get display hint + formatted range string for a CC param
    function getSemanticInfo(ctrl, rangeLow, rangeHigh) {
      var mp = ccLookup[ctrl];
      if (!mp || !DH) return { unit: '', rangeStr: '', hint: null };
      var paramId = (def.machine || '') + '_' + (mp.id || '').replace(/-/g, '_');
      var hint = DH.resolveHint(paramId, mp.name, mp);
      if (!hint) return { unit: '', rangeStr: '', hint: null };

      var physLow = DH.rawToDisplay(rangeLow, 0, 127, hint);
      var physHigh = DH.rawToDisplay(rangeHigh, 0, 127, hint);
      var fmtLow = DH.formatDisplayValue(physLow, hint);
      var fmtHigh = DH.formatDisplayValue(physHigh, hint);
      return {
        unit: hint.unit || '',
        rangeStr: fmtLow + ' \u2192 ' + fmtHigh,
        hint: hint,
        scale: hint.scale || 'lin'
      };
    }

    // Helper: render an interactive knob wrapped in om-knob div
    function renderKnob(paramIdx, param, size, color) {
      var val = param ? (param.def || 0) : 0;
      var mn = param ? (param.min || 0) : 0;
      var mx = param ? (param.max || 127) : 127;
      var name = param ? (param.name || ('P' + paramIdx)) : ('P' + paramIdx);
      var h = '<div class="om-knob om-knob-interactive" data-value="' + val + '" data-min="' + mn + '" data-max="' + mx + '" data-idx="' + paramIdx + '" data-color="' + color + '" title="' + S.esc(name) + ' \u2014 drag up/down">';
      h += S.renderKnobSVG({ value: val, min: mn, max: mx, color: color, size: size });
      h += '</div>';
      return h;
    }

    // Helper: compute value dot CC position for a parameter + mapping
    function computeValueDot(param, mapping, ai) {
      var addEntry = mapping.add[ai];
      if (!addEntry) return null;
      var is14 = mapping.bits === 14;
      var maxCC = is14 ? 16383 : 127;
      var start = mapping.start || 0;
      var mul = addEntry.mul || 1;
      var div = addEntry.div || 1;
      var paramVal = param ? (param.def || 0) : 0;
      var ccVal = start + Math.round(paramVal * mul / div);
      ccVal = Math.max(0, Math.min(maxCC, ccVal));
      return { cc: ccVal, maxCC: maxCC };
    }

    // Helper: render a single CC target row with range slider + value dot + 14-bit toggle
    function renderCCRow(mi, ai, m, addEntry) {
      var ctrl = m.ctrl;
      var mp = ccLookup[ctrl];
      var ccName = mp ? mp.name : '?';
      var is14 = m.bits === 14;
      var maxCC = is14 ? 16383 : 127;
      var range = sourceToRange(m, ai);
      var curve = addEntry.curve || 'linear';
      var lowPct = range.low / maxCC * 100;
      var highPct = range.high / maxCC * 100;
      var sem = getSemanticInfo(ctrl, range.low, range.high);

      // Compute value dot for the source param
      var srcParam = paramsByIdx[addEntry.src];
      var dot = computeValueDot(srcParam, m, ai);

      var r = '';
      r += '<div class="om-cc-row" data-mapping-idx="' + mi + '" data-add="' + ai + '">';

      // CC label + name
      r += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
      r += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';

      // Range low input
      r += '<input type="number" class="mapping-input om-range-low' + (is14 ? ' is-14bit' : '') + '" value="' + range.low + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="Low' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';

      // Range track with thumbs + value dot
      r += '<div class="om-range-track" data-mapping="' + mi + '" data-add="' + ai + '">';
      r += '<div class="om-range-fill" style="left:' + lowPct + '%;width:' + (highPct - lowPct) + '%"></div>';
      r += '<div class="om-range-thumb om-thumb-low" style="left:' + lowPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      r += '<div class="om-range-thumb om-thumb-high" style="left:' + highPct + '%" data-mapping="' + mi + '" data-add="' + ai + '"></div>';
      if (dot) {
        var dotPct = (dot.maxCC > 0) ? (dot.cc / dot.maxCC * 100) : 0;
        r += '<div class="om-value-dot" style="left:' + dotPct + '%" data-mapping="' + mi + '" data-add="' + ai + '" title="Current CC value: ' + dot.cc + '"></div>';
      }
      r += '</div>';

      // Range high input
      r += '<input type="number" class="mapping-input om-range-high' + (is14 ? ' is-14bit' : '') + '" value="' + range.high + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" data-add="' + ai + '" title="High' + (is14 ? ' (0\u201316383)' : ' (0\u2013127)') + '" />';

      // Curve select
      r += '<select class="mapping-select om-curve-select" data-mapping="' + mi + '" data-add="' + ai + '" title="Response curve">';
      ['linear','log','exp','scurve'].forEach(function(c) {
        r += '<option value="' + c + '"' + (curve === c ? ' selected' : '') + '>' + c + '</option>';
      });
      r += '</select>';

      // 14-bit toggle
      r += '<label class="om-bit-toggle" title="Enable 14-bit CC (0\u201316383) for higher precision">';
      r += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
      r += '<span>14-bit</span>';
      r += '</label>';

      // Remove mapping button
      r += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove this CC mapping">\u00d7</button>';
      r += '</div>';

      // Semantic range info + scale hint (only show log badge if mapping curve is 'linear' — to hint that a log curve would be better)
      if (sem.rangeStr) {
        r += '<div class="om-semantic-row">';
        r += '<span class="om-semantic-range">' + sem.rangeStr + '</span>';
        if (sem.scale === 'log' && curve === 'linear') {
          r += '<a class="om-scale-hint om-scale-fix" href="#" data-mapping="' + mi + '" data-add="' + ai + '" title="This DSP parameter has a logarithmic scale. Click to switch the curve to log.">\ud83d\udca1 use log curve</a>';
        }
        r += '</div>';
      }

      return r;
    }

    // ── Render page sections (groups) ──
    def.groups.forEach(function(group, gi) {
      html += '<div class="mb-page-section" data-group-idx="' + gi + '">';
      html += '<div class="mb-page-header">';
      html += '<span class="mb-page-icon"><sl-icon name="grid-3x3-gap" style="font-size:0.7rem;"></sl-icon></span>';
      html += '<span class="mb-page-label">Page ' + (gi + 1) + '</span>';
      html += '<input class="mapping-input mb-group-name" value="' + S.esc(group.name) + '" data-group="' + gi + '" placeholder="Name (optional)" />';
      html += '<span class="mb-page-info">' + (group.parameters || []).length + '/4 knobs</span>';
      html += '<div class="om-card-spacer"></div>';
      if ((group.parameters || []).length < 4) {
        html += '<button class="mapping-add-btn mb-add-knob-btn" data-group="' + gi + '" title="Add a new knob parameter">+ Add Knob</button>';
      }
      html += '</div>';

      html += '<div class="mb-page-content">';
      // Render each parameter as a knob card
      (group.parameters || []).forEach(function(param, pi) {
        var paramIdx = param.idx;
        var entries = paramMappings[paramIdx] || [];
        var isMacro = entries.length >= 2;
        var knobColor = isMacro ? 'macro' : 'normal';

        html += '<div class="om-knob-card' + (isMacro ? ' is-macro' : '') + '" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '">';

        // ── Card header: drag handle + knob badge + name/knob/value + actions ──
        html += '<div class="om-knob-header">';
        html += '<span class="om-drag-handle" title="Drag to reorder">⫶</span>';
        html += '<span class="om-knob-badge">Knob ' + (pi + 1) + '</span>';
        html += '<div class="om-knob-cell">';
        html += '<input class="mapping-input mb-param-name" value="' + S.esc(param.name) + '" data-group="' + gi + '" data-param="' + pi + '" placeholder="Knob name" />';
        html += renderKnob(paramIdx, param, 64, knobColor);
        html += '<span class="om-knob-value' + (isMacro ? ' is-macro' : '') + '">' + (param.def || 0) + '</span>';
        html += '</div>';
        html += '<div class="om-card-spacer"></div>';
        if (isMacro) {
          html += '<sl-badge class="om-macro-badge" variant="warning" size="small">MACRO \u00b7 ' + entries.length + '</sl-badge>';
        }
        html += '<select class="mapping-select mapping-add-cc-for-knob" data-src-idx="' + paramIdx + '" title="Map this knob to another CC">';
        html += '<option value="">+ map to CC\u2026</option>';
        machineParams.forEach(function(mp) {
          html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
        });
        html += '</select>';
        html += '<button class="mapping-remove-btn mb-remove-knob-btn" data-group="' + gi + '" data-param="' + pi + '" data-param-idx="' + paramIdx + '" title="Remove this knob">\u00d7</button>';
        html += '</div>';

        // ── Properties row ──
        html += '<div class="mb-props-row">';
        html += '<label class="mb-prop"><span>def</span><input type="number" class="mapping-input mb-prop-def" value="' + (param.def || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>min</span><input type="number" class="mapping-input mb-prop-min" value="' + (param.min || 0) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>max</span><input type="number" class="mapping-input mb-prop-max" value="' + (param.max || 127) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>res</span><input type="number" class="mapping-input mb-prop-res" value="' + (param.res || 64) + '" data-group="' + gi + '" data-param="' + pi + '" /></label>';
        html += '<label class="mb-prop"><span>ui</span><select class="mapping-select mb-prop-ui" data-group="' + gi + '" data-param="' + pi + '">';
        ['bignum', 'slider', 'toggle', 'selector'].forEach(function(ui) {
          html += '<option value="' + ui + '"' + (param.ui === ui ? ' selected' : '') + '>' + ui + '</option>';
        });
        html += '</select></label>';
        html += '</div>';

        // ── CC mapping rows ──
        html += '<div class="om-knob-body">';
        if (entries.length === 0) {
          html += '<div class="mb-no-mappings">No CC mappings \u2014 use "+ map to CC\u2026" above</div>';
        }
        entries.forEach(function(entry) {
          html += renderCCRow(entry.mi, entry.ai, entry.mapping, entry.mapping.add[entry.ai]);
        });

        // (+ map to CC dropdown is now in the card header)
        html += '</div>'; // om-knob-body
        html += '</div>'; // om-knob-card
      });

      if (!group.parameters || group.parameters.length === 0) {
        html += '<div class="mb-empty-group">No knobs in this page \u2014 click "+ Add Knob" to create one</div>';
      }
      html += '</div>'; // mb-page-content

      html += '</div>'; // mb-page-section
    });

    // Add Page button
    if (def.groups.length < 6) {
      html += '<div style="text-align:center;margin:0.6rem 0;">';
      html += '<button class="mapping-add-btn mb-add-group-btn" title="Add a new knob page (up to 6)">+ Add Page</button>';
      html += '</div>';
    }

    // ── Render constants (locked parameters) card ──
    if (constants.length > 0) {
      html += '<div class="om-card om-constants-card">';
      html += '<div class="om-card-header">';
      html += '<sl-icon name="lock" style="font-size:0.85rem;color:var(--sl-color-neutral-500);"></sl-icon>';
      html += '<span class="om-cc-name" style="font-weight:700;">Locked Parameters</span>';
      html += '<div class="om-card-spacer"></div>';
      html += '<sl-badge variant="neutral" size="small">' + constants.length + ' locked</sl-badge>';
      html += '</div>';
      html += '<div class="om-card-body">';

      constants.forEach(function(entry) {
        var m = entry.mapping;
        var mi = entry.mi;
        var ctrl = m.ctrl;
        var mp = ccLookup[ctrl];
        var ccName = mp ? mp.name : '?';
        var is14 = m.bits === 14;
        var maxCC = is14 ? 16383 : 127;
        var fixedVal = m.start || 0;
        var fixedPct = fixedVal / maxCC * 100;
        var sem = getSemanticInfo(ctrl, fixedVal, fixedVal);

        html += '<div class="om-constant-row" data-mapping-idx="' + mi + '">';
        html += '<span class="om-cc-label">' + fmtCC(ctrl) + '</span>';
        html += '<span class="om-cc-name">' + S.esc(ccName) + '</span>';
        html += '<input type="number" class="mapping-input om-fixed-input' + (is14 ? ' is-14bit' : '') + '" value="' + fixedVal + '" min="0" max="' + maxCC + '" data-mapping="' + mi + '" />';
        html += '<div class="om-range-track" title="Fixed CC value">';
        html += '<div class="om-range-mark" style="left:' + fixedPct + '%"></div>';
        html += '</div>';
        if (sem.rangeStr) {
          html += '<span class="om-semantic-val">' + S.esc(sem.rangeStr.split(' \u2192 ')[0]) + '</span>';
        }
        html += '<label class="om-bit-toggle" title="Enable 14-bit CC">';
        html += '<input type="checkbox" class="om-14bit-check" data-mapping="' + mi + '"' + (is14 ? ' checked' : '') + ' />';
        html += '<span>14-bit</span>';
        html += '</label>';
        html += '<button class="mapping-remove-btn remove-mapping-btn" data-mapping="' + mi + '" title="Remove"><sl-icon name="x-circle"></sl-icon></button>';
        html += '</div>';
      });

      html += '</div>';
      html += '</div>';
    }

    // ── Unmapped CC add dropdown ──
    html += '<div style="margin-top:0.5rem;text-align:center;">';
    html += '<select class="mapping-select add-unmapped-cc-select" title="Add a constant (locked) CC mapping">';
    html += '<option value="">+ add locked CC\u2026</option>';
    machineParams.forEach(function(mp) {
      var alreadyMapped = mappings.some(function(m) { return m.ctrl === mp.ctrl; });
      if (!alreadyMapped) {
        html += '<option value="' + mp.ctrl + '">' + fmtCC(mp.ctrl) + ' ' + S.esc(mp.name) + '</option>';
      }
    });
    html += '</select>';
    html += '</div>';

    return html;
  }

  // ── Helpers: range ↔ start/mul/div conversion ──

  /**
   * Convert a mapping + source index to low/high CC range for the UI.
   * Single source: low = start, high = start + 127*mul/div
   * Multi source: shows per-source contribution (0 to max)
   * Respects mapping.bits for 14-bit CC support.
   */
  function sourceToRange(mapping, addIdx) {
    var add = (mapping.add || [])[addIdx];
    var maxCC = (mapping.bits === 14) ? 16383 : 127;
    if (!add) return { low: mapping.start || 0, high: mapping.start || 0 };
    var mul = add.mul || 1;
    var div = add.div || 1;
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      var low = mapping.start || 0;
      var high = low + Math.round(127 * mul / div);
      return { low: Math.max(0, Math.min(maxCC, low)), high: Math.max(0, Math.min(maxCC, high)) };
    } else {
      // Per-source contribution (0 to max_contribution)
      var maxC = Math.round(127 * mul / div);
      return { low: 0, high: Math.max(0, Math.min(maxCC, maxC)), base: mapping.start || 0 };
    }
  }

  /**
   * Write low/high back to mapping start/mul/div.
   */
  function rangeToSource(mapping, addIdx, low, high) {
    var singleSource = (mapping.add || []).length === 1;
    if (singleSource) {
      mapping.start = low;
      mapping.add[addIdx].mul = high - low;
      mapping.add[addIdx].div = 127;
    } else {
      // Multi-source: only update this source's contribution
      mapping.add[addIdx].mul = high;
      mapping.add[addIdx].div = 127;
    }
  }

  // ── Render: Sound Presets ──

  // ─── Sortable: Knob reordering within pages ──────────────

  var knobSortableInstances = [];

  function setupKnobSortables(container) {
    // Destroy previous instances
    knobSortableInstances.forEach(function(s) { try { s.destroy(); } catch(e) {} });
    knobSortableInstances = [];

    if (typeof Sortable === 'undefined') return;

    container.querySelectorAll('.mb-page-content').forEach(function(pageContent) {
      var section = pageContent.closest('.mb-page-section');
      if (!section) return;
      var gi = parseInt(section.getAttribute('data-group-idx'), 10);

      var inst = Sortable.create(pageContent, {
        handle: '.om-drag-handle',
        animation: 150,
        ghostClass: 'sortable-ghost',
        chosenClass: 'sortable-chosen',
        draggable: '.om-knob-card',
        onEnd: function(evt) {
          if (evt.oldIndex !== evt.newIndex) {
            reorderKnobInGroup(gi, evt.oldIndex, evt.newIndex);
          }
        },
      });
      knobSortableInstances.push(inst);
    });
  }

  /**
   * Move a knob from oldPos to newPos within a page group.
   * The param.idx values stay unchanged (they are the mapping identity keys).
   * Only the array position changes, which determines display order.
   * Re-renders editor so Knob Preview also reflects the new order.
   */
  function reorderKnobInGroup(groupIdx, oldPos, newPos) {
    if (!state.editDef) return;
    var group = state.editDef.groups[groupIdx];
    if (!group || !group.parameters) return;

    // Splice to reorder — param.idx values are NOT changed (they're mapping keys)
    var moved = group.parameters.splice(oldPos, 1)[0];
    group.parameters.splice(newPos, 0, moved);

    state.dirty = true;
    renderMappingEditor();
    renderDSPPanel();
  }

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
      html += '<tr class="mapping-row" data-preset-id="' + S.esc(preset.id) + '">';
      html += '<td><input class="mapping-input preset-name-input" value="' + S.esc(preset.name || preset.id) + '" data-preset-id="' + S.esc(preset.id) + '" style="width:120px;" /></td>';
      html += '<td><input class="mapping-input preset-group-input" value="' + S.esc(preset.group || '') + '" data-preset-id="' + S.esc(preset.id) + '" style="width:80px;" /></td>';
      html += '<td class="preset-values-cell">';
      if (preset.values && preset.values.length > 0) {
        preset.values.forEach(function(v, vi) {
          var pName = paramNames[vi] || vi;
          html += '<input class="mapping-input preset-value-input" type="number" value="' + v + '" data-preset-id="' + S.esc(preset.id) + '" data-value-idx="' + vi + '" title="' + S.esc(String(pName)) + '" style="width:42px;" />';
        });
      } else {
        html += '<span style="opacity:0.4;">empty</span>';
      }
      html += '</td>';
      html += '<td style="display:flex;gap:0.25rem;">';
      html += '<button class="mapping-btn save-preset-btn" data-preset-id="' + S.esc(preset.id) + '" title="Save changes" style="font-size:0.65rem;padding:0.15rem 0.4rem;"><sl-icon name="floppy"></sl-icon></button>';
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
      var targetDH = window.TBD && window.TBD.displayHints;
      outputs.forEach(function(o) {
        var row = panel.querySelector('.knob-target-row[data-ctrl="' + o.ctrl + '"]');
        if (!row) return;
        var valEl = row.querySelector('.knob-target-val');
        var dotEl = row.querySelector('.knob-target-dot');
        if (valEl) {
          var fmt = String(o.value);
          if (targetDH && def.machine) {
            var pid = def.machine + '_' + o.name.replace(/[- ]/g, '_');
            var hint = targetDH.resolveHint(pid, o.name);
            if (hint) {
              var physVal = targetDH.rawToDisplay(o.value, 0, 127, hint);
              fmt = targetDH.formatDisplayValue(physVal, hint);
            }
          }
          valEl.textContent = fmt;
        }
        if (dotEl) dotEl.style.left = (o.value / 127 * 100) + '%';
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
        knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: knobColor, size: 64 });

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
      defNameInput.addEventListener('input', function() {
        if (state.editDef) {
          state.editDef.name = defNameInput.value;
          state.dirty = true;
          // Auto-generate ID from name for new definitions
          if (!state.selectedDefId && defIdInput) {
            var machinePrefix = state.editDef.machine ? (state.editDef.machine.substring(0, 2) + '-') : '';
            var slug = defNameInput.value.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
            var autoId = machinePrefix + slug;
            state.editDef.id = autoId;
            defIdInput.value = autoId;
          }
        }
      });
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

    // ── Macro Builder: group name changes ──
    container.querySelectorAll('.mb-group-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        if (state.editDef && state.editDef.groups[gi]) {
          state.editDef.groups[gi].name = input.value;
          state.dirty = true;
        }
      });
    });

    // Macro Builder: parameter property changes (name, def, min, max, res, ui)
    container.querySelectorAll('.mb-param-name').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.name = input.value; state.dirty = true; }
      });
    });
    container.querySelectorAll('.mb-prop-def, .mb-prop-min, .mb-prop-max, .mb-prop-res').forEach(function(input) {
      input.addEventListener('change', function() {
        var gi = parseInt(input.getAttribute('data-group'), 10);
        var pi = parseInt(input.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (!param) return;
        var v = parseInt(input.value, 10) || 0;
        if (input.classList.contains('mb-prop-def')) param.def = v;
        if (input.classList.contains('mb-prop-min')) param.min = v;
        if (input.classList.contains('mb-prop-max')) param.max = v;
        if (input.classList.contains('mb-prop-res')) param.res = v;
        state.dirty = true;
        renderMappingEditor();
      });
    });
    container.querySelectorAll('.mb-prop-ui').forEach(function(select) {
      select.addEventListener('change', function() {
        var gi = parseInt(select.getAttribute('data-group'), 10);
        var pi = parseInt(select.getAttribute('data-param'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        var param = state.editDef.groups[gi].parameters[pi];
        if (param) { param.ui = select.value; state.dirty = true; }
      });
    });

    // Macro Builder: add knob to a group
    container.querySelectorAll('.mb-add-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;
        if ((state.editDef.groups[gi].parameters || []).length >= 4) return;

        var maxIdx = -1;
        state.editDef.groups.forEach(function(g) {
          (g.parameters || []).forEach(function(p) {
            if (p.idx > maxIdx) maxIdx = p.idx;
          });
        });

        state.editDef.groups[gi].parameters.push({
          idx: maxIdx + 1,
          name: 'New Knob',
          def: 0, min: 0, max: 127, res: 64, curve: 'linear', ui: 'bignum',
        });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Macro Builder: remove knob (and its mappings)
    container.querySelectorAll('.mb-remove-knob-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var gi = parseInt(btn.getAttribute('data-group'), 10);
        var pi = parseInt(btn.getAttribute('data-param'), 10);
        var paramIdx = parseInt(btn.getAttribute('data-param-idx'), 10);
        if (!state.editDef || !state.editDef.groups[gi]) return;

        // Remove the parameter
        state.editDef.groups[gi].parameters.splice(pi, 1);

        // Remove all mappings that reference this paramIdx as a source
        if (state.editDef.mapping) {
          state.editDef.mapping = state.editDef.mapping.filter(function(m) {
            if (!m.add || m.add.length === 0) return true;
            // Remove sources referencing this param
            m.add = m.add.filter(function(a) { return a.src !== paramIdx; });
            // Keep the mapping if it still has sources or is a constant
            return m.add.length > 0 || (m.start !== undefined);
          });
        }
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // ── Sortable.js: make knob cards within each page draggable ──
    setupKnobSortables(container);

    // Macro Builder: add page
    container.querySelectorAll('.mb-add-group-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        if (!state.editDef) return;
        if (state.editDef.groups.length >= 6) return;
        state.editDef.groups.push({
          name: 'Page ' + (state.editDef.groups.length + 1),
          parameters: [],
        });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // ── Output mapping / CC row event handlers ──

    // Interactive knob drag (updates value dots in real-time)
    container.querySelectorAll('.om-knob-interactive').forEach(function(knob) {
      var paramIdx = parseInt(knob.getAttribute('data-idx'), 10);
      var min = parseInt(knob.getAttribute('data-min'), 10) || 0;
      var max = parseInt(knob.getAttribute('data-max'), 10) || 127;
      var startY = 0;
      var startVal = 0;

      knob.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        knob.classList.add('dragging');
        startY = e.clientY;
        startVal = parseInt(knob.getAttribute('data-value'), 10) || 0;

        function onMove(ev) {
          var dy = startY - ev.clientY;
          var range = max - min;
          var sensitivity = range / 200;
          var newVal = Math.round(startVal + dy * sensitivity);
          newVal = Math.max(min, Math.min(max, newVal));

          knob.setAttribute('data-value', newVal);
          var color = knob.getAttribute('data-color') || 'normal';
          var size = knob.querySelector('.knob-svg') ? parseInt(knob.querySelector('.knob-svg').getAttribute('width'), 10) : 32;
          knob.innerHTML = S.renderKnobSVG({ value: newVal, min: min, max: max, color: color, size: size });

          // Update value display
          var card = knob.closest('.om-knob-card');
          if (card) {
            var valEl = card.querySelector('.om-knob-value');
            if (valEl) valEl.textContent = newVal;

            // Update def input
            var defInput = card.querySelector('.mb-prop-def');
            if (defInput) defInput.value = newVal;

            // ── Move value dots on all CC rows in this card ──
            card.querySelectorAll('.om-value-dot').forEach(function(dot) {
              var mi = parseInt(dot.getAttribute('data-mapping'), 10);
              var ai = parseInt(dot.getAttribute('data-add'), 10);
              if (!state.editDef || !state.editDef.mapping[mi]) return;
              var mapping = state.editDef.mapping[mi];
              var addEntry = mapping.add && mapping.add[ai];
              if (!addEntry) return;
              var is14 = mapping.bits === 14;
              var maxCC = is14 ? 16383 : 127;
              var start = mapping.start || 0;
              var mul = addEntry.mul || 1;
              var div = addEntry.div || 1;
              var ccVal = start + Math.round(newVal * mul / div);
              ccVal = Math.max(0, Math.min(maxCC, ccVal));
              dot.style.left = (ccVal / maxCC * 100) + '%';
              dot.title = 'Current CC value: ' + ccVal;
            });
          }

          // Update editDef param default
          if (state.editDef) {
            state.editDef.groups.forEach(function(g) {
              (g.parameters || []).forEach(function(p) {
                if (p.idx === paramIdx) p.def = newVal;
              });
            });
            state.dirty = true;
          }
        }

        function onUp() {
          knob.classList.remove('dragging');
          document.removeEventListener('pointermove', onMove);
          document.removeEventListener('pointerup', onUp);
        }

        document.addEventListener('pointermove', onMove);
        document.addEventListener('pointerup', onUp);
      });
    });

    // Range low/high input changes (knob-card CC rows)
    container.querySelectorAll('.om-range-low, .om-range-high').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        var ai = parseInt(input.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        var maxCC = (mapping.bits === 14) ? 16383 : 127;

        // Find sibling inputs in the same row
        var row = input.closest('.om-cc-row, .om-source-row');
        var lowInput = row ? row.querySelector('.om-range-low') : null;
        var highInput = row ? row.querySelector('.om-range-high') : null;
        var low = Math.max(0, Math.min(maxCC, parseInt(lowInput ? lowInput.value : 0, 10) || 0));
        var high = Math.max(0, Math.min(maxCC, parseInt(highInput ? highInput.value : maxCC, 10) || 0));
        if (low > high) { var tmp = low; low = high; high = tmp; }
        rangeToSource(mapping, ai, low, high);
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Curve select changes
    container.querySelectorAll('.om-curve-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        var ai = parseInt(select.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) {
          addEntry.curve = select.value;
          state.dirty = true;
        }
      });
    });

    // Scale hint auto-fix (click to switch curve to log)
    container.querySelectorAll('.om-scale-fix').forEach(function(link) {
      link.addEventListener('click', function(e) {
        e.preventDefault();
        var mi = parseInt(link.getAttribute('data-mapping'), 10);
        var ai = parseInt(link.getAttribute('data-add'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var addEntry = state.editDef.mapping[mi].add[ai];
        if (addEntry) {
          addEntry.curve = 'log';
          state.dirty = true;
          renderMappingEditor();
        }
      });
    });

    // Fixed value inputs (constant mappings)
    container.querySelectorAll('.om-fixed-input').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = (state.editDef.mapping[mi].bits === 14) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Base start inputs (multi-source cards)
    container.querySelectorAll('.om-multi-card .mapping-start').forEach(function(input) {
      input.addEventListener('change', function() {
        var mi = parseInt(input.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var maxCC = (state.editDef.mapping[mi].bits === 14) ? 16383 : 127;
        state.editDef.mapping[mi].start = Math.max(0, Math.min(maxCC, parseInt(input.value, 10) || 0));
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // 14-bit toggle
    container.querySelectorAll('.om-14bit-check').forEach(function(checkbox) {
      checkbox.addEventListener('change', function() {
        var mi = parseInt(checkbox.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var mapping = state.editDef.mapping[mi];
        if (checkbox.checked) {
          mapping.bits = 14;
        } else {
          delete mapping.bits;
          // Clamp values back to 7-bit range
          if (mapping.start > 127) mapping.start = 127;
          (mapping.add || []).forEach(function(a) {
            if (a.mul > 127) a.mul = 127;
          });
        }
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Range slider thumb drag
    container.querySelectorAll('.om-range-thumb').forEach(function(thumb) {
      thumb.addEventListener('pointerdown', function(e) {
        e.preventDefault();
        thumb.setPointerCapture(e.pointerId);
        thumb.classList.add('dragging');

        var mi = parseInt(thumb.getAttribute('data-mapping'), 10);
        var ai = parseInt(thumb.getAttribute('data-add'), 10);
        var isLow = thumb.classList.contains('om-thumb-low');
        var track = thumb.closest('.om-range-track');
        if (!track) return;

        var parentRow = thumb.closest('.om-cc-row, .om-source-row');

        function onMove(ev) {
          if (!state.editDef || !state.editDef.mapping[mi]) return;
          var mapping = state.editDef.mapping[mi];
          var maxCC = (mapping.bits === 14) ? 16383 : 127;

          var rect = track.getBoundingClientRect();
          var pct = (ev.clientX - rect.left) / rect.width;
          pct = Math.max(0, Math.min(1, pct));
          var ccVal = Math.round(pct * maxCC);

          var range = sourceToRange(mapping, ai);
          var low = range.low, high = range.high;

          if (isLow) {
            low = Math.min(ccVal, high);
          } else {
            high = Math.max(ccVal, low);
          }

          rangeToSource(mapping, ai, low, high);
          state.dirty = true;

          // Update visuals without full re-render
          var newRange = sourceToRange(mapping, ai);
          var lowPct = newRange.low / maxCC * 100;
          var highPct = newRange.high / maxCC * 100;
          var fill = track.querySelector('.om-range-fill');
          var thumbLow = track.querySelector('.om-thumb-low');
          var thumbHigh = track.querySelector('.om-thumb-high');

          if (fill) { fill.style.left = lowPct + '%'; fill.style.width = (highPct - lowPct) + '%'; }
          if (thumbLow) thumbLow.style.left = lowPct + '%';
          if (thumbHigh) thumbHigh.style.left = highPct + '%';

          if (parentRow) {
            var lowInput = parentRow.querySelector('.om-range-low');
            var highInput = parentRow.querySelector('.om-range-high');
            if (lowInput) lowInput.value = newRange.low;
            if (highInput) highInput.value = newRange.high;
          }
        }

        function onUp() {
          thumb.classList.remove('dragging');
          thumb.removeEventListener('pointermove', onMove);
          thumb.removeEventListener('pointerup', onUp);
          renderMappingEditor();
        }

        thumb.addEventListener('pointermove', onMove);
        thumb.addEventListener('pointerup', onUp);
      });
    });

    // Add CC mapping for an existing knob (dropdown in knob card header)
    container.querySelectorAll('.mapping-add-cc-for-knob').forEach(function(select) {
      select.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(select.value, 10);
        var srcIdx = parseInt(select.getAttribute('data-src-idx'), 10);
        if (isNaN(ctrl) || isNaN(srcIdx)) return;
        state.editDef.mapping.push({ ctrl: ctrl, start: 0, add: [{ src: srcIdx, mul: 1, div: 1 }] });
        state.dirty = true;
        renderMappingEditor();
      });
    });

    // Add unmapped CC as constant (dropdown at bottom)
    var unmappedSelect = container.querySelector('.add-unmapped-cc-select');
    if (unmappedSelect) {
      unmappedSelect.addEventListener('change', function() {
        if (!state.editDef) return;
        var ctrl = parseInt(unmappedSelect.value, 10);
        if (isNaN(ctrl)) return;
        state.editDef.mapping.push({ ctrl: ctrl, start: 0, add: [] });
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

    // Add source to mapping (multi-source cards)
    container.querySelectorAll('.mapping-add-src-select').forEach(function(select) {
      select.addEventListener('change', function() {
        var mi = parseInt(select.getAttribute('data-mapping'), 10);
        if (!state.editDef || !state.editDef.mapping[mi]) return;
        var src = parseInt(select.value, 10);
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

    // Save edited preset values
    container.querySelectorAll('.save-preset-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var presetId = btn.getAttribute('data-preset-id');
        saveEditedPreset(presetId, container);
      });
    });

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

  /**
   * Save an edited preset — reads updated values from the inline inputs.
   */
  function saveEditedPreset(presetId, container) {
    var preset = S.data.soundPresets.find(function(p) { return p.id === presetId; });
    if (!preset) {
      S.toast('Preset not found', 'danger', 2000);
      return;
    }

    // Read updated name
    var nameInput = container.querySelector('.preset-name-input[data-preset-id="' + presetId + '"]');
    if (nameInput) preset.name = nameInput.value;

    // Read updated group
    var groupInput = container.querySelector('.preset-group-input[data-preset-id="' + presetId + '"]');
    if (groupInput) preset.group = groupInput.value;

    // Read updated values
    container.querySelectorAll('.preset-value-input[data-preset-id="' + presetId + '"]').forEach(function(input) {
      var vi = parseInt(input.getAttribute('data-value-idx'), 10);
      preset.values[vi] = parseInt(input.value, 10) || 0;
    });

    var jsonStr = JSON.stringify(preset, null, 2);
    var filePath = 'macrosoundpresets/' + presetId + '.json';

    fetch('/api/v1/samples?action=uploadconfig&path=' + encodeURIComponent(filePath), {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: jsonStr,
    }).then(function(r) {
      if (!r.ok) throw new Error('HTTP ' + r.status);
      S.toast('Saved preset: ' + preset.name, 'success', 2000);
      return S.reloadMacroData();
    }).then(function() {
      renderMappingEditor();
    }).catch(function(err) {
      S.toast('Save failed: ' + err.message, 'danger', 3000);
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
