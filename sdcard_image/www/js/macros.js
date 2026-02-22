const ROOT = 'http://192.168.4.1/api/v1/picoseq';

let synthdefs;
let synthdefinitionids = [];
let synthdefinitions = {};
let trackdefinitions = [];
let activetrackmachines = {};
let macrodefinitionids = [];
let macrodefinitions = [];
let soundpresetids = {};
let soundpresets = {};

async function delay(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function fetchSynthDefinitions() {
    const response = await fetch(`${ROOT}/synthdefinitionlist`);
    const data = await response.json();
    console.log(data);
    return data;
}

function recreateSynthMachineList() {
    const root = document.getElementById('machines');
    root.innerHTML = ''

    for(const id of synthdefinitionids) {
        const mach = synthdefinitions[id];

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
        let track = trackdefinitions[t];
        let st = activetrackmachines[t];

        const li = document.createElement('li');
        const title = document.createElement('b');

        if (track) {
            title.textContent = `#${track.index} "${track.name}"`;
        } else {
            title.textContent = `#${t}...`;
        }

        if (st) {
            title.textContent += ` (active machine: ${st})`;
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

async function updateSynthDefinitions() {
    const ret = await fetchSynthDefinitions();
    console.log(ret)

    synthdefinitionids = ret.machines || [];
    synthdefinitions = {};

    recreateSynthMachineList();
    recreateTrackList();

    for(const k of synthdefinitionids) {
        const response = await fetch(`${ROOT}/synthdefinition/${k}`);
        if (response.ok) {
            const data = await response.json();
            console.log(data);
            synthdefinitions[k] = data;
        }

        await delay(100);
    }

    recreateSynthMachineList();

    for(let t=0; t<16; t++) {
        const response = await fetch(`${ROOT}/trackdefinition/${t}`);
        if (response.ok) {
            const data = await response.json();
            console.log(data);
            trackdefinitions[t] = data;
        }

        await delay(100);
    }

    recreateTrackList();
}

function recreateSoundPresetList() {
    const root = document.getElementById('soundpresets');
    root.innerHTML = ''

    for(const id of soundpresetids || []) {
        const sp = soundpresets[id];

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


async function getMacroSoundPresetList() {
    const response = await fetch(`${ROOT}/soundpresets`);
    const data = await response.json();
    console.log(data);
    return data;
}

async function updateMacroSoundPresetList() {
    const ret = await getMacroSoundPresetList();
    console.log(ret)

    await delay(100);

    soundpresetids = ret.presets || [];
    soundpresets = {};

    recreateSoundPresetList();

    for(const k of soundpresetids) {
        const response = await fetch(`${ROOT}/soundpreset/${k}`);
        if (response.ok) {
            const data = await response.json();
            console.log(data);
            soundpresets[k] = data;
        }

        await delay(100);
    }

    recreateSoundPresetList();
}


function recreateMacroDefinitionList() {
    const root = document.getElementById('macrodefinitions');
    root.innerHTML = ''

    for(const id of macrodefinitionids || []) {
        const mach = macrodefinitions[id];

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

async function getMacroDefinitionList() {
    const response = await fetch(`${ROOT}/macrodefinitions`);
    const data = await response.json();
    console.log(data);
    return data;
}

async function updateMacroDefinitionList() {
    const ret = await getMacroDefinitionList();
    console.log(ret)

    await delay(100);

    macrodefinitionids = ret.machines || [];
    macrodefinitions = {};

    recreateMacroDefinitionList();

    for(const k of macrodefinitionids) {
        const response = await fetch(`${ROOT}/macrodefinition/${k}`);
        if (response.ok) {
            const data = await response.json();
            console.log(data);
            macrodefinitions[k] = data;
        }

        await delay(100);
    }

    recreateMacroDefinitionList();
}

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
    const id = json.id;
    const response = await fetch(`${ROOT}/macrodefinition/${id}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: packed
    });
    if (response.ok) {
        // alert('Macro definition uploaded successfully');
        await updateMacroDefinitionList();
    } else {
        alert('Failed to upload macro definition: ' + response.statusText);
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
    const id = json.id;
    const response = await fetch(`${ROOT}/soundpreset/${id}`, {
        method: 'PUT',
        headers: {
            'Content-Type': 'application/json'
        },
        body: packed
    });
    if (response.ok) {
        // alert('Sound preset uploaded successfully');
        await updateMacroSoundPresetList();
    } else {
        alert('Failed to upload sound preset: ' + response.statusText);
    }
}


async function getTrackState() {
    const response = await fetch(`${ROOT}/trackstatus`);
    const data = await response.json();
    console.log(data);
    return data;
}

async function updateTrackState() {
    const ret = await getTrackState();
    console.log(ret)

    ret.tracks.forEach(t => {
        activetrackmachines[t.index] = t.activeMachineId;
    })

    await delay(100);

    recreateTrackList();

    // macrodefinitionids = ret.machines || [];
    // macrodefinitions = {};

    // recreateMacroDefinitionList();

    // for(const k of macrodefinitionids) {
    //     const response = await fetch(`${ROOT}/macrodefinition/${k}`);
    //     if (response.ok) {
    //         const data = await response.json();
    //         console.log(data);
    //         macrodefinitions[k] = data;
    //     }

    //     await delay(100);
    // }

    // recreateMacroDefinitionList();
}

async function init() {
    document.getElementById('uploadmacrodefinition').addEventListener('click', uploadMacroDefinition);
    document.getElementById('uploadsoundpreset').addEventListener('click', uploadSoundPreset);
    await updateTrackState();
    await updateMacroSoundPresetList();
    await updateMacroDefinitionList();
    await updateSynthDefinitions();
}

window.addEventListener('load', init);