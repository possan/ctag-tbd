#!/usr/bin/env python3
"""
add_curves_to_allparams.py
────────────────────────────────────────────────────────────────
Adds DSP-aware response curve fields to all allparams macro definitions.

Curve types:
  linear  — default, 1:1 knob-to-CC mapping
  log     — slow start, fast end: for frequency/cutoff/tone params
  exp     — fast start, slow end: for envelope times, saturation, FM depth
  scurve  — gentle at extremes, steep middle: for modulation amounts, resonance

The curve field is added in TWO places:
  1. mapping.add[].curve  — used by C++ MacroTranslator engine
  2. groups.parameters[].curve — used by WebUI for badge display

Usage:
  python3 add_curves_to_allparams.py          # dry-run: show changes
  python3 add_curves_to_allparams.py --apply  # write changes to files
"""

import json
import os
import sys

MACRO_DIR = os.path.join(os.path.dirname(__file__), '..', 'sdcard_image', 'data', 'macrodefinitions')

# ──────────────────────────────────────────────────────────────
# CURVE ASSIGNMENTS: machine → { param_idx: curve_type }
# Only non-linear curves are listed; unlisted params stay "linear".
# ──────────────────────────────────────────────────────────────

CURVE_MAP = {
    # ── DRUM MACHINES ──

    "ab": {  # Analog Kick
        0: "log",    # Freq — oscillator frequency
        1: "log",    # Tone — filter-like
        2: "exp",    # Decay — envelope time
        3: "exp",    # A FM — FM depth
        4: "exp",    # S FM — FM depth
        # 5: Accent → linear
    },

    "as": {  # Analog Snare
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        # 3: Snap → linear
        # 4: Accent → linear
    },

    "cl": {  # Clap
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        # 3: Scale → linear
        # 4: Trans → linear
    },

    "db": {  # Synth Kick
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        3: "exp",    # Dirt — detail at low levels
        4: "exp",    # Fm Env — FM envelope amount
        5: "exp",    # Fm Decay — envelope time
        # 6: Fm Accent → linear
    },

    "ds": {  # Digital Snare
        0: "log",    # Freq
        1: "exp",    # Decay
        2: "exp",    # FM — FM depth
        # 3: Snap → linear
        # 4: Accent → linear
    },

    "fmb": {  # FM Kick
        0: "exp",    # FM — modulation depth
        1: "exp",    # DB — body decay
        2: "exp",    # FM — second FM param
        3: "exp",    # DM — decay modulation
        4: "exp",    # BM — body modulation
        5: "log",    # AF — attack frequency
        6: "exp",    # DF — FM decay
        # 7: I → linear (index, discrete-ish)
    },

    "hh1": {  # Hihat 1
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        # 3: Noise → linear
        # 4: Accent → linear
    },

    "hh2": {  # Hihat 2
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        # 3: Noise → linear
        # 4: Accent → linear
    },

    "rs": {  # Rimshot
        0: "log",    # Freq
        1: "log",    # Tone
        2: "exp",    # Decay
        # 3: Noise → linear
        # 4: Accent → linear
    },

    # ── SYNTH MACHINES ──

    "mo": {  # Macro Osc
        1: "scurve",  # P0 — morph parameter
        2: "scurve",  # P1 — morph parameter
        3: "scurve",  # Waveshape — morphing
        4: "exp",     # P0 A — modulation amount
        5: "exp",     # P1 A — modulation amount
        6: "exp",     # FM A — FM amount
        8: "exp",     # Attack — envelope time
        9: "exp",     # Decay — envelope time
        11: "exp",    # Decim — decimation
        12: "exp",    # Bit Red — bit reduction
        # 0: Bank → linear (discrete)
        # 7: Q Scale → linear
        # 10: Loop Env → linear (boolean-ish)
    },

    "pp": {  # Polypad
        2: "exp",      # Detune — small values most useful
        3: "log",      # Cutoff — filter cutoff
        4: "scurve",   # Reso — resonance
        7: "exp",      # Attack
        8: "exp",      # Decay
        10: "exp",     # Release
        11: "log",     # L1 Speed — LFO speed
        12: "scurve",  # L1 Amount — mod depth
        13: "log",     # L2 Speed — LFO speed
        14: "scurve",  # L2 Amount — mod depth
        15: "scurve",  # E Filter Amount — mod depth
        # 0: Chord → linear (discrete)
        # 1: Inversion → linear (discrete)
        # 5: Type → linear (discrete)
        # 6: Q Scale → linear
        # 9: Sustain → linear (level)
        # 16: L2 Random → linear
        # 17: Number of Notes → linear (discrete)
    },

    "ro": {  # Rompler
        4: "log",      # Cutoff — filter cutoff
        5: "scurve",   # Reso — resonance
        7: "exp",      # Bit.CR — bit crush
        8: "exp",      # Attack
        9: "exp",      # Decay
        10: "log",     # Speed — playback speed
        11: "scurve",  # Pitch — centered pitch shift
        15: "exp",     # EG2FM — modulation amount
        17: "scurve",  # TSAmt — time stretch amount
        # 0: Bank → linear (discrete)
        # 1: Slice → linear (discrete)
        # 2: Start → linear
        # 3: End → linear
        # 6: Type → linear (discrete)
        # 12: Loop → linear (boolean)
        # 13: PingPong → linear (boolean)
        # 14: PPStart → linear
        # 16: TSMode → linear (discrete)
    },

    "td3": {  # TBD03
        1: "scurve",  # P0 — oscillator morph
        2: "exp",     # VCA D — VCA decay
        3: "exp",     # VCF D — VCF decay
        4: "log",     # Cutoff — THE most important one!
        5: "scurve",  # Reso — resonance
        6: "exp",     # EnvDec — envelope decay
        8: "exp",     # Satur — saturation
        9: "exp",     # Drive
        12: "scurve", # P1 — oscillator morph
        13: "scurve", # P0 Amt — modulation amount
        14: "scurve", # P1 Amt — modulation amount
        # 0: Bank → linear (discrete)
        # 7: Type → linear (discrete)
        # 10: Slide → linear
        # 11: Accent → linear
        # 15: Acc. Lev → linear
    },

    "wtosc": {  # Wavetable Osc
        2: "scurve",   # Tune — centered pitch
        4: "log",      # Cutoff — filter cutoff
        5: "scurve",   # Reso — resonance
        7: "exp",      # Attack
        8: "exp",      # Decay
        10: "exp",     # Release
        12: "exp",     # E2 FM — modulation amount
        13: "exp",     # E2 Filt — filter modulation
        14: "log",     # Speed — LFO speed
        17: "scurve",  # L2 AM — modulation depth
        18: "scurve",  # L2 FM — modulation depth
        19: "scurve",  # L2 Filt — modulation depth
        # 0: Bank → linear (discrete)
        # 1: Wave → linear (selector)
        # 3: Type → linear (discrete)
        # 6: Q Scale → linear
        # 9: Sustain → linear (level)
        # 11: E2 Wave → linear (selector)
        # 15: Sync → linear (boolean)
        # 16: L2 Wave → linear (selector)
    },

    # ── FX MACHINES ──

    "fxdelay": {
        0: "log",      # Time — delay time
        6: "scurve",   # Feedback — careful at extremes
        # 1: Sync → linear (boolean)
        # 2: Freeze → linear (boolean)
        # 3: Tapedig → linear (discrete)
        # 4: Stereo width → linear
        # 5: FX2 Send → linear
        # 7: Base → linear
        # 8: Width 2 → linear
        # 9: Level → linear
    },

    "fxreverb": {
        0: "log",   # Time — reverb time
        1: "log",   # Lowpass — filter
        # 2: Level → linear
    },

    "fxmaster": {
        1: "log",   # Ratio — compression ratio
        2: "exp",   # Attack — envelope time
        3: "exp",   # Release — envelope time
        4: "log",   # LPF — filter cutoff
        # 0: Thresh → linear
        # 5: Gain → linear
        # 6: Mix → linear
        # 7: Dly.Lev → linear
        # 8: Rev.Lev → linear
        # 9: Sum mute → linear (boolean)
        # 10: Sum lev → linear
    },
}


