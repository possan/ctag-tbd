# Macro and Preset Definition Concept

> **Date:** March 3, 2026
> **Status:** Working implementation on `macropresets` branch

---

## 1. Problem Statement

PicoSeqRack is a 16-track drum machine / synth rack running on the ESP32-P4. Each track hosts a selectable DSP engine (machine) with many raw parameters exposed as MIDI CCs. The raw parameter space is large (up to 16 CCs per track × 16 tracks = 256+ parameters) and the parameters are low-level DSP controls — not intuitive for performers or sound designers.

The **Macro and Preset system** introduces two abstraction layers that sit between the user and the raw DSP parameters:

1. **Macro Definitions** — reduce and reshape the parameter space for a given machine into a curated set of virtual knobs (up to 6 pages × 4 knobs = 24 virtual parameters)
2. **Sound Presets** — store specific value snapshots of a macro definition's virtual knobs, enabling instant recall of sounds

Together they enable a workflow where sound designers create macro definitions and factory presets, while performers simply browse and tweak presets without needing to understand the underlying DSP.

---

## 2. Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                           WebUI / OLED                             │
│   ┌──────────────┐  ┌──────────────────┐  ┌──────────────────────┐ │
│   │ Sound Preset │  │ Macro Definition │  │ Machine Definition   │ │
│   │  (values[])  │──│  (groups/mapping)│──│  (synthdefinitions)  │ │
│   └──────────────┘  └──────────────────┘  └──────────────────────┘ │
└───────────────────────────┬─────────────────────────────────────────┘
                            │ POST /api/v1/macroapi
                            ▼
                    ┌───────────────────┐
                    │  MacroTranslator  │
                    │  (C++ on ESP32)   │
                    │                   │
                    │  For each output  │
                    │  mapping entry:   │
                    │                   │
                    │  value = start +  │
                    │   Σ(param[src] ×  │
                    │     mul / div)    │
                    └─────────┬─────────┘
                              │ handleMidiControlChange(ch, cc, value)
                              ▼
                    ┌───────────────────┐
                    │  PicoSeqRack DSP  │
                    │  Engine           │
                    │                   │
                    │  CC → parameter   │
                    │  variable (0-4095)│
                    └───────────────────┘
