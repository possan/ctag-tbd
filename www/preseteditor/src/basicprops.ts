import { replaceDropdown, sortDropdown } from "./domutils";
import { MacroPreset } from "./macropreset";
import { outputMappingUpdate } from "./outputmappinglist";
import { parametersUpdate } from "./parameterlist";
import { previewUpdate } from "./preview";
import { SoundPreset } from "./soundpreset";
import {
  cache,
  machinedefinitions,
  macropreset,
  saveCache,
  selection,
  soundpreset,
} from "./state";
import {
  fetchSamplesFile,
  fetchSamplesResponse,
  fetchSynthDefinitionConfig,
  refreshDeviceConfig,
} from "./device";

async function handleClickCopyJson() {
  const jsonstring = JSON.stringify(macropreset.serialize());
  await navigator.clipboard.writeText(jsonstring);
}

async function handleDownloadJson() {
  const jsonstring = JSON.stringify(macropreset.serialize());
  const blob = new Blob([jsonstring], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `${macropreset.id || "preset"}.json`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

function handleUpdatePresetName(evttarget: HTMLInputElement) {
  macropreset.name = evttarget.value;
  updateJsonPreview();
}

function handleUpdatePresetId(evttarget: HTMLInputElement) {
  macropreset.id = evttarget.value;
  updateJsonPreview();
}

export function updateJsonPreview() {
  const jsonobj = macropreset.serialize();
  console.log("Macro preset json", jsonobj);
  // const json = JSON.stringify(jsonobj);
  // const textarea = document.getElementById('json') as HTMLTextAreaElement;
  // textarea.value = json;
  // (document.getElementById('jsonsize') as HTMLElement).textContent = json.length.toString();
}

export function loadMacroPresetFromJson(json: string) {
  let presetData = null;
  try {
    presetData = JSON.parse(json);
  } catch (err) {
    console.warn("Invalid json", err);
  }

  if (!presetData) {
    return;
  }

  macropreset.reset();
  macropreset.deserialize(presetData);
  basicUpdate();
  parametersUpdate();
  outputMappingUpdate();
  soundpreset.reset();
  soundpreset.recreateParameters(macropreset);
  soundpreset.resetToMacroParameters(macropreset);
  previewUpdate();
  updateJsonPreview();
}

function handleClickLoadJson() {
  const json = prompt("Paste macro preset json here:");
  if (!json) {
    return;
  }

  loadMacroPresetFromJson(json);
}

function handleClickUpload() {}

async function updateTrackDropdown() {
  const trackselect = document.getElementById(
    "targettrack",
  ) as HTMLSelectElement;
  const trackoptions =
    cache.synthdefinition?.tracks.map((t, idx) => ({
      value: `${idx}`,
      label: `#${idx}: ${t.name || "?"}`,
    })) || [];
  replaceDropdown(trackselect, trackoptions, `${selection.track}`);
}

async function updatePresetDropdowns() {
  console.log("Updating preset dropdowns, track is", selection.track);

  let validmachines: string[] = [];
  const track = cache.synthdefinition?.tracks[selection.track];
  if (track) {
    validmachines = track.machines;
  }
  console.log("Valid machine ids", validmachines);

  let validmacros = [];
  for (const m of cache.macrodefinitions || []) {
    if (validmachines.includes(m.machine)) {
      validmacros.push(m.id);
    }
  }
  console.log("Valid macro ids", validmacros);

  const soundpresetselect = document.getElementById(
    "soundpreset",
  ) as HTMLSelectElement;
  const soundpresetitems = [{ value: "", label: "-- Select --" }];
  for (const sp of cache.soundpresets || []) {
    if (validmacros.includes(sp.macro)) {
      soundpresetitems.push({
        value: sp.id || "",
        label: `${sp.group} /// ${sp.name || sp.id || ""} (${sp.id})`,
      });
    } else {
      console.log(
        "skipping sound preset",
        sp.id,
        "because macro",
        sp.macro,
        "not in valid machines for track",
      );
    }
  }
  sortDropdown(soundpresetitems);
  replaceDropdown(soundpresetselect, soundpresetitems, selection.soundpreset);

  const macropresetselect = document.getElementById(
    "macropreset",
  ) as HTMLSelectElement;
  const macropresetitems = [{ value: "", label: "-- Select --" }];
  for (const mp of cache.macrodefinitions || []) {
    if (validmachines.includes(mp.machine)) {
      macropresetitems.push({
        value: mp.id || "",
        label: `${mp.name || mp.id || ""} (${mp.id})`,
      });
    } else {
      console.log(
        "skipping macro preset",
        mp.id,
        "because machine",
        mp.machine,
        "not in valid machines for track",
      );
    }
  }
  sortDropdown(macropresetitems);
  replaceDropdown(macropresetselect, macropresetitems, selection.macro);
}

export async function refreshSynthDefinitions() {
  const def = await fetchSynthDefinitionConfig();
  console.log("refreshSynthDefinitions got", def);
  if (def) {
    cache.synthdefinition = def;
    machinedefinitions.deserialize(def);
    await saveCache();
  }

  machinedefinitions.deserialize(cache.synthdefinition!);

  updateTrackDropdown();
  updatePresetDropdowns();
}

export async function refreshMacroDefinitions() {
  const config = await fetchSamplesResponse();
  console.log("refreshMacroDefinitions got", config);

  cache.macrodefinitions = [];

  for (const cf of config.configfiles) {
    console.log("checking config file", cf);
    if (cf.path === "macrodefinitions") {
      const f = await fetchSamplesFile(cf.path + "/" + cf.name);
      console.log("got sound preset config", f);
      if (f) {
        const sp = new MacroPreset();
        if (sp.deserialize(f)) {
          cache.macrodefinitions.push(f);
        }
      }
    }
  }

  await saveCache();

  updatePresetDropdowns();
}

export async function refreshSoundPresets() {
  const config = await fetchSamplesResponse();
  console.log("refreshSoundPresets got", config);

  cache.soundpresets = [];

  for (const cf of config.configfiles) {
    console.log("checking config file", cf);
    if (cf.path === "macrosoundpresets") {
      const f = await fetchSamplesFile(cf.path + "/" + cf.name);
      console.log("got sound preset config", f);
      if (f) {
        const sp = new SoundPreset();
        if (sp.deserialize(f)) {
          cache.soundpresets.push(f);
        }
      }
    }
  }

  await saveCache();

  updatePresetDropdowns();
}

async function loadSelectedSoundPresetAndMacro() {
  const id = (document.getElementById("soundpreset") as HTMLSelectElement)
    .value;
  if (!id) {
    return;
  }

  const sp = cache.soundpresets?.find((sp) => sp.id === id);
  if (!sp) {
    console.warn("Selected sound preset not found in cache", id);
    return;
  }

  const md = cache.macrodefinitions?.find((md) => md.id === sp.macro);
  if (!md) {
    console.warn("Selected macro preset not found in cache", sp.macro);
    return;
  }

  console.log("Loading macro preset", md);
  macropreset.deserialize(md);

  console.log("Loading sound preset", sp);
  soundpreset.deserialize(sp);

  basicUpdate();
  outputMappingUpdate();
  parametersUpdate();
  previewUpdate();
}

async function loadSelectedMacroPresetAndDefaults() {
  const id = (document.getElementById("macropreset") as HTMLSelectElement)
    .value;
  if (!id) {
    return;
  }

  const md = cache.macrodefinitions?.find((md) => md.id === id);
  if (!md) {
    console.warn("Selected macro preset not found in cache", id);
    return;
  }

  console.log("Loading macro preset", md);

  macropreset.deserialize(md);
  soundpreset.recreateParameters(macropreset);
  soundpreset.resetToMacroParameters(macropreset);

  basicUpdate();
  outputMappingUpdate();
  parametersUpdate();
  previewUpdate();
}

export function basicUpdate() {
  (document.getElementById("presetid") as HTMLInputElement).value =
    macropreset.id;
  (document.getElementById("presetname") as HTMLInputElement).value =
    macropreset.name;

  updateJsonPreview();
  updateTrackDropdown();
  updatePresetDropdowns();
}

export function basicHandleClick(_evt: Event, ds: Record<string, string>) {
  if (ds.action === "copyjson") {
    handleClickCopyJson();
  } else if (ds.action === "downloadjson") {
    handleDownloadJson();
  } else if (ds.action === "loadjson") {
    handleClickLoadJson();
  } else if (ds.action === "upload") {
    handleClickUpload();
  } else if (ds.action === "refreshsynthdefinitions") {
    refreshSynthDefinitions();
  } else if (ds.action === "refreshsoundpresets") {
    refreshSoundPresets();
  } else if (ds.action === "refreshmacrodefinitions") {
    refreshMacroDefinitions();
  } else if (ds.action === "loadsoundandmacro") {
    loadSelectedSoundPresetAndMacro();
  } else if (ds.action === "loadmacroanddefaults") {
    loadSelectedMacroPresetAndDefaults();
  } else if (ds.action === "refreshdeviceconfig") {
    refreshDeviceConfig();
  }
}

export function basicHandleChange(evt: Event, ds: Record<string, string>) {
  if (ds.inputfield === "presetname") {
    handleUpdatePresetName(evt.target as HTMLInputElement);
  } else if (ds.inputfield === "presetid") {
    handleUpdatePresetId(evt.target as HTMLInputElement);
  } else if (ds.selectfield === "targettrack") {
    const select = evt.target as HTMLSelectElement;
    selection.track = parseInt(select.value);
    updatePresetDropdowns();
  }
}