def process_file(filepath, apply=False):
    """Process a single allparams JSON file, adding curve fields."""
    with open(filepath, 'r') as f:
        data = json.load(f)

    machine = data.get('machine', '')
    file_id = data.get('id', '')

    # Only process allparams files
    if 'allparams' not in file_id:
        return None

    if machine not in CURVE_MAP:
        return None  # No curve assignments for this machine

    curves = CURVE_MAP[machine]
    changes = []

    # Build a lookup: param_idx → mapping_index and add_index
    # For allparams, it's always 1:1: each mapping has one source
    src_to_mapping = {}
    for mi, m in enumerate(data.get('mapping', [])):
        for ai, a in enumerate(m.get('add', [])):
            src_to_mapping[a['src']] = (mi, ai)

    for param_idx, curve_type in sorted(curves.items()):
        param_name = "?"

        # 1. Add curve to parameter definition (for WebUI badge)
        for group in data.get('groups', []):
            for p in group.get('parameters', []):
                if p['idx'] == param_idx:
                    param_name = p.get('name', '?')
                    old_curve = p.get('curve')
                    if old_curve != curve_type:
                        p['curve'] = curve_type
                        changes.append(f"  param [{param_idx}] {param_name}: curve → {curve_type}")
                    break

        # 2. Add curve to mapping add entry (for C++ engine)
        if param_idx in src_to_mapping:
            mi, ai = src_to_mapping[param_idx]
            add_entry = data['mapping'][mi]['add'][ai]
            old_curve = add_entry.get('curve')
            if old_curve != curve_type:
                add_entry['curve'] = curve_type
                ctrl = data['mapping'][mi].get('ctrl', '?')
                changes.append(f"  mapping ctrl={ctrl} src={param_idx}: curve → {curve_type}")

    if not changes:
        return None

    if apply:
        with open(filepath, 'w') as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            f.write('\n')

    return changes


def main():
    apply = '--apply' in sys.argv
    mode = "APPLYING" if apply else "DRY-RUN"
    print(f"═══ add_curves_to_allparams.py ({mode}) ═══\n")

    total_changes = 0
    files_changed = 0

    for filename in sorted(os.listdir(MACRO_DIR)):
        if not filename.endswith('.json'):
            continue
        filepath = os.path.join(MACRO_DIR, filename)
        changes = process_file(filepath, apply=apply)
        if changes:
            files_changed += 1
            total_changes += len(changes)
            print(f"📄 {filename}:")
            for c in changes:
                print(c)
            print()

    print(f"────────────────────────────────────")
    print(f"Files modified: {files_changed}")
    print(f"Total curve fields added: {total_changes}")

    if not apply:
        print(f"\nRe-run with --apply to write changes.")


if __name__ == '__main__':
    main()
