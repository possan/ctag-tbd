# Track Default Presets — Integration Guide

## Overview

This document describes the new **Track Default Presets** feature that lets the ESP32-P4 control which machine/preset each track loads on boot, replacing the previous random selection.

The configuration lives entirely on the **P4's SD card** (`/data/trackdefaults.json`), making it editable via WebUI or direct SD card access — no Pico reflashing needed.

---

## Architecture

```
┌──────────────────┐         SPI (0xA5)          ┌──────────────────────────┐
│   Pico RP2350    │ ──── GetTrackDefaultPresets ──▶  ESP32-P4             │
│   (Sequencer)    │ ◀──── JSON response ──────── │  (Audio DSP)           │
│                  │                              │                        │
│  initializeMacro │                              │  Reads:                │
│  Presets()       │                              │  /sdcard/data/         │
│                  │         SPI (0xA4)           │   trackdefaults.json   │
│  For each track: │ ──── LoadTrackSoundPreset ──▶│                        │
│  apply preset    │                              │  Activates machines    │
└──────────────────┘                              └──────────────────────────┘
```

### Boot sequence

1. Pico boots and calls `initializeMacroPresets()`
2. Pico sends SPI command `0xA5` (`GetTrackDefaultPresets`) to the P4
3. P4 reads `/sdcard/data/trackdefaults.json` from its own SD card
4. P4 returns the JSON contents to the Pico via SPI
5. Pico parses the response and, for each track:
   - If a preset is specified → applies it via `LoadTrackSoundPreset` (SPI `0xA4`)
   - If not specified → queries `GetMacroSoundPresetList` (SPI `0xA0`) and picks the first available preset
6. After boot, the user can override any track's preset at runtime via the OLED UI (or future WebUI)

### Runtime override priority

```
Boot defaults (from P4 trackdefaults.json)
    ↓  overridden by
OLED UI → Sound Preset screen (user picks a preset)
    ↓  overridden by
Future: WebUI live preset change (same SPI path)
```

All paths converge on the same `BACKGROUNDUPDATE_TYPE_APPLYSOUNDPRESET` handler, so they are fully interchangeable.

---

## What's Already Implemented

### Pico side (this repo: `tbd-pico-seq3`)

**Branch:** `feature/deterministic-track-machines`

| File | Change |
|------|--------|
| `src/SpiAPI.h` | Added `GetTrackDefaultPresets = 0xA5` to the `RequestType_t` enum and method declaration |
| `src/SpiAPI.cpp` | Added `GetTrackDefaultPresets()` method — sends SPI `0xA5`, receives JSON response |
| `examples/main.cpp` | Rewrote `initializeMacroPresets()` — queries P4 for defaults, falls back to first preset |

### P4 side (repo: `ctag-tbd_hacking`)

**Branch:** `feature/webui-merge-planning`

| File | Change |
|------|--------|
| `main/SpiAPI.hpp` | Added `GetTrackDefaultPresets = 0xA5` to the `RequestType` enum |
| `main/SpiAPI.cpp` | Added handler: reads `/sdcard/data/trackdefaults.json`, returns contents via `transmitCString()` |
| `sdcard_image/data/trackdefaults.json` | Default config file with all 19 tracks mapped |

---

## P4 / WebUI Developer Tasks

### 1. Verify the SPI handler (already committed)

The handler in `main/SpiAPI.cpp` simply reads the file and returns it:

```cpp
case RequestType::GetTrackDefaultPresets:
{
    std::string json = "{}";
    FILE *f = fopen("/sdcard/data/trackdefaults.json", "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        if (sz > 0 && sz < 8192) {
            char *buf = (char*)malloc(sz + 1);
            if (buf) {
                fread(buf, 1, sz, f);
                buf[sz] = '\0';
                json = buf;
                free(buf);
            }
        }
        fclose(f);
    }
    result = transmitCString(requestType, json.c_str());
}
break;
```

If the file doesn't exist, it returns `"{}"` and the Pico falls back gracefully.

### 2. Create a WebUI page to edit `trackdefaults.json`

Build a simple UI page that:

1. **Reads** `/sdcard/data/trackdefaults.json` (or creates it if missing)
2. **Displays** a table of all 19 tracks with:
   - Track index and name (from `synthdefinitions.json`)
   - Current preset ID
   - A dropdown/selector populated from `data/macrosoundpresets/` (filtered to presets valid for that track)
3. **Saves** the updated JSON back to `/sdcard/data/trackdefaults.json`

