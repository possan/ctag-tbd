# Proposal: Branching & Collaboration Strategy — ctag-tbd × dadamachines

## Context

The ctag-tbd project exists across multiple repositories and branches with different focus areas, licenses, and stakeholders. This document proposes a sustainable branching strategy that keeps collaboration smooth while respecting the distinct goals of each project.

### The Two Projects

| | **ctag-tbd** (upstream) | **dadamachines TBD-16** |
|---|---|---|
| Repository | [ctag-fh-kiel/ctag-tbd](https://github.com/ctag-fh-kiel/ctag-tbd) | [dadamachines/ctag-tbd](https://github.com/dadamachines/ctag-tbd) |
| Primary branch | `p4_main` | `dada-tbd-master` |
| License | GPLv3 | Parts LGPL (product-specific) |
| Hardware target | ESP32-P4 only | ESP32-P4 + RP2350 (TBD-16) |
| Focus | Core DSP platform, plugins, research | Commercial product, sequencer, UI, presets |
| Maintainer | ctag-fh-kiel | dadamachines |

### The Relationship

```
ctag-fh-kiel/ctag-tbd (p4_main)       ← core platform, GPLv3
        │
        │  fork + product overlay
        ▼
dadamachines/ctag-tbd (dada-tbd-master) ← TBD-16 product, adds RP2350/sequencer/macros
        │
        │  experimental feature branches
        ▼
nevvkid/ctag-tbd_hacking (feature/*)   ← active development, to be stabilized
                                           and merged back into dada-tbd-master
```

`dada-tbd-master` was originally based on `p4_main`. All core DSP engine code, audio codec drivers, plugin architecture, and ESP-IDF platform setup came from upstream. dadamachines added the RP2350 SPI bridge, sequencer integration, macro/preset system, PicoSeqRack DSP machine, WebUI, sample management, and product-specific configuration.

---

## Current State

### Branch Topology (as of 2026-03-07)

```
upstream/p4_main ─────────────────────────────────────── (stopped, 0 commits ahead of merge base)
                  \
                   └──► upstream/dada-tbd-master ─────── (180 commits ahead of p4_main)
                         \
                          └──► feature/webui-merge-planning  (190 commits ahead, 248 behind dada-tbd-master)
                                     ▲
                                     │  merged in
                         possan/macropresets ─── (macro system, PicoSeqRack, rack DSP machines)
```

### Where Things Stand

- **`upstream/p4_main`** — Last commit: `f7dcbcde` ("better sd performance"). This is the merge base for `dada-tbd-master`. No new commits since the fork. This branch represents the core platform before any TBD-16 additions.

- **`upstream/dada-tbd-master`** — 180 commits ahead of `p4_main`. Contains: Shoelace WebUI, documentation site, beta firmware pages, artist interviews, deployment tooling, SD card improvements. This is the current stable production branch for TBD-16.

- **`feature/webui-merge-planning`** (our current branch) — 190 commits ahead of its merge base with `dada-tbd-master`, 248 behind. This is the leading edge: contains the merged macro/preset system from possan, rack DSP machines, Persona/Performer/Designer WebUI prototype, Rompler fixes, sample manager improvements, and the configurable build proposal. **This needs to be stabilized and merged into `dada-tbd-master`.**

### The Merge That Already Happened

The `feature/webui-merge-planning` branch was created by merging two feature branches (documented in [MERGE-PLANNING.md](MERGE-PLANNING.md)):

1. **`feature/webui-persona-prototype`** — Our WebUI and firmware work (macro system scaffold, persona prototype, sample manager)
2. **`feature/webui-general-ui-rework`** — The base Shoelace SPA, simulator, documentation, test suites

And then incorporating **`possan/macropresets`** — possan's sequencer developer work: the full macro/preset system, PicoSeqRack sound processor, 20 rack DSP machines, 33 macro definitions, 58 sound presets.

The merge is functional — firmware builds, boots, audio plays. What remains is stabilization, testing, and cleanup before this flows back into `dada-tbd-master`.

---

## The Problem

Right now the relationship between upstream `ctag-fh-kiel/ctag-tbd` and `dadamachines/ctag-tbd` is informal. There's no clear mechanism for:

1. **Upstream improvements flowing downstream** — If ctag-tbd improves the DSP engine, codec driver, or adds new plugins on `p4_main`, dadamachines has to manually cherry-pick or merge.

2. **Product additions flowing upstream** — Bug fixes, DSP improvements, and new plugins developed for TBD-16 that would benefit the core platform require manual extraction and PR.

3. **Avoiding divergence** — The longer both projects evolve independently, the harder merges become. The 180+ commit divergence today is already significant.

4. **License separation** — GPLv3 core code must stay clean in `ctag-fh-kiel`. LGPL product code (sequencer, macro system, TBD-16-specific features) must stay in `dadamachines`.

---

## Proposed Strategy

### Principle: Shared Core, Product Overlay

The codebase naturally divides into two layers:

```
┌─────────────────────────────────────────────────────┐
│  PRODUCT LAYER (dadamachines, LGPL)                 │
│  RP2350 SPI bridge, sequencer, macros, presets,     │
│  PicoSeqRack, rack/* DSP machines, macro WebUI,     │
│  product config, deployment tooling                  │
├─────────────────────────────────────────────────────┤
│  SHARED LAYER (dadamachines → upstream, GPLv3)      │
│  WebUI (Shoelace SPA, themeable),                    │
│  core plugin manager UI, device config UI            │
├─────────────────────────────────────────────────────┤
│  CORE PLATFORM (ctag-fh-kiel, GPLv3)               │
│  ESP-IDF setup, audio codec, PSRAM management,      │
│  plugin architecture, DSP plugin base class,         │
│  sample ROM, core plugins, REST server skeleton,     │
│  WiFi/network, filesystem abstraction                │
└─────────────────────────────────────────────────────┘
```

### What Goes Upstream vs. What Stays in dadamachines

Not everything in the product layer is product-specific. The layering above has three tiers because some dadamachines work is designed to benefit the entire ctag-tbd ecosystem.

#### Stays in dadamachines only (LGPL, not upstream)

| Component | Why |
|-----------|-----|
| Macro/preset system (`main/Macro*.cpp`, `main/Synth*.cpp`, `main/Track*.cpp`) | LGPL, TBD-16 product concept — maps CC knobs to DSP params via JSON definitions |
| PicoSeqRack sound processor (`ctagSoundProcessorPicoSeqRack.*`) | LGPL, requires RP2350 sequencer |
| Rack DSP machines (`components/ctagSoundProcessor/rack/*`) | LGPL, designed for PicoSeqRack's multi-machine architecture |
| Macro definitions + sound presets (`sdcard_image/data/macrodefinitions/`, `macrosoundpresets/`) | Tied to macro system |
| Synth definitions (`sdcard_image/data/synthdefinitions.json`) | Tied to macro system |
| RP2350 SPI bridge (`rp2350_spi_stream.*`, `SpiAPI.*`, `SpiProtocol.*`) | Hardware-specific to TBD-16 |
| WebUI macro/preset pages (macro editor, preset browser, performer knob view) | UI for the macro system |

#### Ships upstream to ctag-tbd (GPLv3)

| Component | How it adapts for upstream |
|-----------|--------------------------|
| **WebUI (Shoelace SPA)** | Ships as the standard ctag-tbd web interface. Themeable — upstream uses a neutral/ctag theme, dadamachines uses a product theme. The macro/preset sections of the UI are hidden or removed when `CONFIG_TBD_USE_RP2350=n` (no macro system present). The core plugin manager, device config, and audio controls work identically on any TBD. |
| **Sample manager UI** | Adapted for upstream's flash-only setup (no SD card). Upstream version shows the PSRAM buffer contents and the `.tbd` flash sample ROM. No SD card file browser, no kit switching, no WAV upload. The `sample_bank_manager.html` tool (already in `sample_rom/`) handles offline sample preparation. |
| **REST API routes** | Core routes (plugin config, device API, sample ROM info) go upstream. MacroAPI route is excluded. |
| **DSP engine improvements** | Any audio codec, PSRAM, or plugin architecture fixes discovered during TBD-16 work are PRed back to `p4_main`. |
| **New GPLv3 plugins** | Individual sound processors that don't depend on the rack/macro architecture can be contributed upstream. |

#### WebUI: Theming & Feature Gating

The Shoelace-based WebUI is designed as a single-page app that can adapt to the build configuration. The plan:

```
WebUI (shared codebase, ships in all TBD builds)
├── Plugin manager          → always present
├── Device config           → always present
├── Audio controls          → always present
├── Sample manager          → adapted per config:
│   ├── SD card mode        → full file browser, kit switching, WAV upload
│   └── Flash-only mode     → PSRAM buffer view, flash ROM info, no upload
├── Macro editor            → only when CONFIG_TBD_USE_RP2350=y (or macro system present)
├── Preset browser          → only when macro system present
└── Theme                   → CSS variables, logo, color scheme
    ├── ctag-tbd theme      → neutral, research/platform branding
    └── dadamachines theme  → TBD-16 product branding
```

Feature gating can work at two levels:

1. **Build-time**: The REST API simply doesn't register MacroAPI routes when the macro system isn't compiled in. The WebUI's JS detects missing API endpoints and hides the corresponding UI sections.

2. **Runtime**: A `/api/v1/capabilities` endpoint returns which features are available (`{ "macros": false, "sdcard": false, "sequencer": false }`). The WebUI reads this on load and shows/hides sections accordingly. This is more flexible and doesn't require separate WebUI builds.

The runtime approach is preferred — one WebUI binary works everywhere, it just adapts to what the firmware supports.

The Kconfig flags proposed in [proposal-simple-tbd-config.md](proposal-simple-tbd-config.md) (`CONFIG_TBD_USE_SD_CARD`, `CONFIG_TBD_USE_RP2350`, `CONFIG_TBD_USE_P4_SEQUENCER`) are the mechanism that makes this layering work at build time. Code guarded by `#ifdef CONFIG_TBD_USE_RP2350` is naturally product-layer code.

### Option A: Fork with Periodic Rebase (Current Model, Improved)

```
ctag-fh-kiel/ctag-tbd (p4_main)
        │
        │  dadamachines/ctag-tbd is a GitHub fork
        │  dada-tbd-master rebases on or merges from p4_main periodically
        ▼
dadamachines/ctag-tbd (dada-tbd-master)
```

**How it works:**
1. `ctag-fh-kiel/ctag-tbd` `p4_main` is the core platform. All core changes land here first.
2. `dadamachines/ctag-tbd` is a GitHub fork. `dada-tbd-master` tracks `p4_main` via periodic merges.
3. When dadamachines finds bugs or improves core code, they open PRs against `ctag-fh-kiel/ctag-tbd` `p4_main`.
4. Product-only code (RP2350, macros, sequencer) stays exclusively in `dada-tbd-master`.

**Merge cadence:** After each ctag-tbd release or significant batch of commits on `p4_main`:
```bash
# In dadamachines/ctag-tbd repo
git fetch ctag-upstream   # remote pointing to ctag-fh-kiel/ctag-tbd
git checkout dada-tbd-master
git merge ctag-upstream/p4_main
# Resolve conflicts (mostly in shared files like SPManager.cpp, CMakeLists.txt)
# Test build + flash + verify
git push origin dada-tbd-master
```

**Pros:**
- Simple mental model — dadamachines is "just a fork with extras"
- Standard GitHub fork workflow — PRs back upstream are natural
- Each repo has its own releases, CI, issue tracker

**Cons:**
- Merge conflicts at integration points (SPManager.cpp, RestServer.cpp, main.cpp)
- Risk of drift if merges are infrequent
- Shared files need careful maintenance

### Option B: Monorepo with Kconfig Layering (Tighter Integration)

```
ctag-fh-kiel/ctag-tbd (main)
        │
        │  Both projects commit to the SAME repo
        │  Product code is guarded by Kconfig flags
        │  dadamachines-only code is in clearly marked directories
        ▼
Single codebase, build flags select product
```

**How it works:**
1. A single repository (`ctag-fh-kiel/ctag-tbd`) holds everything.
2. The Kconfig flags from the config proposal control what gets compiled.
3. Product-specific directories are clearly marked:
   ```
   components/ctagSoundProcessor/rack/        ← TBD-16 only (LGPL)
   main/Macro*.cpp                             ← TBD-16 only (LGPL)
   main/SpiAPI.cpp                             ← TBD-16 only (LGPL)
   components/drivers/rp2350_spi_stream.*      ← TBD-16 only (LGPL)
   ```
4. A `LICENSE-PRODUCT` file alongside `LICENSE` clarifies which directories are LGPL.
5. `idf.py menuconfig` → disable RP2350+macros = pure ctag-tbd. Enable them = TBD-16.

**Pros:**
- Zero merge overhead — there's only one codebase
- Changes to shared files (SPManager, RestServer) are always compatible with both configs because CI tests both
- Easier for contributors — one repo, one build, config flags
- The Kconfig proposal already provides the mechanism

**Cons:**
- ctag-fh-kiel may not want LGPL product code in their repo
- Repository contains code that only one product uses
- More complex CI (must test multiple configurations)
- License mixing in one repo needs careful directory-level attribution

### Option C: Git Submodule for Product Layer (Recommended)

```
ctag-fh-kiel/ctag-tbd (p4_main)        ← core platform, GPLv3
        │
        │  submodule reference
        ▼
dadamachines/tbd16-product              ← product overlay, LGPL
  ├── rack/                             ← rack DSP machines
  ├── macro/                            ← macro/preset system
  ├── spi/                              ← RP2350 SPI bridge
  ├── sequencer/                        ← sequencer integration
  ├── webui/                            ← TBD-16 WebUI
  └── Kconfig.product                   ← product-specific Kconfig
```

**How it works:**
1. `ctag-fh-kiel/ctag-tbd` stays clean — GPLv3 core platform only.
2. `dadamachines/tbd16-product` is a separate repo containing all TBD-16-specific code.
3. `dadamachines/ctag-tbd` `dada-tbd-master` references both via git submodule:
   ```
   dadamachines/ctag-tbd/
   ├── (core files from ctag-fh-kiel/ctag-tbd)
   └── product/  → submodule: dadamachines/tbd16-product
   ```
4. The ESP-IDF `CMakeLists.txt` conditionally includes the product submodule based on Kconfig.
5. Core improvements flow naturally — update the submodule pin.

**Pros:**
- Clean license separation (GPLv3 repo vs LGPL repo)
- Core platform stays lean — no product code mixed in
- Product code is versioned independently
- Updating the core is just `git submodule update`

**Cons:**
- Submodules add git workflow complexity (developers must `git submodule update --init`)
- ESP-IDF component integration with submodules requires careful CMakeLists.txt setup
- Files that straddle the boundary (SPManager.cpp, RestServer.cpp) are harder to handle — they're in the core but need product hooks
- Refactoring shared files requires coordinated changes across two repos

---

## Recommendation: Option A with Kconfig Discipline

**Option A (improved fork model)** is the most practical path forward given where things stand today. Here's why:

1. **It's the current model** — just needs to be done more deliberately.
2. **The Kconfig flags** from the config proposal provide the technical mechanism for clean separation without restructuring the repo.
3. **The merge boundary is manageable** — only ~6 files need `#ifdef` guards, and these are the same files that would cause conflicts in any model.
4. **License separation is already clear** — dadamachines adds LGPL code in its fork; ctag-fh-kiel's repo stays GPLv3.
5. **No submodule overhead** — submodules are widely disliked and add friction for every contributor.

### Concrete Workflow

#### Repository Setup

```
ctag-fh-kiel/ctag-tbd       ← upstream, GPLv3 core
  branch: p4_main           ← primary development branch
  
dadamachines/ctag-tbd        ← fork, LGPL product additions
  branch: dada-tbd-master   ← stable TBD-16 releases
  branch: dev               ← integration testing before release
  remote: ctag-upstream     ← points to ctag-fh-kiel/ctag-tbd
```

#### Day-to-Day Development

**For TBD-16 features (dadamachines):**
```bash
git checkout -b feature/new-rack-machine   # branch from dada-tbd-master
# develop, test, PR to dada-tbd-master
```

**For core platform improvements (ctag-tbd or dadamachines):**
```bash
# If discovered during TBD-16 work, isolate the fix:
git checkout -b fix/sample-rom-bug ctag-upstream/p4_main
# Fix the bug in core code only (no product-specific changes)
# PR to ctag-fh-kiel/ctag-tbd p4_main
# Then merge p4_main into dada-tbd-master to pick up the fix
```

**New DSP plugins or engines:**
- If GPLv3 compatible → PR to `ctag-fh-kiel/ctag-tbd` — benefits everyone
- If LGPL / product-specific (rack machines, PicoSeqRack) → stays in `dadamachines/ctag-tbd`

#### Periodic Sync (Monthly or After Upstream Releases)

```bash
# In dadamachines/ctag-tbd
git fetch ctag-upstream
git checkout dada-tbd-master
git merge ctag-upstream/p4_main --no-ff -m "Sync core platform from ctag-tbd p4_main"
# Resolve conflicts in the known hotspots:
#   - main/SPManager.cpp (audio task + MIDI routing)
#   - main/RestServer.cpp (API routes)
#   - main/main.cpp (boot sequence)
#   - CMakeLists.txt (source lists)
#   - sdkconfig.defaults (config values)
# Build, test all four configs, push
```

#### Known Merge Hotspots

These files are modified by both core and product code. The Kconfig `#ifdef` guards minimize conflicts:

| File | Core concern | Product concern | Guard |
|------|-------------|-----------------|-------|
| `main/SPManager.cpp` | Audio pipeline, plugin hosting | SPI buffer exchange, macro system | `#ifdef CONFIG_TBD_USE_RP2350` |
| `main/RestServer.cpp` | Base API routes, CORS, server setup | MacroAPI route, product endpoints | `#ifdef CONFIG_TBD_USE_RP2350` |
| `main/main.cpp` | Boot sequence, filesystem init | SpiAPI start, sequencer init | `#ifdef CONFIG_TBD_USE_RP2350` |
| `CMakeLists.txt` | Core source list | rack/ sources, macro files | `if(CONFIG_TBD_USE_RP2350)` |
| `sdkconfig.defaults` | Platform config | SPI pins, USB config | Kconfig defaults |
| `components/.../ctagSoundProcessor.hpp` | Plugin base class | MIDI virtual methods | Always present (harmless) |

With proper `#ifdef` guards, most of these files can accept changes from both sides without conflict — the core code runs unconditionally, product code is guarded.

---

## Stabilization Path: Current Branch → dada-tbd-master

Before any branching strategy matters, the current `feature/webui-merge-planning` branch needs to become the new `dada-tbd-master`. Here's the path:

### Phase 1 — Stabilize (current priority)

1. **Fix remaining issues** on `feature/webui-merge-planning`:
   - All DSP/audio regressions from the possan merge
   - WebUI functional completeness (Shoelace SPA in `www/` + prototype in `www-prototype/`)
   - Sample manager edge cases
   - Boot stability (SD card mount retries, network fallback)

2. **Run full test matrix:**
   - `tests/test_dsp_json_alignment.py` — all 128 tests passing (currently 119/128, 9 pre-existing)
   - `tests/apitest/test-device-api.sh` — REST API regression suite
   - Manual test: boot, WebUI loads, presets work, audio plays, MIDI responds, sequencer functions

3. **Clean up commit history** — squash/fixup the experimental commits into clean logical units. The 190 commits since the merge base include debugging, back-and-forth fixes, and AI-assisted iterations that should be consolidated.

### Phase 2 — Merge into dada-tbd-master

```bash
# Option 1: Fast-forward merge (cleanest, if possible after rebase)
git checkout dada-tbd-master
git merge feature/webui-merge-planning

# Option 2: Squash merge (if history is too messy)
git checkout dada-tbd-master
git merge --squash feature/webui-merge-planning
git commit -m "TBD-16 v2.0: macro system, PicoSeqRack, WebUI rework, sample manager"
```

### Phase 3 — Push to dadamachines/ctag-tbd

```bash
git push upstream dada-tbd-master
```

This makes the stabilized branch the new production release for TBD-16.

### Phase 4 — Contribute Core Improvements Back Upstream

Identify commits from the TBD-16 work that benefit the core platform and PR them to `ctag-fh-kiel/ctag-tbd` `p4_main`:

- Audio codec improvements
- PSRAM management fixes  
- Sample ROM bug fixes
- New GPLv3-compatible DSP plugins
- WiFi/network stack improvements
- ESP-IDF configuration updates

---

## Relationship to Configurable Build Proposal

The [proposal-simple-tbd-config.md](proposal-simple-tbd-config.md) is the technical foundation that makes this branching strategy work well. Without Kconfig flags:

- Every merge between `p4_main` and `dada-tbd-master` would conflict in SPManager.cpp, main.cpp, etc.
- Product code and core code would be tangled with no clear boundary

With Kconfig flags:

- `p4_main` builds with `CONFIG_TBD_USE_RP2350=n` — the product code compiles out cleanly
- `dada-tbd-master` builds with `CONFIG_TBD_USE_RP2350=y` — same source files, product code active
- Merge conflicts are minimized because both sides modify different `#ifdef` blocks

**Implementation order:**
1. Stabilize current branch (fix bugs, pass tests)
2. Implement Phase 1 of the config proposal (STORAGE_ROOT abstraction, Kconfig flags)
3. Merge into `dada-tbd-master`
4. Then: core improvements can start flowing back to `p4_main` cleanly

---

## Contributor Workflow Summary

### "I want to add a new DSP plugin" → ctag-fh-kiel/ctag-tbd

```bash
# Fork ctag-fh-kiel/ctag-tbd, branch from p4_main
# Add plugin to components/ctagSoundProcessor/
# PR to p4_main
# dadamachines picks it up on next sync
```

### "I want to add a new rack machine for TBD-16" → dadamachines/ctag-tbd

```bash
# Branch from dada-tbd-master
# Add machine to components/ctagSoundProcessor/rack/
# Add macro definition + presets to sdcard_image/data/
# PR to dada-tbd-master
```

### "I found a bug in the audio engine" → ctag-fh-kiel/ctag-tbd

```bash
# Fix on a branch from p4_main (core code only)
# PR to p4_main
# Then merge p4_main into dada-tbd-master
```

### "I want to improve the TBD-16 WebUI" → dadamachines/ctag-tbd

```bash
# Branch from dada-tbd-master
# Modify sdcard_image/www/ or www-prototype/
# PR to dada-tbd-master
```

---

## External Contributor Integration

The possan/macropresets branch integration is a template for how external contributors work on TBD-16 features:

1. Contributor forks `dadamachines/ctag-tbd`, branches from `dada-tbd-master`
2. Develops feature (macro system, new DSP machines, etc.)
3. dadamachines creates a merge planning document (like [MERGE-PLANNING.md](MERGE-PLANNING.md))
4. Merge is executed on a feature branch, tested, then merged into `dada-tbd-master`

For the possan merge specifically:
- **Source:** `possan/ctag-tbd` `macropresets` branch
- **Content:** 3,564 lines of macro system C++, 8,200 lines of rack DSP machines, 33 macro definitions, 58 sound presets
- **Integration:** Merged into `feature/webui-merge-planning` via the documented merge plan, with careful reconciliation of RestServer.cpp, SPManager.cpp, and component headers
- **Remote still tracked:** `possan` remote is configured in this repo for ongoing coordination

---

## Documentation Strategy

### History

The ctag-tbd documentation has a fragmented history:

- **`ctag-fh-kiel/ctag-tbd` `p4_main`** — Has **zero documentation** in `docs/`. The `p4_main` branch never received any Sphinx docs.
- **`ctag-fh-kiel/ctag-tbd` `dev`** — An older pre-P4 branch (now stale) contained the original Sphinx/Furo documentation setup with basic platform documentation.
- **`dadamachines/ctag-tbd` `dada-tbd-master`** — dadamachines imported the Sphinx docs from `dev`, then invested weeks of work expanding them significantly. The result is a comprehensive documentation site with custom branding.

### Current Documentation Contents

The `docs/` directory contains ~159 files organized as a Sphinx site with the Furo theme:

| Section | Content | Upstream relevance |
|---------|---------|-------------------|
| **`get_started/`** | Using TBD, WiFi & Link, storage, audio interface | **Core** — applicable to any TBD with minor adaptation |
| **`plugins/`** | 48 plugin reference pages, architecture docs, building guide, simulator, web API reference | **Core** — all plugins ship with every TBD build |
| **`flash/`** | Flashing DSP firmware, UI, device recovery | **Core** — process is similar, just different binaries |
| **`hardware/`** | TBD-16, TBD-Core specs, custom integration guide | **dadamachines-specific** — needs rewrite for upstream (open hardware ESP32-P4 reference design) |
| **`config/`** | Sphinx conf.py, Doxygen config, Dockerfile | **Shared** — theme/branding settings differ |
| **`about/`** | Credits, community links | **Shared** — with different project attribution |
| **`apps/`** | Application guides (groovebox, MCL, MIDI controller, multi-effect, RP2350, bootloader) | **Mixed** — some are TBD-16-specific (RP2350, MCL), others are universal |
| **`interviews/`** | Artist interviews (Bill Youngman, Eric D. Clark, Jessica Kert, Robert Henke) | **dadamachines only** — product marketing content |
| **`blog/`** | Blog posts | **dadamachines only** — product updates |
| **`faq.rst`** | FAQ | **Shared** — with product-specific entries split out |
| **`_static/brand.css`** | 1,110 lines of custom CSS: Lelo typeface, dadamachines colors, hero sections, feature cards | **dadamachines only** — upstream uses default Furo styles |
| **`_static/assets/`** | dadamachines product photos, logos, fonts | **dadamachines only** |
| **`_templates/`** | Custom Sphinx templates (layout, page, ablog) | **Mixed** — some tweaks are branding, some are structural |
| **`_includes/`** | Footer links, newsletter signup | **dadamachines only** |
| **`index.rst`** | Landing page with dadamachines hero, product marketing copy | **dadamachines only** — upstream needs a platform-focused landing page |

### Plan: Split Documentation for Upstream

The goal is to bring core platform documentation back to `ctag-fh-kiel/ctag-tbd` `p4_main`, while keeping dadamachines product documentation in its fork.

#### Goes upstream (core platform docs)

| Section | Adaptation needed |
|---------|-------------------|
| `plugins/` (all 48 plugin pages) | None — plugins are the same everywhere |
| `plugins/architecture.rst` | None |
| `plugins/building.rst` | Minor — adjust partition sizes for no-SD config |
| `plugins/simulator.rst` | None (once simulator is updated, see below) |
| `plugins/web-api.rst` | Exclude macro API section; keep plugin/device/sample endpoints |
| `plugins/getting-started.rst`, `step-by-step.rst` | None |
| `get_started/` (all pages) | Adapt storage section for flash-only setup; remove SD card references where `CONFIG_TBD_USE_SD_CARD=n` |
| `flash/` | Adapt for single-firmware flash (no RP2350 step); simpler partition layout |
| `about/credits.rst` | Update attribution for ctag-fh-kiel project |
| `faq.rst` | Keep platform questions, remove TBD-16-specific entries |
| `config/conf.py` | Strip dadamachines branding: `project = 'CTAG TBD'`, remove Lelo font, use default Furo theme without custom CSS |
| `index.rst` | New platform-focused landing page (not product marketing) |

#### Stays in dadamachines only

| Section | Reason |
|---------|--------|
| `interviews/` | Product marketing — artist profiles for dadamachines brand |
| `blog/` | Product update posts |
| `hardware/` (current content) | TBD-16 and TBD-Core product specs |
| `apps/rp2350.rst`, `apps/mcl.rst` | Require RP2350 hardware |
| `_static/brand.css` | 1,110 lines of dadamachines branding (Lelo font, custom colors, hero sections) |
| `_static/assets/` (product photos, logos, fonts) | dadamachines brand assets |
| `_includes/newsletter.rst` | dadamachines newsletter signup |
| Custom `index.rst` landing page | dadamachines product marketing hero |

#### Needs new content for upstream

| Section | What to write |
|---------|---------------|
| `hardware/` | Open-source / open-hardware ESP32-P4 reference design for a "simple TBD" — schematic, BOM, PCB layout, build guide. This aligns with Config A (Minimal TBD) from the configurable build proposal. |
| `index.rst` | Platform-focused landing page: "CTAG TBD is an open-source audio DSP platform for ESP32-P4" — not product marketing but project introduction |
| `apps/` | Adapt to show only apps that work without RP2350 (groovebox, multi-effect, MIDI controller, debugging, utilities) |

#### Theme & Branding

| Aspect | Upstream (ctag-tbd) | dadamachines |
|--------|--------------------:|-------------:|
| Sphinx theme | Furo (default styles) | Furo + `brand.css` (1,110 lines) |
| Font | System default (Furo default) | Lelo (custom dadamachines typeface) |
| Logo | ctag-tbd project logo or none | dadamachines logo (light/dark variants) |
| Colors | Furo default purple/blue | dadamachines custom palette |
| `conf.py` `project` | `'CTAG TBD'` | `'dadamachines tbd'` |
| `conf.py` `author` | `'CTAG creative technologies AG'` | `'dadamachines'` |
| Blog (ABlog) | Not included | Enabled with product posts |
| Hero landing page | Simple RST intro | Custom HTML hero with product photos |

The upstream docs should use **stock Furo** — the same theme, just without the custom `brand.css`, font files, and product imagery. This was the approach in the original `dev` branch before dadamachines added the branding layer.

#### Shared Content Maintenance

For sections that exist in both repos (plugins, getting started, flash guides), the content should be maintained in `dadamachines/ctag-tbd` and periodically synced upstream — or ideally kept identical with conditional RST directives:

```rst
.. only:: tbd16

   The TBD-16 includes an SD card for sample storage.

.. only:: not tbd16

   Samples are stored in a flash partition using the .tbd format.
```

Sphinx's `only` directive combined with a tag set in `conf.py` (`tags.add('tbd16')` in dadamachines, absent in upstream) can gate product-specific paragraphs without forking the files.

---

## Simulator: v2 API Migration Needed

### Current State

The simulator (`simulator/`) is **stuck on API v1**. The firmware and WebUI were migrated to v2 (documented in [API-V1-TO-V2-MIGRATION.md](API-V1-TO-V2-MIGRATION.md)), but the simulator's `WebServer.cpp` was not updated.

**`simulator/WebServer.cpp`** — 570 lines, **22 route handlers all using `/api/v1/` paths**:

```
/api/v1/getPlugins              → needs → /api/v2/plugins?action=list
/api/v1/getActivePlugin/N       → needs → /api/v2/plugins?action=getActive&ch=N
/api/v1/getPluginParams/N       → needs → /api/v2/plugins?action=getParams&ch=N
/api/v1/setActivePlugin/N       → needs → /api/v2/plugins?action=setActive&ch=N
/api/v1/setPluginParam/N        → needs → /api/v2/plugins?action=setParam&ch=N&...
/api/v1/setPluginParamCV/N      → needs → (merged into setParam with key=cv)
/api/v1/setPluginParamTRIG/N    → needs → (merged into setParam with key=trig)
/api/v1/getPresets/N            → needs → /api/v2/plugins?action=getPresets&ch=N
/api/v1/getPresetData/X         → needs → /api/v2/plugins?action=getPresetData&id=X
/api/v1/setPresetData/X         → needs → /api/v2/plugins?action=setPresetData&id=X
/api/v1/getConfiguration        → needs → /api/v2/device?action=getConfig
/api/v1/setConfiguration        → needs → /api/v2/device?action=setConfig
/api/v1/getIOCaps               → needs → /api/v2/device?action=getIOCaps
/api/v1/reboot                  → needs → /api/v2/device?action=reboot
/api/v1/favorites/*             → needs → /api/v2/device?action=getFavorites|storeFavorite|recallFavorite
/api/v1/samples                 → needs → /api/v2/samples
/api/v1/srom/getSize            → needs → (review if still needed)
```

**`simulator/www/ui.html`** — 446 lines. This is a standalone modulation control UI (sliders for CV/trig simulation). It only uses `/ctrl-set` and `/ctrl-get` endpoints (not the plugin API), so it doesn't need v2 migration. However, when the simulator serves the main WebUI from `sdcard_image/www/`, that WebUI speaks v2 — and the simulator's C++ server only understands v1.

### What Needs to Happen

The simulator's `WebServer.cpp` needs to be rewritten to match the v2 action-based dispatch pattern. The v1 approach used 22 separate regex-matched route handlers. The v2 approach consolidates these into ~5 handlers that parse `?action=` from query strings:

```
/api/v2/plugins   GET  → parse ?action= → dispatch to list|getActive|getParams|getPresets|getPresetData|getAll
/api/v2/plugins   POST → parse ?action= → dispatch to setActive|setParam|savePreset|loadPreset|setPresetData
/api/v2/device    GET  → parse ?action= → dispatch to getConfig|getIOCaps|getFavorites|getAll
/api/v2/device    POST → parse ?action= → dispatch to setConfig|reboot|storeFavorite|recallFavorite
/api/v2/samples   GET/POST → (same pattern as firmware)
```

Additionally, the new v2 bulk endpoints (`getAll` for plugins and device) should be implemented in the simulator to match the firmware behavior and reduce WebUI load time during development.

The macro API endpoints (`/api/v2/macros`) don't need to be implemented in the simulator — macros are dadamachines-specific and the simulator is a core platform tool. The WebUI will detect missing macro endpoints and hide those sections (per the capabilities/feature-gating approach described earlier).

### Priority

This is a **medium-priority task** — the simulator is a development tool, not end-user-facing. But it's currently broken for anyone trying to develop or test the WebUI without hardware, which blocks frontend development work. It should be fixed as part of the stabilization phase before merging into `dada-tbd-master`.

---

*Related documents:*
- [proposal-simple-tbd-config.md](proposal-simple-tbd-config.md) — Kconfig-based hardware configuration
- [MERGE-PLANNING.md](MERGE-PLANNING.md) — Detailed merge execution log for the possan integration
- [API-V1-TO-V2-MIGRATION.md](API-V1-TO-V2-MIGRATION.md) — Complete v1→v2 API endpoint mapping and WebUI changes

*Generated: 2026-03-07*
