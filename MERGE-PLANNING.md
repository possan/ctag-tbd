# Merge Planning: `feature/webui-persona-prototype` → `feature/webui-general-ui-rework`

**Branch A (source):** `feature/webui-persona-prototype` (current, persona/performer/designer UI + macro system)  
**Branch B (target):** `feature/webui-general-ui-rework` (Shoelace general UI + original plugin manager)

**Summary of scale:**  
520 files changed · +48,060 / −144,934 lines  
3,564 lines of new firmware (macro system)  
8,200 lines of new C++ DSP machines  
33 macro definitions + 58 sound presets (new in A, absent in B)

---

## 1. Documentation (`docs/`)

**Status:** `feature/webui-general-ui-rework` has **159 documentation files** (RST, CSS, HTML, images, firmware binaries) that were **deleted** in our branch.

**Content of B's docs:**
- Full developer and user documentation site (Sphinx/ABlog)
- Getting started, plugin tutorials, MIDI, WiFi, flash guides
- Firmware binaries for p4 and pico targets
- SD-card images and hash files

**Decision needed:**
- [ ] **Restore docs/ from branch B** — we should bring these back as they are maintained/useful  
- [ ] Or: ignore docs/ divergence and handle separately

---

## 2. Unit Tests (`tests/`)

**Branch A adds:**
- `tests/test_dsp_json_alignment.py` — 6-test suite validating C++ ↔ JSON alignment, Mix pages, macro mappings

**Branch B has (deleted in A):**
- `tests/apitest/test-device-api.sh` — shell-based REST API test suite  
- `tests/apitest/reports/` — 2 historical test reports  
- `tests/webui/run-tests.js` — WebUI automation test runner (1,304 lines)

**Decision needed:**
- [ ] **Keep both** — restore B's test files alongside our new test script  
- [ ] Our `test_dsp_json_alignment.py` remains, do not overwrite with B's tests

---

## 3. WebUI — `sdcard_image/www/` ⚠️ MAJOR CONFLICT

This is the most complex area. The two branches have **entirely different implementations** of the `sdcard_image/www/` UI.

### Branch A (our branch) — Old jQuery/OnsenUI multi-page app

- Multi-page HTML structure: `index.html`, `main.html`, `edit.html`, `macros.html`, `drumrack.html`, `fav.html`, `load.html`, `save.html`, `config.html`
- JS: `drumrack.js`, `macros.js`, `jquery`, `jszip`, `ajaxq`, `onsenui`
- CSS: `drumrack.css`, `macros.css`, `onsen-css`
- Built preseteditor: `preseteditor.html`, `preseteditor-*.css`, `preseteditor-*.js`

### Branch B (target) — Modern Shoelace single-page app

- Single `index.html` with all UI inline
- JS: `shoelace-bundle.js`, `app-bundle.js`, `app.js`, `plugin-manager.js`, `sample-manager.js`, `shared.js`, `display-hints.js`
- This is the "proper" modern UI that general-ui-rework is building toward

### Branch A also adds — `sdcard_image/www-prototype/` (NEW, not in B)

- Our newest, best Persona/Performer/Designer WebUI prototype
- Shoelace SPA with performer (knob view + preset browser), designer (macro editor), SPI configurator
- `app.css` v8, `shared.js` v10, `performer.js` v9, `designer.js` v9, `app.js`

**Decision needed:**
- [ ] **Option 1 — Take B's `www/` Shoelace app, keep our `www-prototype/`**  
  Discard the old jQuery multi-page UI from A, restore B's modern `www/` as baseline, keep `www-prototype/` alongside for the new persona prototype work
- [ ] **Option 2 — Keep A's `www/` as-is (old multi-page)**  
  Not recommended — it's older than B's implementation
- [ ] **Regardless:** `sdcard_image/www-prototype/` must be preserved — this is our active development

---

## 4. WebUI Developer Tool — `www/preseteditor/` (TypeScript)

**Status:** Only in our branch A. Not in B.