#### `trackdefaults.json` format

```json
{
  "tracks": [
    { "index": 0,  "preset": "db-all-def" },
    { "index": 1,  "preset": "fmb-all-def" },
    { "index": 2,  "preset": "ds-all-def" },
    ...
  ]
}
```

- `index` — track number (0–18)
- `preset` — preset ID (filename without `.json` from `data/macrosoundpresets/`)
- Optional `_name` and `_comment` fields are ignored by the firmware (for human readability)
- Omit a track entry to let the Pico auto-select the first available preset

### 3. Track definition reference

The 19 tracks are defined in `sdcard_image/data/synthdefinitions.json`:

| Index | Name     | Type  | MIDI Ch | Available Machines |
|-------|----------|-------|---------|-------------------|
|  0    | Kick     | drum  | 9       | nodrum, db, ab, ro, extdrum |
|  1    | Kick2    | drum  | 9       | nodrum, fmb, ro, extdrum |
|  2    | Snare    | drum  | 9       | nodrum, ds, as, ro, extdrum |
|  3    | Hat      | drum  | 10      | nodrum, hh1, hh2, ro, extdrum |
|  4    | Rimshot  | drum  | 10      | nodrum, rs, ro, extdrum |
|  5    | Clap     | drum  | 10      | nodrum, cl, ro, extdrum |
|  6    | Rompler  | drum  | 11      | nodrum, ro, extdrum |
|  7    | Rompler  | drum  | 11      | nodrum, ro, extdrum |
|  8    | Bass     | synth | 0       | nosynth, td3, ro, extsynth |
|  9    | Bass2    | synth | 1       | nosynth, td3, ro, extsynth |
| 10    | Lead     | synth | 2       | nosynth, mo, ro, extsynth |
| 11    | Lead2    | synth | 3       | nosynth, wtosc, mo, ro, extsynth |
| 12    | Rompler  | synth | 4       | nosynth, ro, extsynth |
| 13    | Rompler  | synth | 5       | nosynth, ro, extsynth |
| 14    | Chordo   | synth | 6       | nosynth, pp, ro, extsynth |
| 15    | Input    | synth | 7       | nosynth, inp |
| 16    | FX1      | fx    | 12      | nofx, fxdelay |
| 17    | FX2      | fx    | 12      | nofx, fxreverb |
| 18    | Master   | fx    | 12      | nofx, fxmaster |

### 4. Available preset IDs (current)

These are the filenames in `data/macrosoundpresets/` (without `.json`):

```
ab-all-def          fmb-all-def         msp-acidbass2       msp-morphpad1
as-all-def          fxdelay-all-def     msp-darkchord1      msp-morphpad2
cl-all-def          fxmaster-all-def    msp-darkchord2      msp-phatpunch1
db-all-def          fxreverb-all-def    msp-deepfm1         msp-phatpunch2
ds-all-def          golem               msp-deepfm2         msp-submorph1
ds-snap1            hh1-all-def         msp-kick1           new-kick
extdrum-all-def     hh2-all-def         msp-lushpad1        nodrum-all-def
extsynth-all-def    inp-all-def         msp-lushpad2        nofx-all-def
                    mo-all-def          msp-metallic1       nosynth-all-def
                    msp-acidbass1                           pp-all-def
                                                            punch-testing
                                                            ro-all-def
                                                            rs-all-def
                                                            td3-all-def
                                                            wo-all-def
                                                            wtosc-all-def
```

**Important:** Not every preset is valid for every track. The P4 cross-references each preset's `macroDeviceId` against the track's `machines[]` list in `synthdefinitions.json`. The WebUI should use `GetMacroSoundPresetList` (SPI `0xA0`) per track to get the filtered list, or replicate the same filtering logic.

### 5. Optional: Live reload without reboot

Currently `trackdefaults.json` is only read at boot. If you want a "apply now" button in the WebUI that also pushes the preset change to the Pico immediately (without rebooting), there are two options:

**Option A — WebUI calls existing SPI commands directly:**
The P4 can call `LoadTrackMacroAndPreset(trackIndex, presetId)` internally for each track when the user clicks "Apply" in the WebUI. This changes the running preset on the DSP side. The Pico will pick it up on next preset list query.

**Option B — New SPI command to trigger re-init:**
Add a new SPI command (e.g. `0xA6 ReloadTrackDefaults`) that causes the Pico to re-run `initializeMacroPresets()`. The P4 would need to forward this via SPI, or the Pico could poll periodically.

