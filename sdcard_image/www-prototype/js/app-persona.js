// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Shell & Persona Switcher
//
// Orchestrates boot, persona switching, theme, and connection
// management. This is the entry point that coordinates the
// Performer and Sound Designer views.
//
// DOM IDs (matching index.html):
//   Views:   #view-performer, #view-designer
//   Buttons: #btn-performer (.persona-btn), #btn-designer (.persona-btn)
//   Other:   #conn-pill, #conn-pill-text, #config-btn, #theme-toggle
//            #loading-overlay, #loading-text
//
// (c) 2014-2026 Johannes Elias Lohbihler for dadamachines.
// Licensed under LGPL 3.0.
// ═══════════════════════════════════════════════════════════════
'use strict';

(function() {
  var S = window.TBD.shared;

  // ─── State ───────────────────────────────────────────────
  var activePersona = 'performer'; // 'performer' | 'designer'

  // ─── Persona Switching ───────────────────────────────────

  function switchPersona(persona) {
    if (persona === activePersona) return;
    activePersona = persona;

    var performerView = document.getElementById('view-performer');
    var designerView = document.getElementById('view-designer');
    var performerBtn = document.getElementById('btn-performer');
    var designerBtn = document.getElementById('btn-designer');

    if (persona === 'performer') {
      if (performerView) performerView.classList.add('active');
      if (designerView) designerView.classList.remove('active');
      if (performerBtn) performerBtn.classList.add('active');
      if (designerBtn) designerBtn.classList.remove('active');

      // Lazy-init performer if needed
      if (window.TBD.performer && !window.TBD.performer.state.initialized) {
        window.TBD.performer.init();
      }
    } else {
      if (designerView) designerView.classList.add('active');
      if (performerView) performerView.classList.remove('active');
      if (designerBtn) designerBtn.classList.add('active');
      if (performerBtn) performerBtn.classList.remove('active');

      // Lazy-init designer if needed
      if (window.TBD.designer && !window.TBD.designer.state.initialized) {
        window.TBD.designer.init();
      }
    }

    // Persist choice
    try {
      localStorage.setItem('tbd-persona', persona);
    } catch (e) { /* ignore */ }
  }

  function setupPersonaSwitcher() {
    document.querySelectorAll('.persona-btn').forEach(function(btn) {
      btn.addEventListener('click', function() {
        var persona = btn.getAttribute('data-persona');
        switchPersona(persona);
      });
    });

    // Keyboard shortcut: Ctrl/Cmd + 1 = Performer, Ctrl/Cmd + 2 = Designer
    document.addEventListener('keydown', function(e) {
      if (e.metaKey || e.ctrlKey) {
        if (e.key === '1') {
          e.preventDefault();
          switchPersona('performer');
        } else if (e.key === '2') {
          e.preventDefault();
          switchPersona('designer');
        }
      }
    });
  }

  // ─── Theme ───────────────────────────────────────────────

  function setupTheme() {
    var themeBtn = document.getElementById('theme-toggle');
    if (!themeBtn) return;

    // Restore theme from localStorage
    var savedTheme = null;
    try {
      savedTheme = localStorage.getItem('tbd-theme');
    } catch (e) { /* ignore */ }

    if (savedTheme === 'light') {
      document.documentElement.classList.add('sl-theme-light');
      document.documentElement.classList.remove('sl-theme-dark');
      themeBtn.name = 'sun-fill';
    }

    themeBtn.addEventListener('click', function() {
      var isLight = document.documentElement.classList.contains('sl-theme-light');
      if (isLight) {
        document.documentElement.classList.remove('sl-theme-light');
        document.documentElement.classList.add('sl-theme-dark');
        try { localStorage.setItem('tbd-theme', 'dark'); } catch (e) { /* ignore */ }
        themeBtn.name = 'moon-fill';
      } else {
        document.documentElement.classList.add('sl-theme-light');
        document.documentElement.classList.remove('sl-theme-dark');
        try { localStorage.setItem('tbd-theme', 'light'); } catch (e) { /* ignore */ }
        themeBtn.name = 'sun-fill';
      }
    });
  }

  // ─── Settings ────────────────────────────────────────────

  function setupSettings() {
    var settingsBtn = document.getElementById('config-btn');
    if (!settingsBtn) return;

    settingsBtn.addEventListener('click', function() {
      S.toast('Settings panel — coming in next iteration', 'primary', 2000);
    });
  }

  // ─── Connection Status ───────────────────────────────────

  function updateConnectionStatus(connected) {
    var pill = document.getElementById('conn-pill');
    var pillText = document.getElementById('conn-pill-text');
    if (!pill) return;

    if (connected) {
      pill.classList.add('connected');
      pill.classList.remove('disconnected');
      if (pillText) pillText.textContent = 'Connected';
    } else {
      pill.classList.remove('connected');
      pill.classList.add('disconnected');
      if (pillText) pillText.textContent = 'Offline';
    }
  }

  function setupConnectionMonitor() {
    // Quick connection check by fetching the API
    fetch('/api/v1/samples')
      .then(function() { updateConnectionStatus(true); })
      .catch(function() { updateConnectionStatus(false); });
  }

  // ─── Boot Sequence ───────────────────────────────────────

  function boot() {
    console.log('[TBD-16] Booting persona-based WebUI...');

    // 1. Set up infrastructure
    setupTheme();
    setupSettings();
    setupPersonaSwitcher();
    setupConnectionMonitor();

    // 2. Restore last persona or default to performer
    var savedPersona = null;
    try {
      savedPersona = localStorage.getItem('tbd-persona');
    } catch (e) { /* ignore */ }

    if (savedPersona === 'designer') {
      activePersona = 'designer';
      var designerBtn = document.getElementById('btn-designer');
      var performerBtn = document.getElementById('btn-performer');
      var designerView = document.getElementById('view-designer');
      var performerView = document.getElementById('view-performer');
      if (designerBtn) designerBtn.classList.add('active');
      if (performerBtn) performerBtn.classList.remove('active');
      if (designerView) designerView.classList.add('active');
      if (performerView) performerView.classList.remove('active');
    }

    // 3. Init the active persona view
    if (activePersona === 'performer') {
      if (window.TBD.performer) window.TBD.performer.init();
    } else {
      if (window.TBD.designer) window.TBD.designer.init();
    }

    // 4. Hide loading overlay
    var overlay = document.getElementById('loading-overlay');
    if (overlay) {
      overlay.classList.add('hidden');
      setTimeout(function() { overlay.remove(); }, 500);
    }

    console.log('[TBD-16] Boot complete. Active persona:', activePersona);
  }

  // ─── Start ───────────────────────────────────────────────

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', boot);
  } else {
    boot();
  }

})();
