#!/usr/bin/env python3
"""
test_dsp_json_alignment.py — Verify alignment between C++ DSP parameter
registrations and JSON synth/macro definitions.

Detects:
  1. Parameter ORDER mismatches (C++ offset vs JSON ctrl at same position)
  2. Missing parameters (in C++ but not JSON, or vice versa)
  3. Duplicate ctrl/offset values
  4. Macro definition references to non-existent ctrl numbers
  5. Mix page completeness across all macro definitions
  6. Semantic swap detection (name-similarity heuristics)

Usage:
  python3 tests/test_dsp_json_alignment.py

  Returns exit code 0 if all checks pass, 1 if any fail.
  Use in CI or pre-commit hooks.
"""

import json
import glob
import os
import re
import sys
from collections import defaultdict

# ─── Configuration ───────────────────────────────────────────────────

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RACK_CPP_DIR = os.path.join(REPO_ROOT, 'components', 'ctagSoundProcessor', 'rack')
SYNTHDEF_PATH = os.path.join(REPO_ROOT, 'sdcard_image', 'data', 'synthdefinitions.json')
MACRODEF_DIR = os.path.join(REPO_ROOT, 'sdcard_image', 'data', 'macrodefinitions')
PICOSEQUENCER_PATH = os.path.join(REPO_ROOT, 'components', 'ctagSoundProcessor',
                                   'ctagSoundProcessorPicoSeqRack.cpp')

# Machine ID → Rack C++ file mapping
# Derived from setTrackMachine() in ctagSoundProcessorPicoSeqRack.cpp
MACHINE_TO_RACK = {
    'db':       'RackDBD.cpp',
    'ab':       'RackABD.cpp',
    'fmb':      'RackFMB.cpp',
    'ds':       'RackDSD.cpp',
    'as':       'RackASD.cpp',
    'hh1':      'RackHH1.cpp',
    'hh2':      'RackHH2.cpp',
    'rs':       'RackRimshot.cpp',
    'cl':       'RackClap.cpp',
    'ro':       'RackRompler.cpp',
    'td3':      'RackTBD03.cpp',
    'mo':       'RackMO.cpp',
    'wtosc':    'RackWTOsc.cpp',
    'pp':       'RackPolyPad.cpp',
}

# Machines without Rack C++ files (FX buses, empty, external, input)
SKIP_CPP_CHECK = {'nodrum', 'nosynth', 'nofx', 'fxdelay', 'fxreverb',
                   'fxmaster', 'extsynth', 'extdrum', 'in', 'inp'}

# Machines that should NOT have Mix page in macro definitions
NO_MIX_MACHINES = {'nodrum', 'nosynth', 'nofx', 'fxdelay', 'fxreverb',
                    'fxmaster', 'in', 'inp'}

# Mixer CC offsets (shared across all channels, from RackChannelMixer.cpp)
MIXER_CTRLS = {1, 2, 3, 4}

# ─── Helpers ─────────────────────────────────────────────────────────

class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.warnings = 0
        self.errors = []
        self.warning_msgs = []

    def ok(self, msg):
        self.passed += 1
        print(f'  ✓ {msg}')

    def fail(self, msg):
        self.failed += 1
        self.errors.append(msg)
        print(f'  ✗ {msg}')

    def warn(self, msg):
        self.warnings += 1
        self.warning_msgs.append(msg)
        print(f'  ⚠ {msg}')

    def summary(self):
        total = self.passed + self.failed
        status = 'PASS' if self.failed == 0 else 'FAIL'
        print(f'\n{"═" * 60}')
        print(f'  {status}: {self.passed}/{total} checks passed, '
              f'{self.warnings} warnings')
        if self.errors:
            print(f'\n  Errors:')
            for e in self.errors:
                print(f'    • {e}')
        if self.warning_msgs:
            print(f'\n  Warnings:')
            for w in self.warning_msgs:
                print(f'    • {w}')
        print(f'{"═" * 60}')
        return self.failed == 0