```

---

## 3. The Three Definitions

### 3.1 Machine Definition (`synthdefinitions.json`)

The **machine definition** file describes the hardware layer: what tracks exist, which machines can be loaded on each track, and what raw MIDI CC parameters each machine exposes. This file lives at `/sdcard/data/synthdefinitions.json` and is typically not edited by end users — it is a fixed description of the PicoSeqRack firmware capabilities.

**Structure:**

```json
{
  "tracks": [
    {
      "index": 0,
      "type": "drum",
      "name": "Kick",
      "midichannel": 9,
      "drumnote": 36,
      "basecc": 0,
      "machines": ["nodrum", "db", "ab", "extdrum"]
    },
    {
      "index": 8,
      "type": "synth",
      "name": "Bass 1",
      "midichannel": 0,
      "drumnote": -1,
      "basecc": 0,
      "machines": ["nosynth", "td3"]
    },
    {
      "index": 16,
      "type": "fx",
      "name": "Delay FX",
      "midichannel": 13,
      "drumnote": -1,
      "basecc": 20,
      "machines": ["nofx", "fxdelay"]
    }
  ],
  "machines": [
    {
      "id": "db",
      "name": "Digital Bass Drum",
      "type": "drum",
      "parameters": [
        { "id": "freq",  "name": "Frequency", "type": "cc", "ctrl": 8,  "def": 0 },
        { "id": "decay", "name": "Decay",     "type": "cc", "ctrl": 9,  "def": 0 },
        { "id": "tone",  "name": "Tone",      "type": "cc", "ctrl": 10, "def": 0 }
      ]
    }
  ]
}
```

**Key fields:**

| Field | Description |
|-------|-------------|
| `tracks[].midichannel` | MIDI channel the track listens on |
| `tracks[].basecc` | CC offset — all CCs for this track are relative to this base |
| `tracks[].machines` | List of machine IDs that can be loaded on this track |
| `machines[].parameters[].ctrl` | CC offset within the track for this parameter |

The actual MIDI CC number sent to the engine is: `tracks[].basecc + machines[].parameters[].ctrl`

### 3.2 Macro Definitions

A **macro definition** creates a user-friendly "abstraction skin" over a machine's raw parameters. It defines:

- **Virtual parameters** — named knobs organized into pages (groups), each with a name, default value, range, and UI presentation hint
- **Output mappings** — rules that compute raw CC values from virtual parameter values

Macro definition files live at `/sdcard/data/macrodefinitions/<id>.json`.

**Example** (`ds-snappy.json` — a curated 3-knob interface for the Digital Snare):

```json
{
  "id": "ds-snappy",
  "name": "DS Snappy",
  "machine": "ds",
  "groups": [
    {
      "name": "Page 1",
      "parameters": [
        { "idx": 0, "name": "Freq",  "def": 32, "min": 0, "max": 127, "res": 64, "ui": "bignum" },
        { "idx": 1, "name": "Decay", "def": 16, "min": 0, "max": 127, "res": 64, "ui": "bignum" },
        { "idx": 2, "name": "Tone",  "def": 16, "min": 0, "max": 127, "res": 64, "ui": "bignum" }
      ]
    }
  ],
  "mapping": [
    { "ctrl": 8,  "start": 0, "add": [{ "src": 0, "mul": 1, "div": 1 }] },
    { "ctrl": 9,  "start": 0, "add": [{ "src": 1, "mul": 1, "div": 1 }] },
    { "ctrl": 10, "start": 0, "add": [{ "src": 2, "mul": 1, "div": 1 }] },
    { "ctrl": 11, "start": 64 },
    { "ctrl": 12, "start": 32 }
  ]
}
```

**Field reference:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier, used as the filename |
| `name` | string | Human-readable display name |
| `machine` | string | Target machine ID from `synthdefinitions.json` |
| `groups` | array | Parameter groups (pages), up to 6 |
| `groups[].name` | string | Page display name |
| `groups[].parameters` | array | Parameters on this page, up to 4 per page |
| `groups[].parameters[].idx` | number | Sequential parameter index (0-based) |
| `groups[].parameters[].name` | string | Knob display name |
| `groups[].parameters[].def` | number | Default value |
| `groups[].parameters[].min` | number | Minimum value |
| `groups[].parameters[].max` | number | Maximum value |
| `groups[].parameters[].res` | number | Resolution / step size hint |
| `groups[].parameters[].curve` | string | Response curve: `"linear"` (default), `"log"`, `"exp"`, `"scurve"`. Defines the value-to-CC mapping curve. Not yet implemented in firmware — stored for future use. |
| `groups[].parameters[].ui` | string | UI presentation type (`"bignum"`, `"freq"`, `"curve1"`) |
| `mapping` | array | Output mapping rules |
| `mapping[].ctrl` | number | Target CC offset (relative to the track's `basecc`) |
| `mapping[].start` | number | Constant base value for this CC |
| `mapping[].add` | array? | Additive sources (omit or empty for constant-only) |
| `mapping[].add[].src` | number | Source virtual parameter index |
| `mapping[].add[].mul` | number | Multiplier (numerator of scaling fraction) |
| `mapping[].add[].div` | number | Divider (denominator of scaling fraction) |

### 3.3 Sound Presets

A **sound preset** stores a specific set of virtual parameter values for a given macro definition. It represents a "ready to use" sound.

Sound preset files live at `/sdcard/data/macrosoundpresets/<id>.json`.

**Example** (`ds-snap1.json`):

```json
{
  "id": "ds-snap1",
  "name": "Snappy",
  "group": "Snares",
  "macro": "ds-snappy",
  "values": [68, 16, 16]
}
```

**Field reference:**

| Field | Type | Description |
|-------|------|-------------|
| `id` | string | Unique identifier, used as the filename |
| `name` | string | Human-readable preset name |
| `group` | string | Category for organizing presets (e.g. "Kicks", "Snares", "Hats") |
| `macro` | string | ID of the macro definition this preset is for |
| `values` | number[] | Virtual parameter values in order matching the macro's `groups[].parameters[].idx` |

The `values` array indices correspond to the sequential parameter indices across all groups of the referenced macro definition. A value of `-1` means "use the macro definition's default value."

---

## 4. The Mapping Formula

The mapping formula translates virtual parameter values into raw CC values sent to the DSP engine. For each output mapping entry:

$$\text{ccValue} = \text{start} + \sum_{i} \frac{\text{paramValues}[\text{src}_i] \times \text{mul}_i}{\text{div}_i}$$

The result is clamped to `0–127` before being sent as a MIDI CC message.

### Mapping Capabilities

| Pattern | Description | Example |
|---------|-------------|---------|
| **1:1 passthrough** | One virtual knob controls one CC directly | `"add": [{"src": 0, "mul": 1, "div": 1}]` |
| **Scaled** | Fractional scaling of a virtual parameter | `"add": [{"src": 0, "mul": 3, "div": 10}]` (30% of knob value) |
| **Constant lock** | CC is fixed regardless of knob positions | `"ctrl": 11, "start": 64` (no `add` array) |
| **Many-to-one** | Multiple virtual knobs sum into one CC | `"add": [{"src": 0, "mul": 1, "div": 2}, {"src": 1, "mul": 1, "div": 2}]` |
| **One-to-many** | One virtual knob referenced from multiple mappings | Two mapping entries both with `"src": 0` in their `add` arrays |

### Example Walkthrough

Given the `ds-snappy` macro loaded on a track with `basecc = 0`, and sound preset values `[68, 16, 16]`:

| Mapping | Computation | Result CC | Sent as |
|---------|-------------|-----------|---------|
| `ctrl:8, start:0, src:0 ×1/1` | 0 + (68 × 1/1) = 68 | CC 8 = 68 | Frequency |
| `ctrl:9, start:0, src:1 ×1/1` | 0 + (16 × 1/1) = 16 | CC 9 = 16 | Decay |
| `ctrl:10, start:0, src:2 ×1/1` | 0 + (16 × 1/1) = 16 | CC 10 = 16 | Tone |
| `ctrl:11, start:64` (constant) | 64 | CC 11 = 64 | Noise (locked) |
| `ctrl:12, start:32` (constant) | 32 | CC 12 = 32 | Accent (locked) |

---

## 5. From CC Value to DSP Parameter — The Full Scaling Chain

The mapping formula (Section 4) produces a CC value in the range 0–127. But the DSP engines work with physical quantities — frequencies in Hz, decay times in seconds, ratios, etc. This section explains how the CC value travels through three transformation stages before it reaches the audio algorithm.

### 5.1 Stage 1: CC → Raw Internal Value (0–4095)

When `MacroTranslator` sends a CC value, `PicoSeqRack::handleMidiControlChange()` scales it from the MIDI 0–127 range to an internal 0–4095 range:

```cpp
// in ctagSoundProcessorPicoSeqRack.cpp, line ~638
void ctagSoundProcessorPicoSeqRack::handleMidiControlChange(
    const uint8_t channel, const uint8_t control, const uint8_t value) {
    int cv_value = ((int)value * 4096) / 128;   // 0–127 → 0–4095
    auto it = pMapParCC.find(CC_TO_MAP_KEY(channel, control));
    if (it != pMapParCC.end()) {
        (it->second)(cv_value);  // calls the lambda registered by the Rack engine
    }
}
```

This 0–4095 integer is stored in an `atomic<int16_t>` variable inside the Rack sub-engine (e.g. `RackDBD::f0`, `RackTBD03::td3_cutoff`). The value sits there until the next audio `Process()` call reads it.

### 5.2 Stage 2: Raw Value → DSP Float (the MK_ Macros)

Each Rack engine's `Process()` function converts the raw 0–4095 integer into a floating-point value suitable for the DSP algorithm. This is done via a set of scaling macros defined in [ctagSoundProcessorPicoSeqRack.hpp](components/ctagSoundProcessor/ctagSoundProcessorPicoSeqRack.hpp). **This is where the actual parameter range and scaling curve are defined.**

#### The Scaling Macros

| Macro | Formula | Description |
|-------|---------|-------------|
| `MK_FLT_PAR_ABS_NOCV(out, in, norm, scale)` | `out = in / norm * scale` | **Linear scaling.** Maps 0–4095 to 0–`scale`. |
| `MK_FLT_PAR_ABS_MIN_MAX_NOCV(out, in, norm, min, max)` | `out = in/norm * (max-min) + min` | **Linear with offset.** Maps 0–4095 to `min`–`max`. |
| `MK_FLT_PAR_ABS_PAN_NOCV(out, in, norm, scale)` | `out = (in/norm + 1) / 2 * scale` | **Bipolar-to-unipolar.** For pan controls. |
| `MK_INT_PAR_ABS_NOCV(out, in, scale)` | `out = in * scale / 4096` | **Integer scaling.** Maps 0–4095 to 0–`scale`. |
| `MK_BOOL_PAR_NOCV(out, in)` | `out = in` (truthy/falsy) | **Boolean.** Non-zero = true. |

All of these are **linear** mappings. There is no built-in log/exponential curve at this stage.

#### Real Examples from Rack Engines

**Digital Bass Drum (`RackDBD`)** — [RackDBD.cpp](components/ctagSoundProcessor/rack/RackDBD.cpp):

```cpp
// in RackDBD::Process()
MK_FLT_PAR_ABS_MIN_MAX_NOCV(_f0, f0, 4095.f, 0.0005f, 0.01f)  // frequency: 0–4095 → 0.0005–0.01
MK_FLT_PAR_ABS_NOCV(_accent, accent, 4095.f, 1.f)               // accent:    0–4095 → 0.0–1.0
MK_FLT_PAR_ABS_NOCV(_tone, tone, 4095.f, 1.f)                   // tone:      0–4095 → 0.0–1.0
MK_FLT_PAR_ABS_NOCV(_decay, decay, 4095.f, 1.f)                 // decay:     0–4095 → 0.0–1.0
MK_FLT_PAR_ABS_NOCV(_dirty, dirty, 4095.f, 5.f)                 // dirty:     0–4095 → 0.0–5.0
MK_FLT_PAR_ABS_NOCV(_fmEnv, fm_env, 4095.f, 5.f)               // FM env:    0–4095 → 0.0–5.0
MK_FLT_PAR_ABS_NOCV(_fmDcy, fm_dcy, 4095.f, 4.f)               // FM decay:  0–4095 → 0.0–4.0
```

**TBD03 Acid Bass (`RackTBD03`)** — [RackTBD03.cpp](components/ctagSoundProcessor/rack/RackTBD03.cpp):

```cpp
// Filter cutoff — custom scaling, NOT using a macro:
float c = td3_cutoff / 4095.f;     // normalize to 0–1
c *= 27000.f;                       // scale to 0–27000
c -= 5000.f;                        // shift to -5000–22000
// ... then modified by filter envelope and accent
CONSTRAIN(c, 20.f, 22000.f)         // clamp to 20–22000 Hz

