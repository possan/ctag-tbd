// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — Track Default Presets Editor (Overlay)
//
// Opens as a Shoelace dialog from the header nav.
// Shows all 19 tracks with dropdowns of valid presets,
// allowing sound designers to define the boot-up state.
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

  // ─── Helpers ───────────────────────────────────────────────

  /**
   * Build a map of valid presets per track index.
   * For each track, walks: track.machines → matching macroDefs → matching soundPresets.
   * Returns { 0: [{id, name, group, macro}], 1: [...], ... }
   */
  function buildPresetsPerTrack() {
    var result = {};
    var tracks = S.data.tracks || [];
    var allDefs = S.data.macroDefs || [];
    var allPresets = S.data.soundPresets || [];

    tracks.forEach(function(track) {
      var machines = track.machines || [];

      // Collect all macrodefinition IDs whose machine is in this track's machines list
      var validDefIds = {};
      allDefs.forEach(function(d) {
        if (machines.indexOf(d.machine) !== -1) {
          validDefIds[d.id] = d;
        }
      });

      // Filter soundPresets whose macro references a valid definition
      var validPresets = allPresets.filter(function(p) {
        return !!validDefIds[p.macro];
      });

      // Sort: group, then name
      validPresets.sort(function(a, b) {
        var ga = (a.group || '').toLowerCase();
        var gb = (b.group || '').toLowerCase();
        if (ga !== gb) return ga < gb ? -1 : 1;
        var na = (a.name || a.id).toLowerCase();
        var nb = (b.name || b.id).toLowerCase();
        return na < nb ? -1 : na > nb ? 1 : 0;
      });

      result[track.index] = validPresets;
    });

    return result;
  }

  /**
   * Get the current default preset ID for a track from the loaded defaults.
   */
  function getDefaultPreset(trackIndex) {
    if (!trackDefaults || !trackDefaults.tracks) return '';
    var entry = trackDefaults.tracks.find(function(t) { return t.index === trackIndex; });
    return entry ? (entry.preset || '') : '';
  }

  // ─── API ───────────────────────────────────────────────────

  function loadTrackDefaults() {
    return S.queuedFetch('/macros?action=get_trackdefaults')
      .then(function(data) {
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

  // ─── Rendering ─────────────────────────────────────────────

  function renderOverlayContent() {
    var dialog = document.getElementById('trackdefaults-dialog');
    var body = document.getElementById('trackdefaults-body');
    if (!dialog || !body) return;

    var presetsPerTrack = buildPresetsPerTrack();
    var tracks = S.data.tracks || [];

    var html = '';
    html += '<p class="td-intro">Configure which preset each track loads on boot. ';
    html += 'Changes are saved to the SD card and take effect on next power-up.</p>';

    html += '<div class="td-table">';
    html += '<div class="td-row td-header">';
    html += '<span class="td-col-idx">#</span>';
    html += '<span class="td-col-name">Track</span>';
    html += '<span class="td-col-type">Type</span>';
    html += '<span class="td-col-preset">Boot Preset</span>';
    html += '</div>';

    tracks.forEach(function(track) {
      var idx = track.index;
      var currentPreset = getDefaultPreset(idx);
      var validPresets = presetsPerTrack[idx] || [];

      // Track type badge styling
      var typeClass = 'td-type-badge';
      if (track.type === 'drum') typeClass += ' td-type-drum';
      else if (track.type === 'synth') typeClass += ' td-type-synth';
      else if (track.type === 'fx') typeClass += ' td-type-fx';

      html += '<div class="td-row" data-track="' + idx + '">';
      html += '<span class="td-col-idx">' + String(idx + 1).padStart(2, '0') + '</span>';
      html += '<span class="td-col-name">' + S.esc(track.name) + '</span>';
      html += '<span class="td-col-type"><span class="' + typeClass + '">' + S.esc(track.type) + '</span></span>';

      // Dropdown
      html += '<span class="td-col-preset">';
      html += '<select class="td-select" data-track="' + idx + '">';
      html += '<option value="">(auto — first available)</option>';

      // Group presets by group name for optgroups
      var groups = {};
      var ungrouped = [];
      validPresets.forEach(function(p) {
        if (p.group) {
          if (!groups[p.group]) groups[p.group] = [];
          groups[p.group].push(p);
        } else {
          ungrouped.push(p);
        }
      });

      // Render ungrouped first
      ungrouped.forEach(function(p) {
        var sel = p.id === currentPreset ? ' selected' : '';
        var label = p.name || p.id;
        html += '<option value="' + S.esc(p.id) + '"' + sel + '>' + S.esc(label) + ' (' + S.esc(p.id) + ')</option>';
      });

      // Render grouped
      var groupNames = Object.keys(groups).sort();
      groupNames.forEach(function(g) {
        html += '<optgroup label="' + S.esc(g) + '">';
        groups[g].forEach(function(p) {
          var sel = p.id === currentPreset ? ' selected' : '';
          var label = p.name || p.id;
          html += '<option value="' + S.esc(p.id) + '"' + sel + '>' + S.esc(label) + ' (' + S.esc(p.id) + ')</option>';
        });
        html += '</optgroup>';
      });

      html += '</select>';
      html += '</span>';
      html += '</div>';
    });

    html += '</div>'; // .td-table

    body.innerHTML = html;

    // Attach change listeners
    body.querySelectorAll('.td-select').forEach(function(sel) {
      sel.addEventListener('change', function() {
        dirty = true;
        updateSaveButton();
      });
    });

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
    var result = {
      _comment: 'Default preset per track, loaded by the Pico via SPI command 0xA5.',
      _comment2: 'Preset IDs = filenames (without .json) from data/macrosoundpresets/.',
      tracks: []
    };

    var tracks = S.data.tracks || [];
    var selects = document.querySelectorAll('#trackdefaults-body .td-select');

    selects.forEach(function(sel) {
      var idx = parseInt(sel.getAttribute('data-track'), 10);
      var presetId = sel.value;
      if (presetId) {
        var track = tracks.find(function(t) { return t.index === idx; });
        var trackName = track ? track.name : ('Track ' + idx);
        result.tracks.push({
          index: idx,
          preset: presetId,
          _name: trackName + ' — ' + presetId
        });
      }
      // If empty, omit → Pico auto-selects first available
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