def extract_register_calls(cpp_path):
    """
    Parse registerParamAndCC calls from a Rack*.cpp file.
    Returns list of (param_name, offset) tuples in declaration order.
    """
    results = []
    if not os.path.exists(cpp_path):
        return results

    with open(cpp_path) as f:
        content = f.read()

    # Remove block comments
    content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)
    # Remove line comments (but keep the line for position tracking)
    lines = content.split('\n')
    active_lines = []
    for line in lines:
        # Remove // comments
        idx = line.find('//')
        if idx >= 0:
            line = line[:idx]
        active_lines.append(line)
    content = '\n'.join(active_lines)

    # Match: registerParamAndCC(initdata, "paramName", offset, ...)
    pattern = r'registerParamAndCC\s*\(\s*initdata\s*,\s*"([^"]+)"\s*,\s*(\d+)'
    for match in re.finditer(pattern, content):
        param_name = match.group(1)
        offset = int(match.group(2))
        results.append((param_name, offset))

    return results


def load_synth_definitions(path):
    """Load synthdefinitions.json and return a dict of machine_id → params list."""
    with open(path) as f:
        sd = json.load(f)

    machines = {}
    for m in sd.get('machines', []):
        mid = m['id']
        # Separate mixer params from machine params
        machine_params = []
        mixer_params = []
        for p in m.get('parameters', []):
            if p['ctrl'] in MIXER_CTRLS:
                mixer_params.append(p)
            else:
                machine_params.append(p)
        machines[mid] = {
            'params': machine_params,
            'mixer': mixer_params,
            'raw': m
        }

    return machines, sd


def load_macro_definitions(macro_dir):
    """Load all macro definition JSON files."""
    macros = {}
    for fpath in sorted(glob.glob(os.path.join(macro_dir, '*.json'))):
        basename = os.path.basename(fpath)
        with open(fpath) as f:
            macros[basename] = json.load(f)
    return macros


def normalize_name(name):
    """Normalize a parameter name for fuzzy comparison."""
    return re.sub(r'[_\-\s]', '', name.lower())


def names_similar(a, b):
    """Check if two names are similar enough to be the 'same' parameter."""
    na = normalize_name(a)
    nb = normalize_name(b)
    if na == nb:
        return True
    # Check if one contains the other
    if na in nb or nb in na:
        return True
    return False


# ─── Test 1: C++ ↔ JSON Parameter Offset Alignment ──────────────────