// Resonance — linear 0–1:
float r = td3_resonance / 4095.f;

// Drive — linear with offset:
float dri = td3_drive / 4095.f * 30.f;
CONSTRAIN(dri, 1.f, 30.f)

// Shape — discrete selector:
int s = td3_shape * 47 / 4096;      // maps to oscillator shape index
```

**Channel Mixer (`RackChannelMixer`)** — [RackChannelMixer.cpp](components/ctagSoundProcessor/rack/RackChannelMixer.cpp):

```cpp
MK_FLT_PAR_ABS_NOCV(fLev, mix_lev, 4096.f, 2.f);  fLev *= fLev;  // squared for perceptual loudness
MK_FLT_PAR_ABS_NOCV(fFX1Send, mix_fx1, 4095.f, 2.f); fFX1Send *= fFX1Send;  // also squared
```

**Global FX — Compressor** — in [PicoSeqRack Process()](components/ctagSoundProcessor/ctagSoundProcessorPicoSeqRack.cpp):

```cpp
MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompThresdB, c_thres, 4095.f, -80.f, 0.f)   // threshold: -80–0 dB
MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRatio, c_ratio, 4095.f, 0.0001f, 1.25f) // ratio
MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompAtk, c_atk, 4095.f, 0.3f, 30.f)        // attack: 0.3–30 ms
MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompRel, c_rel, 4095.f, 40.f, 2000.f)       // release: 40–2000 ms
MK_FLT_PAR_ABS_MIN_MAX_NOCV(fCompMUPGain, c_gain, 4095.f, 0.f, 60.f)     // gain: 0–60 dB
```

### 5.3 Stage 3: DSP Float → Audio Algorithm

The DSP float value is passed directly to the underlying algorithm (Mutable Instruments Plaits drums, Braids oscillators, ladder filters, etc.). Some algorithms apply their own internal non-linear curves — for example, the Plaits `SyntheticBassDrum::Render()` applies exponential pitch scaling internally. This is opaque to the macro system.

### 5.4 Where Linear vs. Log/Exponential Is Decided

**Currently, all scaling from CC to DSP float is linear.** There are no log or exponential curves built into the MK_ macros. However, non-linear behavior exists in two places:

1. **Post-macro squaring** — Some parameters are squared after linear scaling (e.g. volume and FX sends in `RackChannelMixer`) to approximate a perceptual loudness curve. This is hardcoded in the Rack engine's `Process()`.

2. **Inside the DSP algorithm** — Algorithms like Plaits drums or the TBD03 filter may apply their own exponential curves internally. The macro system has no control over this.

3. **The mul/div mapping fraction** — The macro definition's mapping formula can partially approximate non-linear curves by using creative scaling. For example, setting `"mul": 1, "div": 2` halves the effective range — but this is still a linear scaling within the mapping, not a true curve.

### 5.5 Summary: The Complete Value Chain

```
Sound Preset value (0–127)
    │
    ▼  Mapping formula: start + Σ(value × mul / div)
CC value (0–127, clamped)
    │
    ▼  handleMidiControlChange: value × 4096 / 128
Raw internal value (0–4095, stored in atomic<int16_t>)
    │
    ▼  MK_ macro in Process(): linear scaling to DSP range
DSP float (e.g. 0.0005–0.01 for frequency, 0.0–1.0 for decay)
    │
    ▼  Some engines apply: squaring, exp, custom curves
