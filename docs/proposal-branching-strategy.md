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

The `feature/webui-merge-planning` branch was created by merging two feature branches (documented in [MERGE-PLANNING.md](../prototyping/MERGE-PLANNING.md)):

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
│  PicoSeqRack, rack/* DSP machines, TBD-16 WebUI,   │
│  product config, deployment tooling                  │
├─────────────────────────────────────────────────────┤
│  CORE PLATFORM (ctag-fh-kiel, GPLv3)               │
│  ESP-IDF setup, audio codec, PSRAM management,      │
│  plugin architecture, DSP plugin base class,         │
│  sample ROM, core plugins, REST server skeleton,     │
│  WiFi/network, filesystem abstraction                │
└─────────────────────────────────────────────────────┘
```

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
3. dadamachines creates a merge planning document (like [MERGE-PLANNING.md](../prototyping/MERGE-PLANNING.md))
4. Merge is executed on a feature branch, tested, then merged into `dada-tbd-master`

For the possan merge specifically:
- **Source:** `possan/ctag-tbd` `macropresets` branch
- **Content:** 3,564 lines of macro system C++, 8,200 lines of rack DSP machines, 33 macro definitions, 58 sound presets
- **Integration:** Merged into `feature/webui-merge-planning` via the documented merge plan, with careful reconciliation of RestServer.cpp, SPManager.cpp, and component headers
- **Remote still tracked:** `possan` remote is configured in this repo for ongoing coordination

---

*Related documents:*
- [proposal-simple-tbd-config.md](proposal-simple-tbd-config.md) — Kconfig-based hardware configuration
- [MERGE-PLANNING.md](../prototyping/MERGE-PLANNING.md) — Detailed merge execution log for the possan integration

*Generated: 2026-03-07*
