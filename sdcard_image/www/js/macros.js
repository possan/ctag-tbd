let ROOT = ''
ROOT = 'http://192.168.4.1/api/v1'
// ROOT = '/api/v1/picoseq'

// let synthdefinitionids = [];
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
    synthdefinition: {},
    configfilelist: [],
    // synthdefinitions: {},
    // synthdefinitionids: [],
    // trackdefinitions: [],
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

    if (!cache.configfilelist) {
        cache.configfilelist = [];
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

    cache.synthdefinitionids = undefined;
    cache.synthdefinitions = undefined
    cache.synthdefinition.tracks = undefined;
}

async function saveCache() {
    console.log('Saving cache', cache);
    localStorage.setItem('picoseq_macro_cache', JSON.stringify(cache));
}


//
// API calls to TBD rest endpoints
//

async function fetchSynthDefinitionConfig() {
    if (ROOT === '') {
        return JSON.parse(`{"machines":["md-analogkick-2","db-1234","db","ab","extdrum","ro"]}`)
    }

    try {
      const response = await fetch(`${ROOT}/samples?getconfig=synthdefinitions.json`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        console.warn('err', e);
        return undefined;
    }
}

async function fetchSynthDefinitionList() {
    if (ROOT === '') {
        return JSON.parse(`{"machines":["md-analogkick-2","db-1234","db","ab","extdrum","ro"]}`)
    }

    try {
      const response = await fetch(`${ROOT}/synthdefinitionlist`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        console.warn('err', e);
        return undefined;
    }
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

    try {
        const response = await fetch(`${ROOT}/samples?getconfig=macrosoundpresets/${id}.json`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        console.warn('err', e);
        return undefined;
    }
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

    try {
        const response = await fetch(`${ROOT}/samples?getconfig=macrodefinitions/${id}.json`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        console.warn('err', e);
        return undefined;
    }
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

    try {
        const response = await fetch(`${ROOT}/macroapi`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        return { tracks: [] };
    }
}

async function fetchSamplesOutput() {
    if (ROOT === '') {
        return JSON.parse(`{"configfiles":[]}`)
    }

    try {
        const response = await fetch(`${ROOT}/samples`);
        const data = await response.json();
        await delay(100);
        return data;
    } catch(e) {
        console.warn('err', e);
        return undefined;
    }
}

async function putMacroDefinition(id, jsonstring) {
    try {
        const response = await fetch(`${ROOT}/samples?action=uploadconfig&path=macrodefinitions/${id}.json`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: jsonstring
        });
        await delay(100);
        return response.ok
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

async function putSoundPreset(id, jsonstring) {
    try {
        const response = await fetch(`${ROOT}/samples?action=uploadconfig&path=macrosoundpresets/${id}.json`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: jsonstring
        });
        await delay(100);
        return response.ok;
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

async function putSynthDefinition(jsonstring) {
    try {
        const response = await fetch(`${ROOT}/samples?action=uploadconfig&path=synthdefinitions.json`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: jsonstring
        });
        await delay(100);
        return response.ok;
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

async function putActiveMachineForTrack(trackindex, machineid) {
    try {
        const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ track: trackindex, machine: machineid })
        });
        await delay(100);
        return response.ok
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

async function putActiveMacroDefinitionForTrack(trackindex, macroid) {
    try {
        const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({ track: trackindex, macro: macroid })
        });
        await delay(100);
        return response.ok
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

async function putParametersForTrack(trackindex, parameters) {
    try {
        const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                track: trackindex,
                parameters: parameters
            })
        });
        await delay(100);
        return response.ok
    } catch(e) {
        console.warn('err', e);
        return false;
    }
}

function reloadOnDevice() {
    return fetch(`${ROOT}/macroapi?action=reload`, {
        method: 'POST',
    });
}

//
// UI update functions
//

function recreateSynthMachineList() {
    const root = document.getElementById('machines');
    root.innerHTML = ''

    for(const mach of cache.synthdefinition.machines) {
        // const mach = cache.synthdefinitions[id];

        const li = document.createElement('li');
        const title = document.createElement('b');
        if (mach) {
            title.textContent = `#${mach.id} ${mach.name}`;
        // } else {
        //     title.textContent = `#${id}...`;
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
        let track = cache.synthdefinition.tracks[t];
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
    const trackmeta = cache.synthdefinition.tracks[activetrack];
    document.getElementById('tracktitle').textContent = `Track ${activetrack + 1}: ${trackmeta ? trackmeta.name : '?'}`;
}

function recreateActiveTrackMachineList() {
    const sel = document.getElementById('activetrackmachine')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const trackinfo = cache.synthdefinition.tracks[activetrack];
    console.log('Recreating machine list for track', activetrack, trackinfo)
    if (trackinfo && trackinfo.machines) {
        for(const machid of trackinfo.machines) {
            const mach = cache.synthdefinition.machines.find(m => m.id === machid);
            // console.log('Adding machine option', machid, mach)
            if (mach) {
                const name = mach ? mach.name : machid;
                sel.options.add(new Option(name, machid));
            }
        }
    }

    const currentmachine = activetrackmachines[activetrack] || '';
    setSelectValue(sel, currentmachine)

    // const jsonel = document.getElementById('activetrackmachinejson')
    // const mach = cache.synthdefinition.machines.find(m => m.id === currentmachine)
    // if (mach) {
    //     jsonel.textContent = JSON.stringify(mach, null, 2);
    // } else {
    //     jsonel.textContent = '';
    // }
}

function recreateActiveTrackMacroDefinitionList() {
    const sel = document.getElementById('activemacrodefinition')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const selectablemachines = cache.synthdefinition.tracks[activetrack] ? cache.synthdefinition.tracks[activetrack].machines : [];

    const currentmachine = activetrackmachines[activetrack] || '';
    const trackinfo = cache.synthdefinition.tracks[activetrack];
    console.log('Recreating macro definition list for track', activetrack, currentmachine, trackinfo)
    for(const machid of cache.macrodefinitionids) {
        const macro = cache.macrodefinitions[machid];
        if (macro) {
            console.log('mach', selectablemachines, currentmachine, macro);
            if (selectablemachines.indexOf(macro.machine) !== -1) {
                sel.options.add(new Option(macro.machine + ': ' +  macro.name, machid));
            }
        }
    }

    const currentmacro = activemacrodefinitions[activetrack] || '';
    setSelectValue(sel, currentmacro)

    let jsonel = document.getElementById('activetrackmachinejson')
    const mach = cache.synthdefinition.machines.find(m => m.id === currentmachine)
    if (mach) {
        jsonel.textContent = JSON.stringify(mach, null, 2);
    } else {
        jsonel.textContent = '';
    }

    jsonel = document.getElementById('activemacrodefinitionjson')
    const macro = cache.macrodefinitions[currentmacro]
    if (macro) {
        jsonel.textContent = JSON.stringify(macro, null, 2);
    } else {
        jsonel.textContent = '';
    }
}

function recreateActiveTrackSoundPresetList() {
    const sel = document.getElementById('activesoundpreset')
    removeAllSelectValues(sel);

    sel.options.add(new Option('-- Select --', ''));

    const currentmachine = activepresetparameters[activetrack] || '';

    for(const presetid of cache.soundpresetids) {
        const preset = cache.soundpresets[presetid];
        console.log('preset', currentmachine, preset);
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

async function deleteSoundPreset(id) {
    if (confirm(`Are you sure you want to delete sound preset #${id}?`)) {
        const f = await fetch(`${ROOT}/samples?action=manage`, {
            method: 'POST',            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                action: 'deleteconfig',
                path: `macrosoundpresets/${id}.json`
            })
        })
    }
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
            // label.textContent = JSON.stringify(sp);
        }

        const but = document.createElement('button');
        but.textContent = 'Delete';
        but.addEventListener('click', deleteSoundPreset.bind(this, id));
        li.appendChild(but);

        // label.textContent = `#${track.index} "${track.name}" (machines: ${track.machines.join(', ')})`;
        li.appendChild(label);
        root.appendChild(li);
    }
}

async function deleteMacroDefinition(id) {
    if (confirm(`Are you sure you want to delete macro definition #${id}?`)) {
        const f = await fetch(`${ROOT}/samples?action=manage`, {
            method: 'POST',
            body: JSON.stringify({
                action: 'deleteconfig',
                path:  `macrodefinitions/${id}.json`
            })
        })
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
        // label.textContent = JSON.stringify(mach);
        // label.textContent = `#${track.index} "${track.name}" (machines: ${track.machines.join(', ')})`;
        li.appendChild(label);

        const but = document.createElement('button');
        but.textContent = 'Delete';
        but.addEventListener('click', deleteMacroDefinition.bind(this, id));
        li.appendChild(but);

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
        // await updateMacroDefinitionList();
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
        // await updateMacroSoundPresetList();
    } else {
        alert('Failed to upload sound preset');
    }
}

async function uploadSynthDefinition() {
    const jsonel = document.getElementById('uploadsynthdefinitionjson');
    const txt = jsonel.value;
    let json = undefined
    try {
        json = JSON.parse(txt);
    } catch (e) {
        alert('Invalid JSON: ' + e.message);
        return;
    }
    const packed = JSON.stringify(json);
    console.log('Uploading synth definition:', packed);
    const ok = await putSynthDefinition(packed)
    if (ok) {
        // await updateMacroDefinitionList();
    } else {
        alert('Failed to upload macro definition');
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
    const macrodefid = getSelectValue(ev.target)
    activemacrodefinitions[activetrack] = macrodefid;
    const macrodef = cache.macrodefinitions[macrodefid];
    console.log(`Selecting macro definition ${macrodefid} (machine ${macrodef?.machine}) for track ${activetrack}`);
    if (macrodef) {
        await putActiveMachineForTrack(activetrack, macrodef.machine);
        activetrackmachines[activetrack] = macrodef.machine;
    }
    await putActiveMacroDefinitionForTrack(activetrack, macrodefid);
    recreateActiveTrackMachineList();
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
    const config = await fetchSynthDefinitionConfig();
    cache.synthdefinition = config;

    // const ret = await fetchSynthDefinitionList();
    // cache.synthdefinitionids = ret.machines || [];

    recreateSynthMachineList();
    recreateTrackList();

    // cache.synthdefinitions = {};
    // for(const k of cache.synthdefinitionids) {
    //     const def = await fetchSynthDefinition(k);
    //     if (def) {
    //         cache.synthdefinitions[k] = def;
    //     }
    // }

    // recreateSynthMachineList();

    // cache.synthdefinition.tracks = [];
    // for(let t=0; t<16; t++) {
    //     const def = await fetchTrackDefinition(t);
    //     if (def) {
    //         cache.synthdefinition.tracks[t] = def;
    //     }
    // }

    recreateTrackList();
}

async function updateMacroSoundPresetList() {
    await updateMacroAndPresetDefinitionList();

    // const ret = await fetchMacroSoundPresetList();
    // cache.soundpresetids = ret.presets || [];
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


async function updateMacroAndPresetDefinitionList() {
    const samplesdata = await fetchSamplesOutput();
    console.log('samples data', samplesdata)

    /*

    "configfiles": [
        {
            "name": "mui-VctrSnt.jsn",
            "path": "sp",
            "size": 15740,
            "mtime": 315532802000
        },
        ...
        {
            "name": "md-kickmachine-2.json",
            "path": "macrodefinitions",
            "size": 1594,
            "mtime": 315532804000
        },
        ...
        {
            "name": "msp-kick2.json",
            "path": "macrosoundpresets",
            "size": 117,
            "mtime": 315532804000
        },

    */

    const macrodefinitionids = [];
    const macrosoundpresetids = [];
    for(const c of samplesdata.configfiles) {
        if (c.path === 'macrodefinitions') {
            macrodefinitionids.push(c.name.replace('.json', ''));
        } else if (c.path === 'macrosoundpresets') {
            macrosoundpresetids.push(c.name.replace('.json', ''));
        }
    }

    cache.macrodefinitionids = macrodefinitionids;
    cache.soundpresetids = macrosoundpresetids;
}

async function updateMacroDefinitionList() {
    await updateMacroAndPresetDefinitionList();

    // const samplesdata = await fetchSamplesOutput();
    // console.log('samples data', samplesdata)

    // const machine = [];
    // samplesdata.configfiles

    // const ret = await fetchMacroDefinitionList();
    // console.log(ret)

    // cache.macrodefinitionids = ret.machines || [];
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
        activetrackmachines[t.index] = t.machine || '';
        activemacrodefinitions[t.index] = t.macro || '';
        // activesoundpresets[t.index] = t.activePreset || '';
    })

    recreateTrackList();
    recreateActiveTrackMachineList();
    recreateActiveTrackMacroDefinitionList();
    recreateActiveTrackSoundPresetList();
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

    document.getElementById('uploadsynthdefinition')
        .addEventListener('click', uploadSynthDefinition);

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

    document.getElementById('reloadmachinesondevice')
        .addEventListener('click', reloadOnDevice);

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