Algorithm input (Hz, dB, ratio, etc.)
```

### 5.6 Parameter Registration: Connecting CCs to Variables

Each Rack sub-engine registers its parameters during `Init()` by calling `registerParamAndCC()`. This creates the lookup table that connects MIDI CCs to parameter storage:

```cpp
// RackDBD::Init() — Digital Bass Drum registers 7 parameters starting at CC offset 8:
initdata->rack->registerParamAndCC(initdata, "f0",     8,  [&](const int val){ f0 = val; });
initdata->rack->registerParamAndCC(initdata, "tone",   9,  [&](const int val){ tone = val; });
initdata->rack->registerParamAndCC(initdata, "decay",  10, [&](const int val){ decay = val; });
initdata->rack->registerParamAndCC(initdata, "dirty",  11, [&](const int val){ dirty = val; });
initdata->rack->registerParamAndCC(initdata, "fm_env", 12, [&](const int val){ fm_env = val; });
initdata->rack->registerParamAndCC(initdata, "fm_dcy", 13, [&](const int val){ fm_dcy = val; });
initdata->rack->registerParamAndCC(initdata, "accent", 14, [&](const int val){ accent = val; });
```

The key is computed as `CC_TO_MAP_KEY(midi_channel, basecc + cc_offset)`, which combines the channel and CC into a unique lookup key. When the macro system sends `handleMidiControlChange(ch=9, cc=8, value=68)` for the Kick track, it resolves to `RackDBD::f0 = 68 * 4096 / 128 = 2180`.

### 5.7 Per-Machine Parameter Tables

Below are the complete parameter registrations for each machine, showing the CC offset, parameter name, and how the DSP float is scaled. This is the authoritative reference for what each CC actually controls.

#### `db` — Digital Bass Drum (RackDBD)

| CC Offset | Name | DSP Scaling | Physical Range |
|-----------|------|-------------|----------------|
| 8 | f0 | `MIN_MAX(0.0005, 0.01)` | Normalized frequency |
| 9 | tone | `ABS(1.0)` | 0.0–1.0 |
| 10 | decay | `ABS(1.0)` | 0.0–1.0 |
| 11 | dirty | `ABS(5.0)` | 0.0–5.0 |
| 12 | fm_env | `ABS(5.0)` | 0.0–5.0 |
| 13 | fm_dcy | `ABS(4.0)` | 0.0–4.0 |
| 14 | accent | `ABS(1.0)` | 0.0–1.0 |

#### `ds` — Digital Snare Drum (RackDSD)

| CC Offset | Name | DSP Scaling | Physical Range |
|-----------|------|-------------|----------------|
| 8 | f0 | `MIN_MAX(0.0008, 0.01)` | Normalized frequency |
| 9 | decay | `ABS(1.0)` | 0.0–1.0 |
| 10 | fm_amt | `ABS(1.5)` | 0.0–1.5 |
| 11 | spy | `ABS(1.0)` | 0.0–1.0 (snappiness) |
| 12 | accent | `ABS(1.0)` | 0.0–1.0 |

#### `hh1` — Hi-Hat 1 (RackHH1)

| CC Offset | Name | DSP Scaling | Physical Range |
|-----------|------|-------------|----------------|
| 8 | f0 | `MIN_MAX(0.0005, 0.1)` | Normalized frequency |
| 9 | tone | `ABS(1.0)` | 0.0–1.0 |
| 10 | decay | `ABS(1.0)` | 0.0–1.0 |
| 11 | noise | `ABS(1.0)` | 0.0–1.0 |
| 12 | accent | `ABS(1.0)` | 0.0–1.0 |

#### `td3` — TBD03 Acid Bass (RackTBD03)

| CC Offset | Name | DSP Scaling | Physical Range |
|-----------|------|-------------|----------------|
| 8 | shape | `× 47 / 4096` (integer) | Oscillator shape index 0–47 |
| 9 | param_0 | `× 32768 / 4096` | Braids timbre 0–32767 |
| 10 | decay_vca | `/ 4095 × 5` | VCA decay time 0–5 sec |
| 11 | decay_vcf | `/ 4095 × 5` | VCF decay time 0–5 sec |
| 12 | cutoff | `/ 4095 × 27000 − 5000` (clamped 20–22000) | Filter cutoff in Hz |
| 13 | resonance | `/ 4095` | 0.0–1.0 |
| 14 | envelope | `/ 4095` | Filter envelope amount 0.0–1.0 |
| 15 | filter_type | `× 4 / 4096` (integer) | Filter type selector 0–4 |
| 16 | saturation | `× 16` (clamped 0–65535) | Waveshaper signature |
| 17 | drive | `/ 4095 × 30` (clamped 1–30) | Filter drive |
| 18 | slide | Boolean | Slide on/off |
| 19 | accent | Boolean-ish | Accent flag |
| 20 | param_1 | `× 32768 / 4096` | Braids color 0–32767 |
| 21 | p0_amt | Direct int | Timbre modulation amount |
| 22 | p1_amt | Direct int | Color modulation amount |
| 23 | accent_level | `/ 4095` | Accent VCF contribution 0.0–1.0 |

#### `ro` — Rompler (RackRompler)

| CC Offset | Name | DSP Scaling | Physical Range |
|-----------|------|-------------|----------------|
| 8 | bank | `INT(128)` | Sample bank 0–31 |
| 9 | slice | `INT(128)` | Slice within bank 0–31 |
| 10 | start | `ABS(1.0)` | Playback start 0.0–1.0 |
| 11 | end | `ABS(1.0)` | Playback length 0.0–1.0 |
| 12 | fc | Filter cutoff | Varies |
| 13 | fq | Filter resonance | Varies |
| 14 | ft | Filter type | Varies |
| 15 | brr | Bit-rate reduction | Varies |
| 16 | atk | `ABS(2.0)` | Attack time 0–2 sec |
| 17 | dcy | `ABS(50.0)` | Decay time 0–50 sec |
| 18 | speed | `ABS(2.0)` (clamped 0–2) | Playback speed |
| 19 | pitch | `ABS(128)` | Pitch in MIDI note units |
| 20 | lp | Boolean | Loop on/off |
| 21 | lp_pp | Boolean | Loop ping-pong |
| 22 | lp_pos | `ABS(1.0)` | Loop marker 0.0–1.0 |
| 23 | eg2fm | `ABS(12.0)` | EG to FM amount |

#### Channel Mixer (per track, CC offsets 1–5)

| CC Offset | Name | DSP Scaling | Physical Range | Notes |
|-----------|------|-------------|----------------|-------|
| 1 | lev | `ABS(2.0)` then **squared** | 0.0–4.0 | Perceptual loudness curve |
| 2 | pan | `ABS(1.0)` then `×2 − 1` | −1.0 to +1.0 | Left-right pan |
| 3 | fx1 | `ABS(2.0)` then **squared** | 0.0–4.0 | Delay send |
| 4 | fx2 | `ABS(1.5)` then **squared** | 0.0–2.25 | Reverb send |
| 5 | tracklength | `ABS(128)` | Sequencer track length | Integer 0–128 |

### 5.8 Implications for Macro Definitions

When writing a macro definition, the `min`, `max`, and `res` fields in the `groups[].parameters[]` refer to the **virtual parameter value space (0–127)**. They have no direct relationship to the DSP float range — the DSP scaling is entirely hardcoded in the Rack engine's C++ `Process()` function.

This means:
- **You cannot change the physical range of a DSP parameter from a macro definition.** For example, the Digital Bass Drum's frequency is always mapped to 0.0005–0.01 regardless of what `min`/`max` you set in the macro JSON.
- **What you can control** is which *portion* of the 0–127 CC range your virtual knob covers, and how multiple virtual knobs combine into a CC value via the mapping formula.
- **There is no way to add logarithmic/exponential curves** from the macro layer. If a parameter needs a log curve, it must be implemented in the Rack engine's C++ code (either in the MK_ macro or the algorithm itself).

---

## 6. End-to-End Data Flow

### Loading a Sound Preset

```
1. User selects a sound preset (e.g. "Snappy") in the WebUI
2. WebUI loads the sound preset JSON → reads its "macro" field → loads that macro definition
3. WebUI sends POST /api/v1/macroapi?action=update_track with:
   { "track": 2, "machine": "ds", "macro": "ds-snappy", "parameters": [68, 16, 16] }