Option A is simpler and doesn't require Pico firmware changes.

---

## Future: Pico Hardware UI Extensions

### Current state of the SequencerUI

The OLED/button UI already has:

- **SoundPresetScreen** (`lib/sequencerui/screens/soundpreset.cpp`) — browse and apply sound presets per track via the `host->requestApplySoundPreset()` interface
- **TrackPresetScreen** (`lib/sequencerui/screens/trackpreset.cpp`) — browse and apply track presets (pattern + sound combined)
- **IHostInterface** (`lib/sequencerui/host.hpp`) — abstraction layer with `requestApplySoundPreset(trackIndex, presetId)`

### How runtime override works

When a user selects a preset on the OLED:

```cpp
// In SoundPresetScreen::input(), on selection confirm:
this->sequi->host->requestApplySoundPreset(activeTrackIndex, presetId);
```

This posts a `BACKGROUNDUPDATE_TYPE_APPLYSOUNDPRESET` event to the same queue that the boot-time init uses. The handler in `main.cpp` then calls:
1. `spi_api.LoadTrackSoundPreset(trackIndex, presetId)` — tells P4 to switch
2. `spi_api.GetMacroSoundPreset(presetId, resp)` — gets preset metadata
3. `spi_api.GetMacroDefinition(macroId, resp)` — gets parameter definitions
4. Updates the UI state (param pages, preset name display, etc.)

**This means the boot defaults from `trackdefaults.json` are always overridable at runtime.** The boot config is just the power-on state.

### Planned UI extensions

#### Machine Selector Screen (new)

A future screen to change the active **machine** (not just preset) per track:

```
┌─────────────────────────────────┐
│  SELECT MACHINE — Track 0 Kick  │
│ ────────────────────────────────│
│  > Synth Kick (db)          [*] │
│    Analog Kick (ab)             │
│    Rompler (ro)                 │
│    External MIDI (extdrum)      │
│    None (nodrum)                │
└─────────────────────────────────┘
```

Implementation steps:
1. Create `screens/machinesel.cpp` / `.hpp`
2. On enter: call `host->requestMachineList(trackIndex)` (new method, uses existing `GetMacroSoundPresetList` or a new SPI command to get available machines)
3. On select: call `host->requestActivateTrackMachine(trackIndex, machineId)` (maps to existing SPI `0xA3 ActivateTrackMachine`)
4. After machine change: refresh the sound preset list for that track (the available presets depend on the active machine)

#### Save Current Setup as New Default

Allow the user to save the current track setup back to `trackdefaults.json`:

1. Add a "Save as Boot Default" option in the system menu
2. Gather current preset IDs from all tracks
3. Send to P4 via a new SPI command (e.g. `0xA6 SaveTrackDefaultPresets`) that writes `trackdefaults.json`

This keeps the single-source-of-truth on the P4 SD card while letting the Pico UI modify it.

### Existing SPI commands available for UI features

| ID | Command | Direction | Use case |
|----|---------|-----------|----------|
| `0xA0` | `GetMacroSoundPresetList` | P4→Pico | List presets valid for a track |
| `0xA1` | `GetMacroSoundPreset` | P4→Pico | Get full preset details (for param display) |
| `0xA2` | `GetMacroDefinition` | P4→Pico | Get macro device param definitions |
| `0xA3` | `ActivateTrackMachine` | Pico→P4 | Switch a track's active machine |
| `0xA4` | `LoadTrackSoundPreset` | Pico→P4 | Load a specific preset on a track |
| `0xA5` | `GetTrackDefaultPresets` | P4→Pico | Read boot defaults (new) |

All infrastructure for runtime machine/preset switching is already in place. Future OLED UI screens just need to call into `IHostInterface` methods that map to these SPI commands.

---

## Testing Checklist

- [ ] Flash P4 with updated firmware (branch `feature/webui-merge-planning`)
- [ ] Ensure `trackdefaults.json` is on the P4 SD card at `/data/trackdefaults.json`
- [ ] Flash Pico with updated firmware (branch `feature/deterministic-track-machines`)
- [ ] Boot: verify serial debug output shows presets loaded from `trackdefaults.json`
- [ ] Modify `trackdefaults.json` on the SD card, reboot, verify different presets load
- [ ] Delete `trackdefaults.json`, reboot, verify fallback to first available preset per track
- [ ] Use OLED UI Sound Preset screen to change a preset at runtime — verify it overrides the boot default
- [ ] WebUI: build editor page, save changes, reboot, verify new defaults take effect