**Content:**  
TypeScript + Vite preset editor tool with 9 source files: `main.ts`, `macropreset.ts`, `soundpreset.ts`, `outputmappinglist.ts`, `parameterlist.ts`, `preview.ts`, `basicprops.ts`, `device.ts`, `state.ts`

**Decision:**
- [ ] **KEEP** — This is our work, not in target. No conflict.

---

## 5. SD Card Data — `sdcard_image/data/` ⚠️ IMPORTANT

**Status:** All macro/preset/synth data exists **only in branch A**. Branch B has none of this.

**Files unique to A:**
- `sdcard_image/data/synthdefinitions.json` — 23 machine definitions with Mix params
- `sdcard_image/data/macrodefinitions/*.json` — 33 macro definition files (all machines + custom presets)
- `sdcard_image/data/macrosoundpresets/*.json` — 58 sound presets

**Modified in both (`.jsn` plugin parameter files):**
- `sdcard_image/data/sp/mp-PicoSeqRack.jsn` — Large (3,199 lines added from possan merge)
- `sdcard_image/data/sp/mui-PicoSeqRack.jsn` — Large (4,566 lines added)
- All other `mp-*.jsn` files — Minor changes, likely compatible

**Decision:**
- [ ] **KEEP all our data files** — macrodefinitions, macrosoundpresets, synthdefinitions.json — do not lose these
- [ ] Review `.jsn` sp files for conflicts carefully during merge

---

## 6. Firmware — `main/` ⚠️ MAJOR ADDITIONS

### New files (only in A, not in B) — KEEP ALL:

| File | Lines | Purpose |
|------|-------|---------|
| `MacroAPI.cpp/.hpp` | 159 | REST API handler for macro operations |
| `MacroDeviceDefinition.cpp/.hpp` | 470 | Device-level macro definition model |
| `MacroDeviceDefinitionDataModel.cpp/.hpp` | 204 | JSON serialization for macro defs |
| `MacroSoundPreset.cpp/.hpp` | 124 | Sound preset data model |
| `MacroSoundPresetDataModel.cpp/.hpp` | 359 | JSON serialization for sound presets |
| `MacroSoundPresetGroup.cpp/.hpp` | 34 | Preset group structure |
| `MacroTranslator.cpp/.hpp` | 519 | MIDI CC → DSP parameter translation |
| `SynthDefinition.cpp/.hpp` | 155 | Machine parameter definitions |
| `SynthDefinitionDataModel.cpp/.hpp` | 266 | JSON serialization for synth defs |
| `TrackDefinition.cpp/.hpp` | 83 | Track-level machine assignment |
| `SpiProtocol.h` | 111 | SPI communication protocol header |
| `SpiProtocolHelper.cpp/.hpp` | 85 | SPI protocol helpers |

### Modified files (both branches changed) — REVIEW:

| File | A changes | Notes |
|------|-----------|-------|
| `RestServer.cpp` | +118 lines mixed | Added `/api/v1/macroapi` route, structural changes. **Need to merge MacroAPI route into B's version** |
| `RestServer.hpp` | +16 lines | Added MacroAPI include/declaration |
| `SPManager.cpp` | +476 lines | Massive additions: PicoSeqRack integration, macro system hookup. **B has no macro code** |
| `SPManager.hpp` | +34 lines | New macro/track fields |
| `SPManagerDataModel.cpp` | +2 lines | Minor additions |
| `SampleAPI.cpp` | +230 lines | Added `manage` action for file delete/rename |
| `SpiAPI.cpp` | +171 lines | SPI protocol changes |
| `SpiAPI.hpp` | +21 lines | SPI interface additions |
| `Control.cpp/.hpp` | Minor | Likely MIDI/SPI hookup changes |
| `CMakeLists.txt` | Minor | Source file additions |
| `component.mk` | +17 lines | Build system entries for new files |