def test_offset_alignment(result, synth_machines):
    """
    Verify that C++ registerParamAndCC offsets match JSON ctrl values
    at corresponding positions. Detects parameter ordering swaps.
    """
    print('\n── Test 1: C++ ↔ JSON Parameter Offset Alignment ──')

    for machine_id, rack_file in sorted(MACHINE_TO_RACK.items()):
        cpp_path = os.path.join(RACK_CPP_DIR, rack_file)
        cpp_params = extract_register_calls(cpp_path)

        if not cpp_params:
            result.warn(f'{machine_id}: No registerParamAndCC calls found in {rack_file}')
            continue

        json_machine = synth_machines.get(machine_id)
        if not json_machine:
            result.fail(f'{machine_id}: Machine not found in synthdefinitions.json')
            continue

        json_params = json_machine['params']

        # Build offset → name maps
        cpp_by_offset = {offset: name for name, offset in cpp_params}
        json_by_ctrl = {p['ctrl']: p for p in json_params}

        # Check that all C++ offsets have corresponding JSON entries
        all_offsets = sorted(set(list(cpp_by_offset.keys()) + list(json_by_ctrl.keys())))

        machine_ok = True
        for offset in all_offsets:
            cpp_name = cpp_by_offset.get(offset)
            json_param = json_by_ctrl.get(offset)

            if cpp_name and not json_param:
                result.warn(f'{machine_id}: C++ param "{cpp_name}" at offset {offset} '
                           f'has no JSON entry (missing from synthdefinitions.json)')
                continue

            if json_param and not cpp_name:
                result.warn(f'{machine_id}: JSON param "{json_param["name"]}" at ctrl {offset} '
                           f'has no C++ registration (possible dead parameter)')
                continue

        # CRITICAL CHECK: Detect position swaps
        # Compare the ORDER of parameters between C++ and JSON
        cpp_ordered = sorted(cpp_params, key=lambda x: x[1])  # by offset
        json_ordered = sorted(json_params, key=lambda x: x['ctrl'])  # by ctrl

        # For each position, check if the parameter at that position in C++
        # matches what JSON says should be there
        # This catches swaps like db: C++ has tone@9/decay@10 but JSON has Decay@9/Tone@10
        for cpp_item, json_item in zip(cpp_ordered, json_ordered):
            cpp_name, cpp_offset = cpp_item
            json_name = json_item['name']
            json_ctrl = json_item['ctrl']

            if cpp_offset != json_ctrl:
                result.fail(f'{machine_id}: Offset mismatch — C++ "{cpp_name}" at {cpp_offset}, '
                           f'JSON "{json_name}" at ctrl {json_ctrl}')
                machine_ok = False
                continue

            # Check for semantic swaps: if a C++ param name at offset N
            # better matches a DIFFERENT JSON param name
            # E.g., C++ "tone"@9 but JSON "Decay"@9--  and C++ "decay"@10 but JSON "Tone"@10
            if not names_similar(cpp_name, json_item['id']) and not names_similar(cpp_name, json_name):
                # Check if this C++ name matches a DIFFERENT JSON param
                for other_json in json_ordered:
                    if other_json['ctrl'] != json_ctrl and (
                        names_similar(cpp_name, other_json['id']) or
                        names_similar(cpp_name, other_json['name'])
                    ):
                        result.fail(
                            f'{machine_id}: PARAMETER SWAP DETECTED — '
                            f'C++ "{cpp_name}" is at offset {cpp_offset}, '
                            f'but its label "{other_json["name"]}" is at JSON ctrl {other_json["ctrl"]}. '
                            f'JSON ctrl {json_ctrl} is labeled "{json_name}" instead.'
                        )
                        machine_ok = False
                        break

        if machine_ok:
            result.ok(f'{machine_id}: All {len(cpp_params)} C++ params align with JSON '
                     f'({rack_file})')


# ─── Test 2: Duplicate Offsets / Ctrls ───────────────────────────────

def test_duplicates(result, synth_machines):
    """Check for duplicate ctrl values in JSON and duplicate offsets in C++."""
    print('\n── Test 2: Duplicate Offset / Ctrl Detection ──')

    # JSON duplicates
    for machine_id, machine_data in sorted(synth_machines.items()):
        if machine_id in SKIP_CPP_CHECK:
            continue

        ctrl_counts = defaultdict(list)
        for p in machine_data['params']:
            ctrl_counts[p['ctrl']].append(p['name'])

        for ctrl, names in ctrl_counts.items():
            if len(names) > 1:
                result.fail(f'{machine_id}: Duplicate JSON ctrl {ctrl}: {names}')

        if all(len(v) == 1 for v in ctrl_counts.values()):
            result.ok(f'{machine_id}: No duplicate JSON ctrl values')

    # C++ duplicates
    for machine_id, rack_file in sorted(MACHINE_TO_RACK.items()):
        cpp_path = os.path.join(RACK_CPP_DIR, rack_file)
        cpp_params = extract_register_calls(cpp_path)

        offset_counts = defaultdict(list)
        for name, offset in cpp_params:
            offset_counts[offset].append(name)

        for offset, names in offset_counts.items():
            if len(names) > 1:
                result.fail(f'{machine_id}: Duplicate C++ offset {offset}: {names}')


# ─── Test 3: Parameter Count Alignment ───────────────────────────────

