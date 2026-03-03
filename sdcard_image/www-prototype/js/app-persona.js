// ═══════════════════════════════════════════════════════════════
// TBD-16 WebUI — App Shell & Persona Switcher
//
// Orchestrates boot, persona switching, theme, and connection
// management. This is the entry point that coordinates the
// Performer and Sound Designer views.
//
// Boot Sequence:
//   1. Init shared (connection check, theme)
//   2. Init active persona view
//   3. Show UI
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

    var performerView = document.getElementById('performer-view');
    var designerView = document.getElementById('designer-view');
    var performerTab = document.querySelector('.persona-tab[data-persona="performer"]');
    var designerTab = document.querySelector('.persona-tab[data-persona="designer"]');

    if (persona === 'performer') {
      performerView.classList.add('active');
      designerView.classList.remove('active');
      performerTab.classList.add('active');
      designerTab.classList.remove('active');

      // Lazy-init performer if needed
      if (!window.TBD.performer.state.initialized) {
        window.TBD.performer.init();
      }
    } else {
      designerView.classList.add('active');
      performerView.classList.remove('active');
      designerTab.classList.add('active');
      performerTab.classList.remove('active');

      // Lazy-init designer if needed
      if (!window.TBD.designer.state.initialized) {
        window.TBD.designer.init();
      }
    }

    // Persist choice
    try {
      localStorage.setItem('tbd-persona', persona);
    } catch (e) { /* ignore */ }
  }

  function setupPersonaSwitcher() {
    document.querySelectorAll('.persona-tab').forEach(function(tab) {
      tab.addEventListener('click', function() {
        var persona = tab.getAttribute('data-persona');
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
    var themeBtn = document.getElementById('theme-btn');
    if (!themeBtn) return;

    // Restore theme from localStorage
    var savedTheme = null;
    try {
      savedTheme = localStorage.getItem('tbd-theme');
    } catch (e) { /* ignore */ }

    if (savedTheme === 'light') {
      document.documentElement.classList.add('sl-theme-light');
      document.documentElement.classList.remove('sl-theme-dark');
    }

    themeBtn.addEventListener('click', function() {
      var isLight = document.documentElement.classList.contains('sl-theme-light');
      if (isLight) {
        document.documentElement.classList.remove('sl-theme-light');
        document.documentElement.classList.add('sl-theme-dark');
        try { localStorage.setItem('tbd-theme', 'dark'); } catch (e) { /* ignore */ }
      } else {
        document.documentElement.classList.add('sl-theme-light');
        document.documentElement.classList.remove('sl-theme-dark');
        try { localStorage.setItem('tbd-theme', 'light'); } catch (e) { /* ignore */ }
      }
    });
  }

  // ─── Settings ────────────────────────────────────────────

  function setupSettings() {
    var settingsBtn = document.getElementById('settings-btn');
    if (!settingsBtn) return;

    settingsBtn.addEventListener('click', function() {
      S.toast('Settings panel — coming in next iteration', 'primary', 2000);
    });
  }

  // ─── Connection Status ───────────────────────────────────

  function updateConnectionStatus(connected) {
    var pill = document.getElementById('connection-pill');
    if (!pill) return;

    if (connected) {
      pill.classList.add('connected');
      pill.classList.remove('disconnected');
      pill.textContent = 'Connected';
    } else {
      pill.classList.remove('connected');
      pill.classList.add('disconnected');
      pill.textContent = 'Offline';
    }
  }

  function setupConnectionMonitor() {
    // For prototype: simulate connected state
    updateConnectionStatus(true);

    // In production, use shared.js connection monitor:
    // window.addEventListener('tbd-connection-change', function(e) {
    //   updateConnectionStatus(e.detail.connected);
    // });
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
      document.querySelector('.persona-tab[data-persona="designer"]').classList.add('active');
      document.querySelector('.persona-tab[data-persona="performer"]').classList.remove('active');
      document.getElementById('designer-view').classList.add('active');
      document.getElementById('performer-view').classList.remove('active');
    }

    // 3. Init the active persona view
    if (activePersona === 'performer') {
      window.TBD.performer.init();
    } else {
      window.TBD.designer.init();
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