4. MacroTranslator receives this:
   a. Sets the track's machine to "ds" → calls PicoSeqRack::setTrackMachine(2, "ds")
      → enables the Digital Snare engine on track 2
   b. Loads the macro definition "ds-snappy" from SD card
   c. Stores parameter values [68, 16, 16] → marks track 2 as dirty
5. On next audio Process() call, MacroTranslator::TranslateInput() runs:
   a. Finds track 2 is dirty
   b. Evaluates all 5 mapping entries using the formula
   c. For each result, calls handleMidiControlChange(channel, basecc + ctrl, value)
6. PicoSeqRack looks up the CC in its pMapParCC table → scales 0–127 to 0–4095
   → writes to the DSP engine's parameter variable
7. Audio renders with the new parameter values
```

### Editing Parameters Live

```
1. User adjusts a slider in the WebUI "Sound preset preview" panel
2. If "Send on change" is enabled, WebUI immediately sends the new values array
   to POST /api/v1/macroapi?action=update_track
3. MacroTranslator updates the stored values and marks track dirty
4. On next Process() call, all mapping outputs are re-evaluated
5. Sound changes in real time
```

---

## 7. SD Card File Layout

```
/sdcard/data/
├── synthdefinitions.json              # Machine and track definitions (fixed)
├── macrodefinitions/                  # Macro definition files
│   ├── db-allparams.json             # "All parameters" macro for Digital Kick
│   ├── ds-snappy.json                # Curated 3-knob snare macro
│   ├── td3-allparams.json            # Full parameter macro for TBD03 synth
│   ├── fxdelay-allparams.json        # FX delay all parameters
│   ├── nodrum-allparams.json         # Empty drum (no parameters)
│   └── ...                           # One file per macro definition
└── macrosoundpresets/                 # Sound preset files
    ├── db-all-def.json               # Default preset for db-allparams
    ├── ds-snap1.json                 # "Snappy" snare preset
    ├── td3-all-def.json              # Default TBD03 preset
    └── ...                           # One file per sound preset
```

### Naming Convention

- **Macro definitions:** `<machine>-<variant>.json` (e.g. `db-allparams.json`, `ds-snappy.json`)
- **Sound presets:** `<machine>-<description>.json` (e.g. `ds-snap1.json`, `hh1-all-def.json`)
- The `id` field inside the JSON matches the filename (without `.json`)

---

## 8. WebUI Preset Editor

The preset editor (`www/preseteditor/`) is a TypeScript/Vite web application for creating and managing macro definitions and sound presets. It runs in the browser and communicates with the device over HTTP.

### Editor Layout

The editor is split into two columns:

**Left column — Macro Definition editing:**
- Device connection panel (track selection, browsing existing definitions on device)
- Macro preset configuration (ID, name, target machine)
- Output mapping list — shows the mapping formula for each CC, with an inline editor for configuring sources, multipliers, dividers, and start values
- Import/export buttons (copy JSON, download, paste, upload to device)

**Right column — Sound Preset editing:**
- Sound preset metadata (ID, name, group, linked macro ID)
- Per-parameter sliders organized by the macro definition's groups/pages
- "Reset to defaults" and "Randomize" for sound design exploration
- Live parameter sending to device with "Send on change" toggle

### Key Features

- **Drag-and-drop file upload:** Drop JSON files onto the device panel to batch-upload macro definitions, sound presets, or a new `synthdefinitions.json`
- **Auto-detection:** Dropped files are automatically classified as macro definition, sound preset, or synth definition based on their structure
- **Track-aware filtering:** When a track is selected, only macro definitions and sound presets for machines valid on that track are shown in the dropdowns
- **Live preview:** Parameter changes can be sent to the device in real time
- **1:1 mapping generator:** A button to auto-create a one-to-one passthrough mapping for all machine parameters
- **Machine reload:** A button to trigger configuration reload on the device

### Development

```bash
cd www/preseteditor
npm install
npm run dev        # Dev server at http://localhost:5173/preseteditor.html
npm run bundle     # Build and copy to sdcard_image/www/
```

---

## 9. C++ Implementation Classes

### Core Classes (in `main/`)

| Class | File | Purpose |
|-------|------|---------|
| `MacroTranslator` | `MacroTranslator.cpp/hpp` | Central engine: stores per-track parameter values, evaluates mapping formulas, sends CC values to the sound processor |
| `MacroDeviceDefinition` | `MacroDeviceDefinition.cpp/hpp` | In-memory model for a macro definition (groups, parameters, mappings) |
| `MacroDeviceDefinitionDataModel` | `MacroDeviceDefinitionDataModel.cpp/hpp` | SD card I/O: load/save macro definitions from `/sdcard/data/macrodefinitions/` |
| `MacroSoundPreset` | `MacroSoundPreset.cpp/hpp` | In-memory model for a sound preset |
| `MacroSoundPresetDataModel` | `MacroSoundPresetDataModel.cpp/hpp` | SD card I/O: load/save sound presets from `/sdcard/data/macrosoundpresets/` |
| `SynthDefinition` | `SynthDefinition.cpp/hpp` | In-memory model for machine definitions |
| `SynthDefinitionDataModel` | `SynthDefinitionDataModel.cpp/hpp` | Loads `synthdefinitions.json` |
| `MacroAPI` | `MacroAPI.cpp/hpp` | HTTP endpoint handler for `/api/v1/macroapi` |

### DSP Engine (in `components/ctagSoundProcessor/`)

| Class | Purpose |
|-------|---------|
| `ctagSoundProcessorPicoSeqRack` | Multi-track DSP engine; hosts sub-engines (Rack* components) per track |
| `RackDBD`, `RackABD`, `RackDSD`, etc. | Individual machine DSP implementations |
| `RackChannelMixer` | Per-channel level/pan/FX send mixing |

---

## 10. REST API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/api/v1/macroapi` | GET | Returns current state of all 16 tracks (machine, macro, parameter values) |
| `/api/v1/macroapi?action=reload` | POST | Reloads all macro/preset definitions from SD card |
| `/api/v1/macroapi?action=update_track` | POST | Updates a track's machine, macro, and/or parameter values |
| `/api/v1/samples` | GET | Lists all files on SD card (used by WebUI for browsing) |
| `/api/v1/samples?getconfig=<path>` | GET | Reads a JSON file from SD card |
| `/api/v1/samples?action=uploadconfig&path=<path>` | POST | Writes a JSON file to SD card |
| `/api/v1/samples?action=manage` | POST | Delete files from SD card |