def test_param_count(result, synth_machines):
    """Verify C++ and JSON have the same number of parameters per machine."""
    print('\n── Test 3: Parameter Count Alignment ──')

    for machine_id, rack_file in sorted(MACHINE_TO_RACK.items()):
        cpp_path = os.path.join(RACK_CPP_DIR, rack_file)
        cpp_params = extract_register_calls(cpp_path)

        json_machine = synth_machines.get(machine_id)
        if not json_machine:
            continue

        json_count = len(json_machine['params'])
        cpp_count = len(cpp_params)

        if cpp_count == json_count:
            result.ok(f'{machine_id}: {cpp_count} C++ params = {json_count} JSON params')
        elif cpp_count > json_count:
            diff = cpp_count - json_count
            # Find which C++ offsets are missing from JSON
            json_ctrls = {p['ctrl'] for p in json_machine['params']}
            missing = [(n, o) for n, o in cpp_params if o not in json_ctrls]
            missing_str = ', '.join(f'"{n}"@{o}' for n, o in missing)
            result.warn(f'{machine_id}: C++ has {diff} more params than JSON. '
                       f'Missing in JSON: {missing_str}')
        else:
            diff = json_count - cpp_count
            cpp_offsets = {o for _, o in cpp_params}
            extra = [p for p in json_machine['params'] if p['ctrl'] not in cpp_offsets]
            extra_str = ', '.join(f'"{p["name"]}"@{p["ctrl"]}' for p in extra)
            result.warn(f'{machine_id}: JSON has {diff} more params than C++. '
                       f'Extra in JSON: {extra_str}')


# ─── Test 4: Mix Page Completeness ───────────────────────────────────

def test_mix_pages(result, synth_machines, macros):
    """
    Verify:
    - All sound machines have mix params in synthdefinitions.json
    - All macro definitions for sound machines have a Mix page
    - Mix page has correct ctrl mappings (1-4)
    - Mix page has default values
    """
    print('\n── Test 4: Mix Page Completeness ──')

    # Check synthdefinitions.json
    for machine_id, machine_data in sorted(synth_machines.items()):
        if machine_id in NO_MIX_MACHINES:
            if machine_data['mixer']:
                result.warn(f'{machine_id}: Has mixer params but should not '
                           f'(machine is in NO_MIX list)')
            continue

        mixer = machine_data['mixer']
        mixer_ctrls = {p['ctrl'] for p in mixer}
        expected = {1, 2, 3, 4}

        if mixer_ctrls == expected:
            result.ok(f'{machine_id}: Has all 4 mixer params in synthdefinitions.json')
        elif not mixer_ctrls:
            result.fail(f'{machine_id}: Missing ALL mixer params in synthdefinitions.json')
        else:
            missing = expected - mixer_ctrls
            result.fail(f'{machine_id}: Missing mixer params for ctrl {missing}')

    # Check macro definitions
    for filename, macro in sorted(macros.items()):
        machine = macro.get('machine', '?')
        if machine in NO_MIX_MACHINES:
            # Should NOT have Mix page
            groups = macro.get('groups', [])
            has_mix = any(g['name'] == 'Mix' for g in groups if g.get('parameters'))
            if has_mix:
                result.warn(f'{filename}: Has Mix page but machine "{machine}" should not')
            continue

        if machine in SKIP_CPP_CHECK and machine not in MACHINE_TO_RACK:
            continue

        groups = macro.get('groups', [])
        has_mix = any(g['name'] == 'Mix' for g in groups if g.get('parameters'))

        if not has_mix:
            result.fail(f'{filename}: Missing Mix page (machine={machine})')
            continue

        # Verify Mix group has 4 params with correct defaults
        mix_group = next(g for g in groups if g['name'] == 'Mix')
        mix_params = mix_group.get('parameters', [])

        if len(mix_params) != 4:
            result.fail(f'{filename}: Mix page has {len(mix_params)} params, expected 4')
            continue

        # Check that mapping entries exist for ctrl 1-4
        mapping = macro.get('mapping', [])
        mapped_ctrls = {m['ctrl'] for m in mapping}
        mix_mapped = {1, 2, 3, 4} & mapped_ctrls

        if mix_mapped == {1, 2, 3, 4}:
            result.ok(f'{filename}: Mix page complete with all 4 mappings')
        else:
            missing = {1, 2, 3, 4} - mix_mapped
            result.fail(f'{filename}: Mix page missing mappings for ctrl {missing}')

        # Check defaults
        for p in mix_params:
            if 'def' not in p:
                result.warn(f'{filename}: Mix param "{p["name"]}" has no default value')