**Decision:**
- [ ] **All new files** → KEEP, no conflict  
- [ ] `RestServer.cpp` → Manually merge: B's version as base, graft in our MacroAPI route registration  
- [ ] `SPManager.cpp` → Ours is the correct version (B's is the old baseline with no macro system)  
- [ ] `SampleAPI.cpp` → Ours (we added delete/rename, B doesn't have it)  
- [ ] `SpiAPI.cpp/.hpp` → Ours (SPI protocol additions)  
- [ ] `Control.cpp/.hpp` → Careful diff, take union of changes

---

## 7. DSP Components — `components/ctagSoundProcessor/` ⚠️ MAJOR ADDITIONS

### New files (only in A) — KEEP ALL:

The entire `rack/` subdirectory is new in A:  
`RackABD`, `RackASD`, `RackChannelMixer`, `RackClap`, `RackDBD`, `RackDSD`, `RackFMB`, `RackFxDelay`, `RackFxMaster`, `RackFxReverb`, `RackHH1`, `RackHH2`, `RackInput`, `RackMO`, `RackPolyPad`, `RackRimshot`, `RackRompler`, `RackSynth`, `RackTBD03`, `RackWTOsc` (+hpp files)

Also new: `ctagSoundProcessorPicoSeqRack.cpp/.hpp` (1,508 lines — the main PicoSeqRack sound processor)

### Modified files (both branches changed) — REVIEW:

| File | Notes |
|------|-------|
| `CMakeLists.txt` | Added PicoSeqRack and rack/ sources — **take ours** |
| `ctagSPDataModel.cpp` | +16 lines — likely component registration. **Diff carefully** |
| `ctagSoundProcessor.hpp` | +53 lines — base class interface. **Diff carefully** |
| `ctagSampleRom.cpp/.hpp` | +50 lines — sample ROM extensions. **Diff carefully, take union** |
| `ctagSampleRomModel.cpp/.hpp` | −21 lines — may have been refactored in B. **Diff carefully** |

**Decision:**
- [ ] All `rack/` files and PicoSeqRack → KEEP (not in B)  
- [ ] Modified files → diff and take the union; prioritize our additions

---

## 8. Simulator — `simulator/`

**Status:** Both branches modified the simulator. Our changes are **smaller** (we stripped it down: `www/ui.html` went from 568 lines in B to 287+287 in ours — net ~280 line reduction).

**Modified files:**

| File | Notes |
|------|-------|
| `WebServer.cpp` | We removed ~268 lines. B has richer simulator server. |
| `www/ui.html` | Heavily diverged — B has fuller WebUI, ours is stripped |
| `CMakeLists.txt` | Minor |
| `SimSPManager.cpp/.hpp` | Minor removals |
| `fake-idf/malloc.c/.h` | B added malloc.c (24 lines), we modified malloc.h |
| `data/cfg_tbd_sim.jsn` | Minor |

**Decision:**
- [ ] **Take B's simulator as base** — it has the more complete WebServer/ui.html  
- [ ] Ensure our `fake-idf/malloc.c` addition from B is included  
- [ ] Review if any of our simulator removals were intentional  

---

## 9. Build System & Root Files

| File | Notes | Decision |
|------|-------|----------|
| `CMakeLists.txt` | Both modified — our version adds PicoSeqRack. **Take ours.** | KEEP OURS |
| `.gitignore` | Both modified — minor differences | **Merge** (take union of entries) |
| `partitions_example.csv` | Both modified — partition sizes | **Diff and decide** |
| `sdkconfig.defaults` | Both modified | **Diff carefully** |
| `create_sd_archive.sh` | Both modified | **Diff and merge** |
| `LICENSE` | Both modified | **Keep B's** (should be authoritative) |
| `README.md` vs `readme.md` | B has `README.md` (deleted in A), A has lowercase `readme.md` | **Restore README.md from B, keep our readme.md** |

---

## 10. Prototyping Notes (`prototyping/`)

**Branch B has:** 12 archived markdown files (planning docs, specs, implementation notes)  
**Our branch:** Deleted all of them, added `MACRO-AND-PRESET-CONCEPT.md`, `MACRO-PRESET-ALIGNMENT.md`, `MEMORY-ANALYSIS.md`, `SMART-HELPER-PROPOSAL.md`, `TODO.txt` at root level

**Decision:**
- [ ] **Restore B's `prototyping/archived/` folder** — historical planning docs are useful  
- [ ] Keep our root-level planning docs as-is  
- [ ] Restore B's `prototyping/DEPLOYMENT-RULES.md` and `WEBUI-STATUS-AND-ROADMAP.md`

---

## 11. GitHub Workflows (`.github/workflows/`)

**Branch B has (deleted in A):**
- `build-docs.yml` — Automated documentation build  
- `deploy-docs.yml` — Automated documentation deployment

**Decision:**
- [ ] **Restore from B** — useful CI automation

---

## Summary Table

| Area | Action | Priority |
|------|--------|----------|
| `docs/` | **Restore from B** | Low |
| `tests/` | **Keep both** | Medium |
| `sdcard_image/www/` | **Take B's Shoelace SPA, discard A's jQuery app** | High |
| `sdcard_image/www-prototype/` | **KEEP OURS** (do not overwrite) | Critical |
| `sdcard_image/data/macrodefinitions/` | **KEEP OURS** | Critical |
| `sdcard_image/data/macrosoundpresets/` | **KEEP OURS** | Critical |
| `sdcard_image/data/synthdefinitions.json` | **KEEP OURS** | Critical |
| `sdcard_image/data/sp/*.jsn` | Diff and merge | High |
| `main/Macro*.cpp/.hpp` | **KEEP OURS** (not in B) | Critical |
| `main/Synth*.cpp/.hpp` | **KEEP OURS** (not in B) | Critical |
| `main/Track*.cpp/.hpp` | **KEEP OURS** (not in B) | Critical |
| `main/RestServer.cpp` | **Merge** (add MacroAPI route to B base) | High |
| `main/SPManager.cpp` | **KEEP OURS** | Critical |
| `main/SampleAPI.cpp` | **KEEP OURS** | Critical |
| `main/SpiAPI.cpp/.hpp` | **KEEP OURS** | High |
| `components/.../rack/*.cpp` | **KEEP OURS** (not in B) | Critical |
| `components/ctagSoundProcessorPicoSeqRack.*` | **KEEP OURS** (not in B) | Critical |
| `components/*` modified | Diff, take union | High |
| `simulator/` | **Take B as base**, review our removals | Medium |
| `prototyping/` | **Restore B's archived docs** | Low |
| `www/preseteditor/` | **KEEP OURS** | High |
| `.github/workflows/` | **Restore from B** | Low |
| Build files (CMakeLists, partitions, sdkconfig) | **Diff individually** | High |

---

## Suggested Merge Approach

1. **Start from B** (`feature/webui-general-ui-rework`) as the base
2. **Cherry-pick or patch in our critical additions:**
   - All new `main/Macro*`, `main/Synth*`, `main/Track*`, `main/Spi*` files  
   - All `components/ctagSoundProcessor/rack/*` files  
   - `ctagSoundProcessorPicoSeqRack.cpp/.hpp`
   - `sdcard_image/www-prototype/` (entire folder)
   - `sdcard_image/data/macrodefinitions/`, `macrosoundpresets/`, `synthdefinitions.json`
   - `www/preseteditor/` (TypeScript tool)
   - Root docs: `MACRO-AND-PRESET-CONCEPT.md`, `SMART-HELPER-PROPOSAL.md`, etc.
   - `tests/test_dsp_json_alignment.py`
3. **Manually reconcile** modified firmware files: RestServer.cpp, SPManager.cpp, components/ headers
4. **Restore from B:** `docs/`, `prototyping/`, `.github/workflows/`, `README.md`, simulator
5. **Build system:** Merge CMakeLists files, partitions_example.csv, sdkconfig.defaults

---

*Generated: 2026-03-04 | Branch comparison: `feature/webui-persona-prototype` → `feature/webui-general-ui-rework`*