### `update_track` request body:

```json
{
  "track": 0,
  "machine": "db",
  "macro": "db-allparams",
  "parameters": [50, 70, 30, 0, 20, 80, 0]
}
```

All three fields (`machine`, `macro`, `parameters`) are optional — you can update any subset. When `machine` is changed, the DSP engine for that track is swapped. When `macro` is set, the mapping definition is loaded. When `parameters` are set, the mapping is re-evaluated.

---

## 11. Design Principles

### Separation of Concerns

1. **Machine definitions** describe *what the hardware can do* — fixed, firmware-level
2. **Macro definitions** describe *how parameters are presented* — editable by sound designers
3. **Sound presets** describe *what values to use* — editable by performers

### Minimal, Portable JSON

All three file types use minimal JSON designed to be:
- **Small enough** to store hundreds of files on the ESP32's SD card
- **Simple enough** to parse on embedded C++ without heavy JSON libraries
- **Portable** — can be created/edited in any text editor or the WebUI
- **Self-contained** — each file has an `id` and all data needed to reconstruct the object

### Value Ranges

- Virtual parameter values: **0–127** (MIDI CC range)
- Mapping output values: **0–127** (clamped after formula evaluation)
- Internal DSP parameters: **0–4095** (the engine scales from 0-127 to 0-4095 internally via `value * 4096 / 128`)

### Convention: "All Params" Macros

Each machine has a default `<machine>-allparams.json` macro that exposes every parameter as a 1:1 passthrough. This serves as:
- A starting point for creating custom macros
- A fallback that gives full control
- A reference for the machine's complete parameter set

---

## 12. Future: Adding Curve Support to the Mapping Engine

Currently the mapping formula is strictly linear (`start + Σ(value × mul / div)`). This section describes what would need to change to support logarithmic, exponential, and other curve types — driven from the macro definition JSON, without modifying individual Rack engines.

### 12.1 Overview of Changes

Three layers would need updates:

| Layer | File(s) | Change |
|-------|---------|--------|
| **JSON format** | Macro definition `.json` files | Add optional `"curve"` field to mapping sources |
| **C++ data model** | `MacroDeviceDefinition.hpp/cpp` | Parse and store the curve type per source |
| **C++ mapping engine** | `MacroTranslator.cpp` | Apply curve function during `TranslateInput()` |
| **WebUI** | `www/preseteditor/src/types.ts`, `outputmappinglist.ts` | Edit and display curve type per source |

### 12.2 Step 1: Extend the JSON Format

Add an optional `"curve"` field to each source in the mapping's `"add"` array:

```json
{
  "ctrl": 8,
  "start": 0,
  "add": [
    { "src": 0, "mul": 1, "div": 1, "curve": "log" }
  ]
}
```

Supported curve values:

| Value | Formula | Use case |
|-------|---------|----------|
| `"linear"` (default) | `value` | Most parameters |
| `"log"` | `min × (max/min)^(value/127)` | Frequency, filter cutoff — more resolution at low end |
| `"exp"` | `value²/127` | Amplitude, level — more resolution at high end |
| `"scurve"` | Sigmoid | Crossfade, morph — gentle at extremes, sensitive in middle |

### 12.3 Step 2: Extend `MacroDeviceOutputMappingSource`

In `MacroDeviceDefinition.hpp`, add a curve type enum and field:

```cpp
// Add to MacroDeviceDefinition.hpp

enum class MacroCurveType : uint8_t {
    Linear = 0,   // default: value unchanged
    Log = 1,       // logarithmic: more resolution at low end
    Exp = 2,       // exponential (quadratic): more resolution at high end
    SCurve = 3     // S-curve (sigmoid): gentle at extremes
};

class MacroDeviceOutputMappingSource {
    public:
        uint8_t parameterIndex;
        int32_t multiplier;
        int32_t divider;
        MacroCurveType curve;    // <-- NEW FIELD
    // ...
};
```

### 12.4 Step 3: Parse the New Field

In `MacroDeviceDefinition.cpp`, update `MacroDeviceOutputMappingSource::DeserializeJSON()`:

```cpp
// Add after parsing "div":

curve = MacroCurveType::Linear;  // default
if (jsonelement.HasMember("curve") && jsonelement["curve"].IsString()) {
    std::string curveStr = jsonelement["curve"].GetString();
    if (curveStr == "log") curve = MacroCurveType::Log;
    else if (curveStr == "exp") curve = MacroCurveType::Exp;
    else if (curveStr == "scurve") curve = MacroCurveType::SCurve;
}
```

### 12.5 Step 4: Apply Curves in `MacroTranslator::TranslateInput()`

This is the core change. In `MacroTranslator.cpp`, add a curve application function and use it in the mapping loop:

