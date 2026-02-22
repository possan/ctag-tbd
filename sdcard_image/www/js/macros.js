let ROOT = ''
ROOT = 'http://192.168.4.1/api/v1/picoseq'
// ROOT = '/api/v1/picoseq'

// let synthdefinitionids = [];
// let synthdefinitions = {};
// let trackdefinitions = [];
// let macrodefinitionids = [];
// let macrodefinitions = [];
// let soundpresetids = {};
// let soundpresets = {};

let activetrack = 0;
let activetrackmachines = {};
let activepresetparameters = [];
let activemacrodefinitions = [];
let activesoundpresets = [];
let parameters = [];

let cache = {
    synthdefinitionids: [],
    synthdefinitions: {},
    trackdefinitions: [],
    macrodefinitionids: [],
    macrodefinitions: [],
    soundpresetids: {},
    soundpresets: {},
};

//
// Misc utilities
//

async function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function setSelectValue(sel, value) {
    for(let i=0; i<sel.options.length; i++) {
        if (sel.options[i].value === value) {
            sel.selectedIndex = i;
            return;
        }
    }
    sel.selectedIndex = 0;
}

function removeAllSelectValues(sel) {
    while(sel.options.length > 0) {
        sel.remove(0);
    }
}

function getSelectValue(sel) {
    return sel.options[sel.selectedIndex].value;
}

async function loadCache() {
    try {
        cache = JSON.parse(localStorage.getItem('picoseq_macro_cache') || '{}');
    } catch(e) {
        cache = {};
    }

    if (!cache.synthdefinitionids) {
       cache.synthdefinitionids = [];
    }

    if (!cache.synthdefinitions) {
        cache.synthdefinitions = {};
    }

    if (!cache.macrodefinitionids) {
        cache.macrodefinitionids = [];
    }

    if (!cache.macrodefinitions) {
        cache.macrodefinitions = {};
    }

    if (!cache.soundpresetids) {
        cache.soundpresetids = [];
    }

    if (!cache.soundpresets) {
        cache.soundpresets = {};
    }

    if (!cache.trackdefinitions) {
        cache.trackdefinitions = [];
    }
}

async function saveCache() {
    console.log('Saving cache', cache);
    localStorage.setItem('picoseq_macro_cache', JSON.stringify(cache));
}


//
// API calls to TBD rest endpoints
//

