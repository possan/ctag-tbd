import {
  deleteSampleFile,
  updateLiveParametersForTrack,
  updateSampleFile,
} from "./device";
import { getCombinedDataset } from "./domutils";
import { parametersUpdate } from "./parameterlist";
import { macropreset, selection, soundpreset } from "./state";

function updateSoundPresetJson() {
  const jsonobj = soundpreset.serialize(macropreset);
  console.log("Sound preset json", jsonobj);
  // const json = JSON.stringify(jsonobj);
  // const textarea = document.getElementById('soundpresetjson') as HTMLTextAreaElement;
  // textarea.value = json;
  // (document.getElementById('soundpresetjsonsize') as HTMLElement).textContent = json.length.toString();
}

export function previewUpdate() {
  soundpreset.recreateParameters(macropreset);

  const el = document.getElementById("previewgroup") as HTMLElement;
  (
    el.querySelector(
      'input[data-inputfield="soundpresetid"]',
    ) as HTMLInputElement
  ).value = soundpreset.id;
  (
    el.querySelector(
      'input[data-inputfield="soundpresetname"]',
    ) as HTMLInputElement
  ).value = soundpreset.name;
  (
    el.querySelector(
      'input[data-inputfield="soundpresetgroup"]',
    ) as HTMLInputElement
  ).value = soundpreset.group;
  (
    el.querySelector(
      'input[data-inputfield="soundpresetmacroid"]',
    ) as HTMLInputElement
  ).value = soundpreset.macroId;

  const el2 = document.getElementById("soundparametergroups") as HTMLElement;

  el2.innerHTML = "";
  for (const pg of macropreset.parameterGroups) {
    const el4 = document.createElement("li");
    el4.innerHTML = `
            <b>?</b>
            <ul></ul>
        `;

    if (pg.name) {
      (el4.querySelector("b") as HTMLElement).textContent = pg.name;

      for (const p of pg.parameters) {
        if (p.name) {
          const el5 = document.createElement("li");
          el5.dataset.soundparameter = `${p.index}`;
          el5.innerHTML = `
                        <b>?</b>
                        <div class="inputs">
                            <input type="range" /> <span data-outputfield="soundpresetparamvalue">123</span>
                        </div>
                    `;

          const sp = soundpreset.parameters.find((sp) => sp.index === p.index);

          (el5.querySelector("b") as HTMLElement).textContent = p.name;
          const input = el5.querySelector("input") as HTMLInputElement;
          input.min = p.minValue.toString();
          input.max = p.maxValue.toString();
          input.value = (sp ? sp.value : p.defaultValue).toString();
          input.dataset.inputfield = "soundpresetparam";
          if (!p.name) {
            input.disabled = true;
          }

          (el5.querySelector("span") as HTMLElement).textContent = sp
            ? sp.value.toString()
            : p.defaultValue.toString();

          (el4.querySelector("ul") as HTMLElement).appendChild(el5);
        }
      }
      el2.appendChild(el4);
    }
  }

  updateSoundPresetJson();
}

async function copySoundPresetJson() {
  const jsonstring = JSON.stringify(soundpreset.serialize(macropreset));
  await navigator.clipboard.writeText(jsonstring);
}

async function downloadSoundPresetJson() {
  const jsonstring = JSON.stringify(soundpreset.serialize(macropreset));
  const blob = new Blob([jsonstring], { type: "application/json" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `${soundpreset.id || "preset"}.json`;
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
}

export async function loadSoundPresetFromJson(json: string) {
  let presetData = null;
  try {
    presetData = JSON.parse(json);
  } catch (err) {
    console.warn("Invalid json", err);
  }

  if (!presetData) {
    return;
  }

  soundpreset.deserialize(presetData);

  previewUpdate();
  parametersUpdate();
  updateSoundPresetJson();
}

async function pasteSoundPresetJson() {
  const json = prompt("Paste sound preset json here:");
  if (!json) {
    return;
  }

  loadSoundPresetFromJson(json);
}

async function handleUploadSoundPreset() {
  // TODO: Implement upload sound preset logic here
  const id = soundpreset.id;
  if (!id) {
    alert("Please enter an ID for the sound preset before uploading.");
    return;
  }

  const jsonstring = JSON.stringify(soundpreset.serialize(macropreset));

  const path = "macrosoundpresets/" + id + ".json";
  updateSampleFile(path, jsonstring);
}

async function handleDeleteSoundPreset() {
  // TODO: Implement delete sound preset logic here
  const id = soundpreset.id;
  if (!id) {
    alert("Please enter an ID for the sound preset before uploading.");
    return;
  }

  if (
    !confirm(
      `Are you sure you want to delete the sound preset "${soundpreset.name}"? This action cannot be undone.`,
    )
  ) {
    return;
  }

  const path = "macrosoundpresets/" + id + ".json";
  deleteSampleFile(path);
}

function updateDeviceParameters() {
  const jsonobj = soundpreset.serialize(macropreset);
  updateLiveParametersForTrack(selection.track, jsonobj.values);
}

function handleParameterChange(evt: Event, ds: Record<string, string>) {
  const paramindex = ~~ds.soundparameter;
  const value = ~~(evt.target as HTMLInputElement).value;
  soundpreset.setParameterValue(paramindex, value);

  const spans = document.querySelectorAll(
    'span[data-outputfield="soundpresetparamvalue"]',
  ) as NodeListOf<HTMLElement>;
  for (const span of spans) {
    const ds2 = getCombinedDataset(span);
    if (~~ds2.soundparameter === paramindex) {
      span.textContent = value.toString();
    }
  }

  updateSoundPresetJson();

  const el = document.querySelector(
    'input[data-inputfield="autoupdatesoundpresetparameters"]',
  ) as HTMLInputElement;
  if (el.checked) {
    updateDeviceParameters();
  }
}

export function previewHandleClick(_evt: Event, ds: Record<string, string>) {
  if (ds.action === "resetsoundpreset") {
    soundpreset.resetToMacroParameters(macropreset);
    previewUpdate();
  } else if (ds.action === "randomizesoundpreset") {
    soundpreset.randomizeParameters(macropreset);
    previewUpdate();
  } else if (ds.action === "copysoundpresetjson") {
    copySoundPresetJson();
  } else if (ds.action === "loadsoundpresetjson") {
    pasteSoundPresetJson();
  } else if (ds.action === "downloadsoundpresetjson") {
    downloadSoundPresetJson();
  } else if (ds.action === "uploadsoundpreset") {
    handleUploadSoundPreset();
  } else if (ds.action === "deletesoundpreset") {
    handleDeleteSoundPreset();
  } else if (ds.action === "updatesoundpresetparameters") {
    updateDeviceParameters();
  }
}

export function previewHandleChange(evt: Event, ds: Record<string, string>) {
  if (ds.inputfield === "soundpresetid") {
    soundpreset.id = (evt.target as HTMLInputElement).value;
    previewUpdate();
  } else if (ds.inputfield === "soundpresetname") {
    soundpreset.name = (evt.target as HTMLInputElement).value;
    previewUpdate();
  } else if (ds.inputfield === "soundpresetgroup") {
    soundpreset.group = (evt.target as HTMLInputElement).value;
    previewUpdate();
  } else if (ds.inputfield === "soundpresetmacroid") {
    soundpreset.macroId = (evt.target as HTMLInputElement).value;
    previewUpdate();
  } else if (ds.inputfield === "soundpresetparam") {
    handleParameterChange(evt, ds);
  }
}