```cpp
// New helper function:
static int32_t applyCurve(int32_t value, MacroCurveType curve) {
    // value is in 0–127 range
    switch (curve) {
        case MacroCurveType::Log: {
            // Attempt integer-friendly log approximation
            // maps 0–127 → 0–127 with log distribution
            // using piecewise linear approximation (no float math):
            //   0-16  → 0-64   (low range gets more resolution)
            //   16-64 → 64-100
            //   64-127 → 100-127
            if (value <= 16)      return value * 4;
            else if (value <= 64) return 64 + (value - 16);
            else                  return 100 + (value - 64) * 27 / 63;
        }
        case MacroCurveType::Exp: {
            // Quadratic: value² / 127
            return (value * value) / 127;
        }
        case MacroCurveType::SCurve: {
            // Piecewise S-curve approximation:
            //   gentle at extremes, steep in middle
            int32_t centered = value - 64;
            int32_t shaped = (centered * centered * centered) / (64 * 64);
            return 64 + shaped;
        }
        case MacroCurveType::Linear:
        default:
            return value;
    }
}

// Then modify the mapping loop in TranslateInput():
for (auto src : om.sources) {
    int val = trackParameterValues[t][src.parameterIndex];
    val = applyCurve(val, src.curve);    // <-- NEW LINE
    if (src.divider > 0) {
        finalvalue += (val * src.multiplier) / src.divider;
    } else {
        finalvalue += val * src.multiplier;
    }
}
```

> **Note:** The piecewise approximations above avoid floating-point math (`powf`, `logf`) in the audio processing path, which is important on the ESP32-P4. For higher quality, floating-point versions could be used since `TranslateInput()` only runs when a track is dirty (not every audio frame).

### 12.6 Step 5: Update the WebUI

In `www/preseteditor/src/types.ts`, add the curve field:

```typescript
export interface ISerializedMacroPresetOutputMappingSource {
  src: number;
  mul: number;
  div: number;
  curve?: string;  // "linear" | "log" | "exp" | "scurve"
}
```

In `outputmappinglist.ts`, add a dropdown for each source to select the curve type alongside the existing mul/div inputs.

### 12.7 Backward Compatibility

- The `"curve"` field is **optional** — if absent, defaults to `"linear"`, so all existing macro definitions continue to work unchanged.
- The C++ parser defaults to `MacroCurveType::Linear` if the field is missing.
- Old firmware that doesn't know about curves will simply ignore the unknown JSON field.

### 12.8 Example: Logarithmic Frequency Knob

With curve support, the frequency macro could look like:

```json
{
  "id": "db-logfreq",
  "name": "Kick with log freq",
  "machine": "db",
  "groups": [
    {
      "name": "Main",
      "parameters": [
        { "idx": 0, "name": "Freq", "def": 30, "min": 0, "max": 100, "res": 50, "ui": "freq" }
      ]
    }
  ],
  "mapping": [
    { "ctrl": 8, "start": 0, "add": [{ "src": 0, "mul": 1, "div": 1, "curve": "log" }] }
  ]
}
```

The knob sweeps 0–100, but with logarithmic distribution: the first half of the knob throw covers the low-frequency range in detail, the upper half sweeps quickly through the highs — exactly how musicians expect a frequency knob to feel.

---

## 13. Workflows by Persona

### 13.1 Sound Designer Workflow

The Sound Designer creates macro definitions and factory presets. They understand DSP parameters and want to craft polished, performable instruments from the raw machine parameters.

#### Task: Create a "Punchy 808 Kick" macro and presets for the Digital Bass Drum

**Step 1: Start from the "all params" template**

Open the Preset Editor WebUI. Connect to the device. Select Track 1 (Kick). Load the existing `db-allparams` macro from the device — this gives a 1:1 passthrough of all 7 raw parameters.

**Step 2: Reduce to the essential knobs**

The raw Digital Bass Drum has 7 parameters (f0, tone, decay, dirty, fm_env, fm_dcy, accent). For a punchy 808-style kick, the performer only needs 3 knobs: **Punch** (attack character), **Body** (fundamental tone), and **Decay** (tail length). The rest should be locked to specific values.

Edit the macro definition in the WebUI:

- Change the ID to `db-808punch`
- Change the name to `808 Punchy Kick`
- Set target machine to `db`
- Set up one parameter group with 3 parameters:

```json
{
  "id": "db-808punch",
  "name": "808 Punchy Kick",
  "machine": "db",
  "groups": [
    {
      "name": "Main",
      "parameters": [
        { "idx": 0, "name": "Punch",  "def": 40, "min": 0,  "max": 100, "res": 50, "ui": "bignum" },
        { "idx": 1, "name": "Body",   "def": 60, "min": 10, "max": 90,  "res": 40, "ui": "bignum" },
        { "idx": 2, "name": "Decay",  "def": 50, "min": 0,  "max": 127, "res": 64, "ui": "bignum" }
      ]
    }
  ],
  "mapping": [
    { "ctrl": 8,  "start": 5,  "add": [{ "src": 1, "mul": 1, "div": 3 }] },
    { "ctrl": 9,  "start": 0,  "add": [{ "src": 1, "mul": 1, "div": 2 }] },
    { "ctrl": 10, "start": 0,  "add": [{ "src": 2, "mul": 1, "div": 1 }] },
    { "ctrl": 11, "start": 10 },
    { "ctrl": 12, "start": 80, "add": [{ "src": 0, "mul": 1, "div": 2 }] },
    { "ctrl": 13, "start": 20, "add": [{ "src": 0, "mul": 1, "div": 4 }] },
    { "ctrl": 14, "start": 0,  "add": [{ "src": 0, "mul": 1, "div": 5 }] }
  ]
}
```

What's happening in this mapping:
- **Punch** (idx 0) drives fm_env, fm_dcy, and accent simultaneously — one knob controls the attack character
- **Body** (idx 1) drives both f0 and tone together — one knob shapes the fundamental
- **Decay** (idx 2) maps 1:1 to the raw decay parameter
- **dirty** (ctrl 11) is locked at 10 — fixed subtle distortion
- **fm_env** (ctrl 12) has a constant start of 80 plus punch contribution — always attack-heavy

**Step 3: Test with the live preview**

Enable "Send on change" in the right panel. Sweep each knob and listen. Adjust the `start` values and `mul`/`div` ratios until it feels right.

**Step 4: Craft factory presets**

Once the macro feels good, create preset snapshots for common sounds:

```json
{"id": "db-808-deep",   "name": "Deep 808",    "group": "808 Kicks", "macro": "db-808punch", "values": [20, 30, 70]}
{"id": "db-808-tight",  "name": "Tight 808",   "group": "808 Kicks", "macro": "db-808punch", "values": [60, 50, 25]}
{"id": "db-808-boomy",  "name": "Boomy 808",   "group": "808 Kicks", "macro": "db-808punch", "values": [10, 80, 110]}
{"id": "db-808-punchy", "name": "Punchy 808",  "group": "808 Kicks", "macro": "db-808punch", "values": [90, 45, 40]}
```