async function fetchSynthDefinitionList() {
    if (ROOT === '') {
        return JSON.parse(`{"machines":["md-analogkick-2","db-1234","db","ab","extdrum","ro"]}`)
    }

    const response = await fetch(`${ROOT}/synthdefinitionlist`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchMacroSoundPresetList() {
    if (ROOT === '') {
        return JSON.parse(`{"presets":["dummypreset-123","dummy123","dummy1234"]}`)
    }

    const response = await fetch(`${ROOT}/soundpresets`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchMacroDefinitionList() {
    if (ROOT === '') {
        return JSON.parse(`{"machines":["md-analogkick-2"]}`)
    }

    const response = await fetch(`${ROOT}/macrodefinitions`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchMacroSoundPreset(id) {
    if (ROOT === '') {
        return JSON.parse(`{
    "id": "dummypreset-123",
    "name": "My sound preset",
    "group": "My group",
    "macro": "db-12345",
    "values": [
        50,
        70,
        -1,
        -1,
        20,
        80
    ]
}`)
    }

    const response = await fetch(`${ROOT}/soundpreset/${id}`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchMacroDefinition(id) {
    if (ROOT === '') {
        return JSON.parse(`{
    "id": "md-analogkick-2",
    "name": "My preset",
    "machine": "db",
    "groups": [
        {
            "name": "Group 1",
            "parameters": [
                {
                    "idx": 0,
                    "name": "Cutoff",
                    "def": 50,
                    "min": 10,
                    "max": 10000,
                    "res": 1000,
                    "ui": "freq"
                },
                {
                    "idx": 1,
                    "name": "Reso",
                    "def": 70,
                    "min": 0,
                    "max": 127,
                    "res": 127,
                    "ui": ""
                },
                {
                    "idx": 2,
                    "name": "Envmod",
                    "def": 30,
                    "min": 0,
                    "max": 127,
                    "res": 127,
                    "ui": ""
                }
            ]
        },
        {
            "name": "Group 2",
            "parameters": [
                {
                    "idx": 4,
                    "name": "Parameter 1",
                    "def": 20,
                    "min": 0,
                    "max": 127,
                    "res": 127,
                    "ui": ""
                },
                {
                    "idx": 5,
                    "name": "Parameter 2",
                    "def": 80,
                    "min": 0,
                    "max": 127,
                    "res": 127,
                    "ui": ""
                }
            ]
        }
    ],
    "mapping": [
        {
            "tgt": "freq",
            "start": 0,
            "add": []
        },
        {
            "tgt": "decay",
            "start": 0,
            "add": []
        },
        {
            "tgt": "tone",
            "start": 0,
            "add": []
        },
        {
            "tgt": "dirt",
            "start": 0,
            "add": []
        },
        {
            "tgt": "fm-env",
            "start": 0,
            "add": []
        },
        {
            "tgt": "fm-decay",
            "start": 0,
            "add": []
        },
        {
            "tgt": "fm-accent",
            "start": 0,
            "add": []
        },
        {
            "tgt": "db-d",
            "start": 10,
            "add": [
                {
                    "src": 0,
                    "amt": 50
                },
                {
                    "src": 2,
                    "amt": 50
                }
            ]
        },
        {
            "tgt": "db-fq",
            "start": 127,
            "add": [
                {
                    "src": 1,
                    "amt": 50
                }
            ]
        }
    ]
}`)
    }

    const response = await fetch(`${ROOT}/macrodefinition/${id}`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchTrackState() {
    if (ROOT === '') {
        return JSON.parse(`{
    "tracks": [
        {
            "index": 0,
            "activeMachineId": "nodrum"
        },
        {
            "index": 1,
            "activeMachineId": "ab"
        },
        {
            "index": 2,
            "activeMachineId": ""
        },
        {
            "index": 3,
            "activeMachineId": "db"
        },
        {
            "index": 4,
            "activeMachineId": ""
        },
        {
            "index": 5,
            "activeMachineId": ""
        },
        {
            "index": 6,
            "activeMachineId": ""
        },
        {
            "index": 7,
            "activeMachineId": ""
        },
        {
            "index": 8,
            "activeMachineId": ""
        },
        {
            "index": 9,
            "activeMachineId": ""
        },
        {
            "index": 10,
            "activeMachineId": ""
        },
        {
            "index": 11,
            "activeMachineId": ""
        },
        {
            "index": 12,
            "activeMachineId": ""
        },
        {
            "index": 13,
            "activeMachineId": ""
        },
        {
            "index": 14,
            "activeMachineId": ""
        },
        {
            "index": 15,
            "activeMachineId": ""
        }
    ]
}`)
    }

    const response = await fetch(`${ROOT}/trackstatus`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchSynthDefinition(id) {
    if (ROOT === '') {
        return JSON.parse(`{
    "id": "ro",
    "name": "Rompler",
    "parameters": [
        {
            "id": "bank",
            "name": "Bank",
            "type": "None",
            "cc": 8,
            "default": 0
        },
        {
            "id": "slice",
            "name": "Slice",
            "type": "None",
            "cc": 9,
            "default": 0
        },
        {
            "id": "start",
            "name": "Start",
            "type": "None",
            "cc": 10,
            "default": 0
        },
        {
            "id": "end",
            "name": "End",
            "type": "None",
            "cc": 11,
            "default": 0
        },
        {
            "id": "cutoff",
            "name": "Cutoff",
            "type": "None",
            "cc": 12,
            "default": 0
        },
        {
            "id": "reso",
            "name": "Reso",
            "type": "None",
            "cc": 13,
            "default": 0
        },
        {
            "id": "type",
            "name": "Type",
            "type": "None",
            "cc": 14,
            "default": 0
        },
        {
            "id": "bitcr",
            "name": "Bit.CR",
            "type": "None",
            "cc": 15,
            "default": 0
        },
        {
            "id": "attack",
            "name": "Attack",
            "type": "None",
            "cc": 16,
            "default": 0
        },
        {
            "id": "decay",
            "name": "Decay",
            "type": "None",
            "cc": 17,
            "default": 0
        },
        {
            "id": "speed",
            "name": "Speed",
            "type": "None",
            "cc": 18,
            "default": 0
        },
        {
            "id": "pitch",
            "name": "Pitch",
            "type": "None",
            "cc": 19,
            "default": 0
        },
        {
            "id": "loop",
            "name": "Loop",
            "type": "None",
            "cc": 20,
            "default": 0
        },
        {
            "id": "pingpong",
            "name": "PingPong",
            "type": "None",
            "cc": 21,
            "default": 0
        },
        {
            "id": "ppstart",
            "name": "PPStart",
            "type": "None",
            "cc": 22,
            "default": 0
        },
        {
            "id": "eg2fm",
            "name": "EG2FM",
            "type": "None",
            "cc": 23,
            "default": 0
        },
        {
            "id": "tsmode",
            "name": "TSMode",
            "type": "None",
            "cc": 24,
            "default": 0
        },
        {
            "id": "tsamt",
            "name": "TSAmt",
            "type": "None",
            "cc": 25,
            "default": 0
        }
    ]
}`)
    }

    const response = await fetch(`${ROOT}/synthdefinition/${id}`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function fetchTrackDefinition(id) {
    if (ROOT === '') {
        return JSON.parse(`{"tracks":[],"index":0,"name":"Kick","machines":["nodrum","db","ab","extdrum"]}`)
    }
    const response = await fetch(`${ROOT}/trackdefinition/${id}`);
    const data = await response.json();
    await delay(100);
    return data;
}

async function putMacroDefinition(id, jsonstring) {
    const response = await fetch(`${ROOT}/macrodefinition/${id}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: jsonstring
    });
    await delay(100);
    return response.ok
}

async function putSoundPreset(id, jsonstring) {
    const response = await fetch(`${ROOT}/soundpreset/${id}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: jsonstring
    });
    await delay(100);
    return response.ok;
}

async function putActiveMachineForTrack(trackindex, machineid) {
    const response = await fetch(`${ROOT}/tracks/${trackindex}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ machine: machineid })
    });
    await delay(100);
    return response.ok
}

async function putActiveMacroDefinitionForTrack(trackindex, macroid) {
    const response = await fetch(`${ROOT}/tracks/${trackindex}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ macro: macroid })
    });
    await delay(100);
    return response.ok
}

async function putParametersForTrack(trackindex, parameters) {
    const response = await fetch(`${ROOT}/tracks/${trackindex}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: JSON.stringify({ parameters: parameters })
    });
    await delay(100);
    return response.ok
}

//
// UI update functions
//

function recreateSynthMachineList() {
    const root = document.getElementById('machines');
    root.innerHTML = ''

    for(const id of cache.synthdefinitionids) {
        const mach = cache.synthdefinitions[id];

        const li = document.createElement('li');
        const title = document.createElement('b');
        if (mach) {
            title.textContent = `#${mach.id} ${mach.name}`;
        } else {
            title.textContent = `#${id}...`;
        }
        li.appendChild(title);
        const label = document.createElement('span');
        if (mach) {
            label.textContent = `parameters: ${mach.parameters.map(p => p.name).join(', ')}`;
        }
        li.appendChild(label);
        root.appendChild(li);
    }
}

function recreateTrackList() {
    const root = document.getElementById('tracks');
    root.innerHTML = ''

    for(let t=0; t<16; t++) {
        let track = cache.trackdefinitions[t];
        let st = activetrackmachines[t];

        const li = document.createElement('li');
        const title = document.createElement('b');

        if (t === activetrack) {
            li.classList.add('active');
        } else {
            li.addEventListener('click', selectTrack.bind(this, t));
        }

        if (track) {
            title.textContent = `#${t + 1} "${track.name}"`;
        } else {
            title.textContent = `#${t + 1}...`;
        }

        if (st) {
            title.textContent += ` (${st})`;
        }


        li.appendChild(title);
        const label = document.createElement('span');
        if (track) {
            label.textContent = ` (machines: ${track.machines && track.machines.join(', ')})`;
        }
        li.appendChild(label);

        root.appendChild(li);
    }
}

function recreateActiveTrackTitle() {
    const trackmeta = cache.trackdefinitions[activetrack];
    document.getElementById('tracktitle').textContent = `Track ${activetrack + 1}: ${trackmeta ? trackmeta.name : '?'}`;
}

function recreateActiveTrackMachineList() {
    const sel = document.getElementById('activetrackmachine')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const trackinfo = cache.trackdefinitions[activetrack];
    console.log('Recreating machine list for track', activetrack, trackinfo)
    if (trackinfo && trackinfo.machines) {
        for(const machid of trackinfo.machines) {
            const mach = cache.synthdefinitions[machid];
            // console.log('Adding machine option', machid, mach)
            if (mach) {
                const name = mach ? mach.name : machid;
                sel.options.add(new Option(name, machid));
            }
        }
    }

    const currentmachine = activetrackmachines[activetrack] || '';
    setSelectValue(sel, currentmachine)
}

function recreateActiveTrackMacroDefinitionList() {
    const sel = document.getElementById('activemacrodefinition')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const currentmachine = activetrackmachines[activetrack] || '';
    const trackinfo = cache.trackdefinitions[activetrack];
    console.log('Recreating macro definition list for track', activetrack, currentmachine, trackinfo)
    for(const machid of cache.macrodefinitionids) {
        const mach = cache.macrodefinitions[machid];
        if (mach) {
            // TODO: filter by machine type
            sel.options.add(new Option(mach.name, machid));
        }
    }

    const currentmacro = activemacrodefinitions[activetrack] || '';
    setSelectValue(sel, currentmacro)
}

function recreateActiveTrackSoundPresetList() {
    const sel = document.getElementById('activesoundpreset')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const currentmachine = activepresetparameters[activetrack] || '';

    for(const presetid of cache.soundpresetids) {
        const preset = cache.soundpresets[presetid];
        if (preset) {
            sel.options.add(new Option(preset.name, presetid));
        }
    }

    const currentmacro = activemacrodefinitions[activetrack];
    setSelectValue(sel, currentmacro)
}

function recreateActiveTrackParameters() {
    const currentmacro = activemacrodefinitions[activetrack];
    const macro = cache.macrodefinitions[currentmacro];
    console.log('Recreating parameters for macro', currentmacro, macro)

    const root = document.getElementById('trackparametergroups');

    root.innerHTML = ''

    if (macro && macro.groups) {
        for(const g of macro.groups) {
            const gel = document.createElement('div');
            gel.classList.add('paramgroup');

            const title = document.createElement('b');
            title.textContent = g.name;
            gel.appendChild(title);

            for(const p of g.parameters) {
                const pel = document.createElement('div');
                pel.classList.add('parameter');

                const label = document.createElement('span');
                label.textContent = p.name;
                pel.appendChild(label);

                // const val = document.createElement('span');
                // val.textContent = ` (def: ${p.def}, min: ${p.min}, max: ${p.max}, res: ${p.res}, ui: ${p.ui})`;
                // pel.appendChild(val);

                const range = document.createElement('input');
                range.type = 'range'
                range.min = p.min;
                range.max = p.max;
                range.value = p.def;
                range.dataset.parameterindex = p.idx;
                range.addEventListener('change', updateParameterValue.bind(this, p.idx))
                pel.appendChild(range);
                // val.textContent = ` (def: ${p.def}, min: ${p.min}, max: ${p.max}, res: ${p.res}, ui: ${p.ui})`;
                // pel.appendChild(val);

                const val2 = document.createElement('span');
                val2.textContent = `${p.def}`;
                val2.dataset.parameterindex = p.idx;
                pel.appendChild(val2);

                gel.appendChild(pel);
            }

            root.appendChild(gel);
        }
    }
}

function updateUiParameters() {
    const currentmacro = activemacrodefinitions[activetrack];
    const macro = cache.macrodefinitions[currentmacro];

    for(const g of macro.groups){
        for(const p of g.parameters) {
            const v = parameters[p.idx];

            const input = document.querySelector(`input[data-parameterindex="${p.idx}"]`);
            if (input) {
                input.value = v;
            }

            const span = document.querySelector(`span[data-parameterindex="${p.idx}"]`);
            if (span) {
                span.textContent = v;
            }
        }
    }
}

function setDefaultParameters() {
    const currentmacro = activemacrodefinitions[activetrack];
    const macro = cache.macrodefinitions[currentmacro];
    parameters = [];
    for(const g of macro.groups){
        for(const p of g.parameters) {
            parameters[p.idx] = p.def;
        }
    }
    updateUiParameters();
    putParametersForTrack(activetrack, parameters);
}

function setRandomParameters() {
    const currentmacro = activemacrodefinitions[activetrack];
    const macro = cache.macrodefinitions[currentmacro];
    parameters = [];
    for(const g of macro.groups){
        for(const p of g.parameters) {
            parameters[p.idx] = Math.floor(Math.random() * (p.max - p.min + 1)) + p.min;
        }
    }
    updateUiParameters();
    putParametersForTrack(activetrack, parameters);
}

function recreateSoundPresetList() {
    const root = document.getElementById('soundpresets');
    root.innerHTML = ''

    for(const id of cache.soundpresetids || []) {
        const sp = cache.soundpresets[id];

        const li = document.createElement('li');
        const title = document.createElement('b');
        if (sp) {
            title.textContent = sp.name;
        } else {
            title.textContent = `#${id}...`;
        }
        li.appendChild(title);
        const label = document.createElement('span');
        if (sp) {
            label.textContent = JSON.stringify(sp);
        }
        // label.textContent = `#${track.index} "${track.name}" (machines: ${track.machines.join(', ')})`;
        li.appendChild(label);
        root.appendChild(li);
    }
}

function recreateMacroDefinitionList() {
    const root = document.getElementById('macrodefinitions');
    root.innerHTML = ''

    for(const id of cache.macrodefinitionids || []) {
        const mach = cache.macrodefinitions[id];

        const li = document.createElement('li');
        const title = document.createElement('b');
        if (mach) {
            title.textContent = mach.name;
        } else {
            title.textContent = `#${id}...`;
        }
        li.appendChild(title);
        const label = document.createElement('span');
        label.textContent = JSON.stringify(mach);
        // label.textContent = `#${track.index} "${track.name}" (machines: ${track.machines.join(', ')})`;
        li.appendChild(label);
        root.appendChild(li);
    }
}

//
// UI event handlers
//

function selectTrack(t) {
    activetrack = t;
    recreateTrackList();
    recreateActiveTrackTitle();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();
}

// async function selectMachineForTrack(t, ev) {
//     const machid = getSelectValue(ev.target)
//     activetrackmachines[t] = machid;
//     console.log(`Selecting machine ${machid} for track ${t}`);
//     await putActiveMachineForTrack(t, machid);
//     recreateTrackList();
// }

async function uploadMacroDefinition() {
    const jsonel = document.getElementById('uploadmacrodefinitionjson');
    const txt = jsonel.value;
    let json = undefined
    try {
        json = JSON.parse(txt);
    } catch (e) {
        alert('Invalid JSON: ' + e.message);
        return;
    }
    if (!json.id) {
        alert('Sound preset JSON must have an "id" field');
        return;
    }
    const packed = JSON.stringify(json);
    console.log('Uploading macro definition:', packed);
    const ok = await putMacroDefinition(json.id, packed)
    if (ok) {
        await updateMacroDefinitionList();
    } else {
        alert('Failed to upload macro definition');
    }
}

async function uploadSoundPreset() {
    const jsonel = document.getElementById('uploadsoundpresetjson');
    const txt = jsonel.value;
    let json = undefined
    try {
        json = JSON.parse(txt);
    } catch (e) {
        alert('Invalid JSON: ' + e.message);
        return;
    }
    if (!json.id) {
        alert('Sound preset JSON must have an "id" field');
        return;
    }
    const packed = JSON.stringify(json);
    console.log('Uploading sound preset:', packed);
    const ok = await putSoundPreset(json.id, packed);
    if (ok) {
        await updateMacroSoundPresetList();
    } else {
        alert('Failed to upload sound preset');
    }
}

async function changeActiveTrackMachine(ev) {
    const machid = getSelectValue(ev.target)
    activetrackmachines[activetrack] = machid;
    console.log(`Selecting machine ${machid} for track ${activetrack}`);
    await putActiveMachineForTrack(activetrack, machid);
    // TODO: select a preset
    recreateTrackList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
}

async function changeActiveTrackMacroDefinition(ev) {
    const machid = getSelectValue(ev.target)
    activemacrodefinitions[activetrack] = machid;
    console.log(`Selecting macro definition ${machid} for track ${activetrack}`);
    await putActiveMacroDefinitionForTrack(activetrack, machid);
    recreateTrackList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();
    setDefaultParameters();
}

async function changeActiveTrackSoundPreset(ev) {
    const machid = getSelectValue(ev.target)
    console.log(`Selecting sound preset ${machid}`);
    // activesoundpresets[t] = machid;
    // await putActiveSoundPresetForTrack(activetrack, machid);
    // recreateTrackList();
    // TODO: load parameters, update parameters in ui
}

async function updateParameterValue(idx, ev) {
    console.log('Updating parameter', idx, 'to value', ev.target.value);

    parameters[idx] = parseInt(ev.target.value);

    const span = document.querySelector(`span[data-parameterindex="${idx}"]`);
    if (span) {
        span.textContent = ev.target.value;
    }

    putParametersForTrack(activetrack, parameters);
}


//
// State updaters
//

async function updateSynthDefinitions() {
    const ret = await fetchSynthDefinitionList();
    cache.synthdefinitionids = ret.machines || [];

    recreateSynthMachineList();
    recreateTrackList();

    cache.synthdefinitions = {};
    for(const k of cache.synthdefinitionids) {
        const def = await fetchSynthDefinition(k);
        if (def) {
            cache.synthdefinitions[k] = def;
        }
    }

    recreateSynthMachineList();

    cache.trackdefinitions = [];
    for(let t=0; t<16; t++) {
        const def = await fetchTrackDefinition(t);
        if (def) {
            cache.trackdefinitions[t] = def;
        }
    }

    recreateTrackList();
}

async function updateMacroSoundPresetList() {
    const ret = await fetchMacroSoundPresetList();

    cache.soundpresetids = ret.presets || [];
    cache.soundpresets = {};

    recreateSoundPresetList();

    for(const k of cache.soundpresetids) {
        const preset = await fetchMacroSoundPreset(k);
        if (preset) {
            cache.soundpresets[k] = preset;
        }
    }

    recreateSoundPresetList();
}


async function updateMacroDefinitionList() {
    const ret = await fetchMacroDefinitionList();
    console.log(ret)

    cache.macrodefinitionids = ret.machines || [];
    cache.macrodefinitions = {};

    recreateMacroDefinitionList();

    for(const k of cache.macrodefinitionids) {
        const def = await fetchMacroDefinition(k);
        if (def) {
            cache.macrodefinitions[k] = def;
        }
    }

    recreateMacroDefinitionList();
}

async function updateTrackState() {
    const ret = await fetchTrackState();
    console.log(ret)

    ret.tracks.forEach(t => {
        activetrackmachines[t.index] = t.activeMachineId || '';
        // activemacrodefinitions[t.index] = t.activeMacro || '';
        // activesoundpresets[t.index] = t.activePreset || '';
    })

    recreateTrackList();
}

async function reloadMachines() {
    await updateSynthDefinitions();
    await saveCache();
    recreateTrackList();
    recreateActiveTrackTitle();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();
}

async function reloadMacroDefinitions() {
    await updateMacroDefinitionList();
    await saveCache();
    recreateTrackList();
    recreateActiveTrackTitle();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();
}

async function reloadSoundPresets() {
    await updateMacroSoundPresetList();
    await saveCache();
    recreateTrackList();
    recreateActiveTrackTitle();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();
}

async function init() {
    await loadCache();

    console.log('Loaded cache', cache);

    document.getElementById('uploadmacrodefinition')
        .addEventListener('click', uploadMacroDefinition);

    document.getElementById('uploadsoundpreset')
        .addEventListener('click', uploadSoundPreset);

    document.getElementById('activetrackmachine')
        .addEventListener('change', changeActiveTrackMachine);

    document.getElementById('activemacrodefinition')
        .addEventListener('change', changeActiveTrackMacroDefinition);

    document.getElementById('activesoundpreset')
        .addEventListener('change', changeActiveTrackSoundPreset);

    document.getElementById('defaultparameters')
        .addEventListener('click', setDefaultParameters);

    document.getElementById('randomizeparameters')
        .addEventListener('click', setRandomParameters);

    document.getElementById('reloadmacrodefinitions')
        .addEventListener('click', reloadMacroDefinitions);

    document.getElementById('reloadsoundpresets')
        .addEventListener('click', reloadSoundPresets);

    document.getElementById('reloadmachines')
        .addEventListener('click', reloadMachines);

    if (!cache.synthdefinitionids || cache.synthdefinitionids.length === 0) {
        await updateSynthDefinitions();
        await saveCache();
    }

    if (!cache.macrodefinitionids || cache.macrodefinitionids.length === 0) {
        await updateMacroDefinitionList();
        await saveCache();
    }

    if (!cache.soundpresetids || cache.soundpresetids.length === 0) {
        await updateMacroSoundPresetList();
        await saveCache();
    }

    // await updateMacroSoundPresetList();
    // await updateMacroDefinitionList();
    // await updateSynthDefinitions();

    recreateTrackList();
    recreateMacroDefinitionList();
    recreateSynthMachineList();
    recreateSoundPresetList();
    recreateActiveTrackTitle();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
    recreateActiveTrackParameters();

    await updateTrackState();
}

window.addEventListener('load', init);