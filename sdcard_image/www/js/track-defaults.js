// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Track Default Presets Editor (Overlay)
//
// Opens as a Shoelace dialog from the header nav.
// Hierarchy mirrors the main Preset & Macro Manager UI:
//   Track → Machine → Preset (grouped by Macro definition)
//
// Machine names come from S.getMachineInfo() (synthdefinitions.json).
// Machine list uses S.getTrackMachines() which filters out noX empties.
// Presets are grouped into <optgroup>s by their macro definition name,
// so the user sees e.g. "Phat Punch" / "Synth Kick — All knobs" sections.
//
// Data source:  /sdcard/data/trackdefaults.json
// API:          GET  /api/v2/macros?action=get_trackdefaults
//               POST /api/v2/macros?action=save_trackdefaults
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ─────────────────────────────────────────────────
  var trackDefaults = null;   // parsed trackdefaults.json
  var dirty = false;
  var facetedData = null;     // per-track: [ { machine, name, macros: [{id, name, presets}] } ]
  var sampleBankNames = [];   // from sample_rom.jsn via samples API

  // ─── Helpers ───────────────────────────────────────────────

  /**
   * Get display name for a machine ID using S.getMachineInfo()
   * (same source as the main UI's MACHINE: dropdown).
   */
  function getMachineName(machineId) {
    var info = S.getMachineInfo(machineId);
    return info ? info.name : machineId;
  }

  /**
   * Build faceted data for every track.
   * Uses S.getTrackMachines() to get the same machine list as the main UI
   * (filters out nodrum/nosynth/nofx).
   *
   * Returns { trackIndex: [ { machine, name, macros: [ { id, name, presets } ] } ] }
   */
  function buildFacetedData() {
    var tracks = S.data.tracks || [];
    var allDefs = S.data.macroDefs || [];
    var allPresets = S.data.soundPresets || [];

    var result = {};

    tracks.forEach(function(track) {
      // Use S.getTrackMachines() — same filter as the main UI MACHINE: dropdown
      var machines = S.getTrackMachines(track);
      var facets = [];

      machines.forEach(function(machineId) {
        // Find all macro definitions for this machine
        var defs = allDefs.filter(function(d) { return d.machine === machineId; });
        if (defs.length === 0) return;

        var macros = [];
        defs.forEach(function(def) {
          // Find all sound presets that use this macrodefinition
          var presets = allPresets.filter(function(p) { return p.macro === def.id; });
          if (presets.length === 0) return;
          presets.sort(function(a, b) {
            var na = (a.name || a.id).toLowerCase();
            var nb = (b.name || b.id).toLowerCase();
            return na < nb ? -1 : na > nb ? 1 : 0;
          });
          macros.push({ id: def.id, name: def.name || def.id, presets: presets });
        });

        // Only include machine if it has at least one preset
        if (macros.length > 0) {
          facets.push({
            machine: machineId,
            name: getMachineName(machineId),
            macros: macros
          });
        }
      });

      result[track.index] = facets;
    });

    return result;
  }

  /**
   * Given a preset ID, find which machine it belongs to within a track's facets.
   */
  function findMachineForPreset(presetId, facets) {
    for (var i = 0; i < facets.length; i++) {
      for (var j = 0; j < facets[i].macros.length; j++) {
        for (var k = 0; k < facets[i].macros[j].presets.length; k++) {
          if (facets[i].macros[j].presets[k].id === presetId) {
            return facets[i].machine;
          }
        }
      }
    }
    return '';
  }

  /**
   * Get the current default preset ID for a track from the loaded defaults.
   */
  function getDefaultPreset(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return '';
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry ? (entry.preset || '') : '';
  }

  /**
   * Get the saved sampleSlice for a track (rompler tracks only).
   */
  function getDefaultSlice(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return 0;
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry && typeof entry.sampleSlice === 'number' ? entry.sampleSlice : 0;
  }

  /**
   * Check if a track supports the rompler machine (has 'ro' in machines list).
   */
  function trackHasRompler(track) {
    return (track.machines || []).indexOf('ro') !== -1;
  }

  // ─── API ───────────────────────────────────────────────────

  /**
   * Fetch sample bank names from the samples API.
   */
  function loadSampleBankNames() {
    return S.queuedFetch('/samples')
      .then(function(data) {
        if (data && data.kits && data.kits.smp_bank_names) {
          sampleBankNames = data.kits.smp_bank_names;
        } else {
          sampleBankNames = [];
        }
        return sampleBankNames;
      })
      .catch(function() {
        sampleBankNames = [];
        return sampleBankNames;
      });
  }

  function loadTrackDefaults() {
    return loadSampleBankNames().then(function() {
      return S.queuedFetch('/macros?action=get_trackdefaults');
    }).then(function(data) {
        trackDefaults = data && data.tracks ? data : { tracks: [] };
        return trackDefaults;
      })
      .catch(function(err) {
        console.warn('[TrackDefaults] Load failed, using empty defaults:', err);
        trackDefaults = { tracks: [] };
        return trackDefaults;
      });
  }

  function saveTrackDefaults(data) {
    return S.queuedPost('/macros?action=save_trackdefaults', data, S.API_MUTATION_TIMEOUT_MS)
      .then(function(resp) {
        if (resp && resp.ok) {
          S.toast('Boot defaults saved', 'success', 3000);
          dirty = false;
        } else {
          S.toast('Save failed', 'danger', 4000);
        }
        return resp;
      })
      .catch(function(err) {
        S.toast('Save failed: ' + err.message, 'danger', 4000);
        throw err;
      });
  }

  // ─── Preset dropdown builder ───────────────────────────────

  /**
   * Rebuild the preset <select> options for a given track row
   * when the machine dropdown changes.
   * Presets are grouped by their macro definition name.
   */
  function rebuildPresetDropdown(trackIdx, machineId, currentPresetId) {
    var presetSel = document.querySelector('.td-preset-select[data-track="' + trackIdx + '"]');
    if (!presetSel) return;

    var facets = facetedData[trackIdx] || [];
    var matchingFacet = null;
    for (var i = 0; i < facets.length; i++) {
      if (facets[i].machine === machineId) { matchingFacet = facets[i]; break; }
    }

    var html = '<option value="">(auto — first available)</option>';

    if (matchingFacet) {
      var macros = matchingFacet.macros;
      var useSections = macros.length > 1;

      macros.forEach(function(macro) {
        // Clean up macro definition name for the optgroup label
        var label = (macro.name || macro.id);
        // If it ends with "All param(s)", make it clearer
        if (/All\s*param/i.test(label)) {
          label = label.replace(/\s*All\s*param(s)?\s*$/i, '') + ' — All knobs';
        }

        if (useSections) {
          html += '<optgroup label="' + S.esc(label) + '">';
        }
        macro.presets.forEach(function(p) {
          var sel = p.id === currentPresetId ? ' selected' : '';
          var pName = p.name || p.id;
          html += '<option value="' + S.esc(p.id) + '"' + sel + '>';
          html += S.esc(pName) + ' (' + S.esc(p.id) + ')';
          html += '</option>';
        });
        if (useSections) {
          html += '</optgroup>';
        }
      });
    }

    presetSel.innerHTML = html;
    presetSel.disabled = !machineId;

    // If the current saved preset wasn't found in the new machine, reset
    if (currentPresetId && presetSel.value !== currentPresetId) {
      presetSel.value = '';
    }
  }

  // ─── Rendering ─────────────────────────────────────────────

  function renderOverlayContent() {
    var body = document.getElementById('trackdefaults-body');
    if (!body) return;

    facetedData = buildFacetedData();
    var tracks = S.data.tracks || [];

    var html = '';
    html += '<p class="td-intro">Configure which preset each track loads on boot. ';
    html += 'Pick a <strong>machine</strong> first, then choose a <strong>preset</strong>. ';
    html += 'Changes take effect on next power-up.</p>';

    // ─── Global sample bank selector ─────────────────────────
    if (sampleBankNames.length > 0) {
      var savedBank = (trackDefaults && typeof trackDefaults.sampleBank === 'number')
        ? trackDefaults.sampleBank : 0;
      html += '<div class="td-sample-bank-section">';
      html += '<label><strong>Sample Bank (PSRAM)</strong> ';
      html += '<select class="td-select" id="td-global-sample-bank">';
      sampleBankNames.forEach(function(name, i) {
        var sel = (i === savedBank) ? ' selected' : '';
        html += '<option value="' + i + '"' + sel + '>' + S.esc(name) + '</option>';
      });
      html += '</select></label>';
      html += '<span class="td-hint"> All rompler tracks share this bank. ';
      html += 'Switching reloads PSRAM from SD card at boot.</span>';
      html += '</div>';
    }

    html += '<div class="td-table">';
    html += '<div class="td-row td-header">';
    html += '<span class="td-col-idx">#</span>';
    html += '<span class="td-col-name">Track</span>';
    html += '<span class="td-col-type">Type</span>';
    html += '<span class="td-col-engine">Machine</span>';
    html += '<span class="td-col-preset">Preset</span>';
    html += '<span class="td-col-slice">Slice</span>';
    html += '</div>';

    tracks.forEach(function(track) {
      var idx = track.index;
      var currentPreset = getDefaultPreset(idx);
      var facets = facetedData[idx] || [];

      // Determine current machine from the saved preset
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';

      // Track type badge
      var typeClass = 'td-type-badge';
      if (track.type === 'drum')  typeClass += ' td-type-drum';
      else if (track.type === 'synth') typeClass += ' td-type-synth';
      else if (track.type === 'fx')    typeClass += ' td-type-fx';

      html += '<div class="td-row" data-track="' + idx + '">';
      html += '<span class="td-col-idx">' + String(idx + 1).padStart(2, '0') + '</span>';
      html += '<span class="td-col-name">' + S.esc(track.name) + '</span>';
      html += '<span class="td-col-type"><span class="' + typeClass + '">' + S.esc(track.type) + '</span></span>';

      // Machine dropdown — same options as the main UI MACHINE: dropdown
      html += '<span class="td-col-engine">';
      html += '<select class="td-select td-machine-select" data-track="' + idx + '">';
      html += '<option value="">(auto)</option>';
      facets.forEach(function(f) {
        var sel = f.machine === currentMachine ? ' selected' : '';
        html += '<option value="' + S.esc(f.machine) + '"' + sel + '>' + S.esc(f.name) + '</option>';
      });
      html += '</select>';
      html += '</span>';

      // Preset dropdown (populated dynamically based on machine selection)
      html += '<span class="td-col-preset">';
      html += '<select class="td-select td-preset-select" data-track="' + idx + '"';
      if (!currentMachine) html += ' disabled';
      html += '>';
      html += '<option value="">(auto — first available)</option>';
      html += '</select>';
      html += '</span>';

      // Slice selector (rompler tracks only)
      var isRompler = trackHasRompler(track);
      var savedSlice = getDefaultSlice(idx);
      html += '<span class="td-col-slice">';
      if (isRompler) {
        html += '<select class="td-select td-slice-select" data-track="' + idx + '">';
        for (var sl = 0; sl < 32; sl++) {
          var slSel = (sl === savedSlice) ? ' selected' : '';
          html += '<option value="' + sl + '"' + slSel + '>' + sl + '</option>';
        }
        html += '</select>';
      } else {
        html += '<span class="td-na">—</span>';
      }
      html += '</span>';

      html += '</div>';
    });

    html += '</div>'; // .td-table

    body.innerHTML = html;

    // Populate preset dropdowns for tracks that have a saved machine
    tracks.forEach(function(track) {
      var currentPreset = getDefaultPreset(track.index);
      var facets = facetedData[track.index] || [];
      var currentMachine = currentPreset ? findMachineForPreset(currentPreset, facets) : '';
      if (currentMachine) {
        rebuildPresetDropdown(track.index, currentMachine, currentPreset);
      }
    });

    // Attach machine change listeners
    body.querySelectorAll('.td-machine-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        var idx = parseInt(sel.getAttribute('data-track'), 10);
        var machineId = sel.value;
        rebuildPresetDropdown(idx, machineId, '');
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach preset change listeners
    body.querySelectorAll('.td-preset-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach slice change listeners
    body.querySelectorAll('.td-slice-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    });

    // Attach global sample bank change listener
    var bankSel = document.getElementById('td-global-sample-bank');
    if (bankSel) {
      bankSel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    }

    dirty = false;
    updateSaveButton();
  }

  function updateSaveButton() {
    var btn = document.getElementById('td-save-btn');
    if (btn) {
      btn.disabled = !dirty;
    }
  }

  // ─── Collect & Save ────────────────────────────────────────

  function collectFromUI() {
    // Read global sample bank
    var bankSel = document.getElementById('td-global-sample-bank');
    var sampleBank = bankSel ? parseInt(bankSel.value, 10) : 0;

    var result = {
      _comment: 'Default preset per track, loaded by the Pico via SPI command 0xA5.',
      _comment2: 'Preset IDs = filenames (without .json) from data/macrosoundpresets/.',
      _comment3: 'Omit a track entry to let the Pico use the first available preset.',
      sampleBank: sampleBank,
      tracks: []
    };

    var tracks = S.data.tracks || [];
    var presetSelects = document.querySelectorAll('#trackdefaults-body .td-preset-select');

    presetSelects.forEach(function(sel) {
      var idx = parseInt(sel.getAttribute('data-track'), 10);
      var presetId = sel.value;
      if (presetId) {
        var track = tracks.find(function(t) { return t.index === idx; });
        var trackName = track ? track.name : ('Track ' + idx);

        // Include machine name in the comment for readability
        var machineSel = document.querySelector('.td-machine-select[data-track="' + idx + '"]');
        var machineName = machineSel ? machineSel.options[machineSel.selectedIndex].text : '';

        var entry = {
          index: idx,
          preset: presetId,
          _name: trackName + ' — ' + machineName + ' — ' + presetId
        };

        // Add rompler-specific fields if this track supports rompler
        if (track && trackHasRompler(track)) {
          entry.sampleBank = sampleBank;
          var sliceSel = document.querySelector('.td-slice-select[data-track="' + idx + '"]');
          entry.sampleSlice = sliceSel ? parseInt(sliceSel.value, 10) : 0;
        }

        result.tracks.push(entry);
      }
    });

    return result;
  }

  // ─── Init ──────────────────────────────────────────────────

  function init() {
    var dialog = document.getElementById('trackdefaults-dialog');
    var openBtn = document.getElementById('trackdefaults-btn');
    var saveBtn = document.getElementById('td-save-btn');
    var closeBtn = document.getElementById('td-close-btn');

    if (!dialog || !openBtn) {
      console.warn('[TrackDefaults] Dialog or trigger button not found');
      return;
    }

    openBtn.addEventListener('click', function() {
      S.showLoading('Loading boot defaults…');
      loadTrackDefaults().then(function() {
        renderOverlayContent();
        S.hideLoading();
        dialog.show();
      }).catch(function() {
        S.hideLoading();
        S.toast('Could not load boot defaults', 'danger', 4000);
      });
    });

    if (saveBtn) {
      saveBtn.addEventListener('click', function() {
        var data = collectFromUI();
        S.showLoading('Saving boot defaults…');
        saveTrackDefaults(data).then(function() {
          trackDefaults = data;
          S.hideLoading();
        }).catch(function() {
          S.hideLoading();
        });
      });
    }

    if (closeBtn) {
      closeBtn.addEventListener('click', function() {
        dialog.hide();
      });
    }

    // Confirm unsaved changes on close
    dialog.addEventListener('sl-request-close', function(e) {
      if (dirty) {
        if (!confirm('You have unsaved changes. Discard them?')) {
          e.preventDefault();
        }
      }
    });
  }

  // ─── Export ────────────────────────────────────────────────

  window.TBD = window.TBD || {};
  window.TBD.trackDefaults = {
    init: init,
  };

})();