**Step 5: Upload to device**

Click "Update on device" for the macro definition. Then for each preset, set the values, fill in the ID/name/group, and click "Update on device" for each sound preset. Alternatively, save all files as `.json` and drag-and-drop them all onto the device panel in one batch.

**Step 6: Verify**

Select each preset from the dropdown. Confirm the sound is correct. The performer now has a 3-knob "808 Punchy Kick" instrument with 4 factory presets ready to use.

---

#### Sound Designer Checklist

- [ ] Start from `<machine>-allparams.json` or an empty macro
- [ ] Identify which DSP parameters the performer actually needs
- [ ] Group related parameters into pages (max 4 per page, max 6 pages)
- [ ] Design mappings: use many-to-one for "meta knobs", constants for locked values
- [ ] Test with live preview, adjust start/mul/div until it feels right
- [ ] Create 3–8 factory presets covering common use cases
- [ ] Name presets clearly and group them logically
- [ ] Upload everything to the device and verify

---

### 13.2 Performer / Artist Workflow

The Performer doesn't need to understand DSP parameters, CC numbers, or mapping formulas. They work with ready-made macros and presets created by Sound Designers.

#### Task: Set up a custom drum kit for a live set

**Step 1: Browse available sounds**

Open the Preset Editor WebUI. Connect to the device.

Select **Track 1 (Kick)** from the track dropdown. The "Sound presets on device for track" dropdown shows only presets compatible with this track — filtered by which machines Track 1 supports (db, ab, extdrum).

Browse the preset list:
```
808 Kicks /// Deep 808 (db-808-deep)
808 Kicks /// Tight 808 (db-808-tight)
808 Kicks /// Boomy 808 (db-808-boomy)
808 Kicks /// Punchy 808 (db-808-punchy)
Synth Kick /// Synth Kick (db-all-def)
Analog Kick /// Analog kick 2 (msp-kick2)
```

**Step 2: Load a sound**

Click "Load sound and macro" to load "Tight 808". This does three things automatically:
1. Switches Track 1's machine to `db` (Digital Bass Drum)
2. Loads the `db-808punch` macro definition (which the preset references)
3. Sends the preset's parameter values `[60, 50, 25]` to the device

The kick is now playing "Tight 808" when triggered.

**Step 3: Tweak to taste**

The right panel shows three sliders: **Punch**, **Body**, **Decay** — the simple interface the Sound Designer created. Enable "Send on change" and adjust:

- Drag **Decay** up a bit for a longer tail
- Drag **Punch** down to soften the attack

The performer doesn't know (or need to know) that "Punch" is simultaneously controlling fm_env, fm_dcy, and accent at different ratios.

**Step 4: Save as a personal preset**

Happy with the tweaked sound? Give it a new ID and name:
- ID: `db-my-kick`
- Name: `My Live Kick`
- Group: `My Kit`

Click "Update on device" to save it as a new sound preset. It now appears in the preset list alongside the factory presets.

**Step 5: Set up remaining tracks**

Repeat for other tracks:
- **Track 3 (Snare):** Load "Snappy" preset → tweak → save as "My Snare"
- **Track 4 (Hi-Hat):** Load "Hihat 1" preset → tweak → save as "My Hat"
- **Track 9 (Bass):** Load "All params" TBD03 preset → tweak filter/resonance → save as "My Bass"

**Step 6: Perform**

During the live set, the performer can:
- Switch between saved presets per track instantly
- Tweak the 3–4 macro knobs for real-time sound manipulation
- All changes happen in real time with the "Send on change" toggle enabled

---

#### Performer Quick Reference

| I want to... | Do this |
|--------------|---------|
| **Browse sounds** | Select a track → browse the "Sound presets" dropdown |
| **Load a sound** | Select a preset → click "Load sound and macro" |
| **Tweak a sound** | Enable "Send on change" → adjust sliders |
| **Save my version** | Change the preset ID/name → click "Update on device" |
| **Reset to the original** | Click "Reset to macro defaults" |
| **Try random sounds** | Click "Randomize values" |
| **Switch sounds live** | Select a different preset → click "Load sound and macro" |

---

### 13.3 Comparison of Personas

| Aspect | Sound Designer | Performer |
|--------|---------------|-----------|
| **Edits** | Macro definitions + sound presets | Sound presets only |
| **Understands** | DSP parameters, CC mappings, mul/div | Knob names, preset categories |
| **Uses** | Both columns of the editor | Right column (preview) mainly |
| **Creates** | Instrument "skins" + factory presets | Personal tweaked presets |
| **JSON knowledge** | May edit JSON directly | Never sees JSON |
| **Typical session** | 30-60 min crafting one macro + presets | 5-10 min browsing and tweaking |

---

## 14. Relationship to Existing Systems

The macro/preset system runs **alongside** the existing `mui-*.jsn` / `mp-*.jsn` patch system used by other CTAG TBD sound processors. For PicoSeqRack specifically:

- The `mui-PicoSeqRack.jsn` still defines the raw DSP parameter UI for the standard CTAG web interface
- The macro system adds a **second, higher-level** interface specifically designed for live performance and sound design
- Both systems ultimately write to the same DSP parameter variables in PicoSeqRack

---

## 15. Glossary

| Term | Definition |
|------|------------|
| **Machine** | A DSP engine type (e.g. `db` = Digital Bass Drum, `td3` = TBD03 Acid Bass). Defined in `synthdefinitions.json`. |
| **Track** | One of 16 audio slots in PicoSeqRack. Each track hosts one machine and has a MIDI channel + CC base. |
| **Macro Definition** | A JSON file that defines virtual parameters and their mapping rules to a machine's CCs. |
| **Sound Preset** | A JSON file that stores specific values for a macro definition's virtual parameters. |
| **Virtual Parameter** | A user-facing knob defined in a macro definition. Can map to one or many raw CCs via the mapping formula. |
| **Output Mapping** | A rule that computes a raw CC value from one or more virtual parameters using `start + Σ(src × mul / div)`. |
| **Group / Page** | A UI grouping of up to 4 virtual parameters within a macro definition. Up to 6 pages per macro. |
| **CC Offset (`ctrl`)** | The MIDI CC number relative to the track's `basecc`. The actual CC sent is `basecc + ctrl`. |
| **MacroTranslator** | The C++ component that evaluates mapping formulas and forwards results to the DSP engine. |