# ─── Test 5: Macro Mapping Validity ──────────────────────────────────

def test_macro_mappings(result, synth_machines, macros):
    """
    Verify macro definition mappings reference valid ctrl numbers
    that exist in the machine's synthdefinitions.json entry.
    """
    print('\n── Test 5: Macro Mapping Validity ──')

    for filename, macro in sorted(macros.items()):
        machine_id = macro.get('machine', '?')
        machine_data = synth_machines.get(machine_id)

        if not machine_data:
            # Try without trailing chars (e.g. 'wo' → might be alias)
            result.warn(f'{filename}: Machine "{machine_id}" not found in synthdefinitions.json')
            continue

        valid_ctrls = {p['ctrl'] for p in machine_data['params']}
        valid_ctrls |= MIXER_CTRLS  # Mix params are always valid

        mapping = macro.get('mapping', [])
        invalid_ctrls = []

        for m in mapping:
            ctrl = m.get('ctrl')
            if ctrl is not None and ctrl not in valid_ctrls:
                invalid_ctrls.append(ctrl)

        if not invalid_ctrls:
            result.ok(f'{filename}: All mapping ctrls are valid')
        else:
            result.fail(f'{filename}: Mapping references invalid ctrls: {invalid_ctrls} '
                       f'(not in machine "{machine_id}")')

        # Check that mapping sources (src) reference valid parameter indices
        groups = macro.get('groups', [])
        valid_idxs = set()
        for g in groups:
            for p in g.get('parameters', []):
                valid_idxs.add(p['idx'])

        for m in mapping:
            for add in m.get('add', []):
                src = add.get('src')
                if src is not None and src not in valid_idxs:
                    result.fail(f'{filename}: Mapping ctrl={m["ctrl"]} references '
                               f'src idx {src} which is not a defined parameter')


# ─── Test 6: C++ File Existence ──────────────────────────────────────

def test_cpp_files_exist(result):
    """Verify all referenced Rack C++ files exist."""
    print('\n── Test 6: C++ Source File Existence ──')

    for machine_id, rack_file in sorted(MACHINE_TO_RACK.items()):
        cpp_path = os.path.join(RACK_CPP_DIR, rack_file)
        if os.path.exists(cpp_path):
            result.ok(f'{machine_id}: {rack_file} exists')
        else:
            result.fail(f'{machine_id}: {rack_file} NOT FOUND at {cpp_path}')


# ─── Main ────────────────────────────────────────────────────────────

def main():
    print('╔══════════════════════════════════════════════════════════╗')
    print('║  TBD-16 DSP ↔ JSON Alignment Test Suite                 ║')
    print('╚══════════════════════════════════════════════════════════╝')

    # Verify paths exist
    if not os.path.exists(SYNTHDEF_PATH):
        print(f'\nERROR: synthdefinitions.json not found at {SYNTHDEF_PATH}')
        print('Make sure you run this from the repo root or tests/ directory.')
        sys.exit(2)

    result = TestResult()

    # Load data
    synth_machines, synth_defs = load_synth_definitions(SYNTHDEF_PATH)
    macros = load_macro_definitions(MACRODEF_DIR)

    print(f'\nLoaded: {len(synth_machines)} machines, {len(macros)} macro definitions')
    print(f'Rack C++ dir: {RACK_CPP_DIR}')

    # Run tests
    test_cpp_files_exist(result)
    test_offset_alignment(result, synth_machines)
    test_duplicates(result, synth_machines)
    test_param_count(result, synth_machines)
    test_mix_pages(result, synth_machines, macros)
    test_macro_mappings(result, synth_machines, macros)

    # Summary
    success = result.summary()
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
