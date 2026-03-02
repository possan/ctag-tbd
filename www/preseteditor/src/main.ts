import {
  basicHandleChange,
  basicHandleClick,
  basicUpdate,
  loadMacroPresetFromJson,
  refreshSynthDefinitions,
} from "./basicprops";
import { updateSampleFile } from "./device";
import { getCombinedDataset } from "./domutils";
import { MacroPreset } from "./macropreset";
import {
  outputMappingHandleChange,
  outputMappingHandleClick,
  outputMappingUpdate,
} from "./outputmappinglist";
import {
  parameterHandleChange,
  parametersHandleClick,
  parametersUpdate,
} from "./parameterlist";
import {
  loadSoundPresetFromJson,
  previewHandleChange,
  previewHandleClick,
  previewUpdate,
} from "./preview";
import { SoundPreset } from "./soundpreset";
import {
  cache,
  loadCache,
  machinedefinitions,
  macropreset,
  soundpreset,
} from "./state";
import type { ISerializedMacroPreset, ISerializedSoundPreset } from "./types";

function handleClick(evt: Event) {
  const ds = getCombinedDataset(evt.target as HTMLElement);
  // console.log('Click event', ds, evt.target);
  basicHandleClick(evt, ds);
  parametersHandleClick(evt, ds);
  outputMappingHandleClick(evt, ds);
  previewHandleClick(evt, ds);
}

function handleChange(evt: Event) {
  const ds = getCombinedDataset(evt.target as HTMLElement);
  // console.log('Change event', ds, evt.target);
  basicHandleChange(evt, ds);
  parameterHandleChange(evt, ds);
  outputMappingHandleChange(evt, ds);
  previewHandleChange(evt, ds);
}

async function initMachineDefinitions() {
  if (!cache.synthdefinition) {
    await refreshSynthDefinitions();
  }

  machinedefinitions.deserialize(cache.synthdefinition!);
}

async function initMacroPreset() {
  macropreset.reset();

  macropreset.id = "db-1234";
  macropreset.name = "My preset";
  macropreset.machineId = "db";

  macropreset.parameterGroups = [
    {
      index: 0,
      name: "Group 1",
      parameters: [
        {
          index: 0,
          name: "Cutoff",
          defaultValue: 50,
          minValue: 10,
          maxValue: 90,
          resolution: 50,
          presentation: "",
        },
        {
          index: 1,
          name: "Reso",
          defaultValue: 70,
          minValue: 0,
          maxValue: 127,
          resolution: 127,
          presentation: "",
        },
        {
          index: 2,
          name: "Envmod",
          defaultValue: 30,
          minValue: 0,
          maxValue: 127,
          resolution: 127,
          presentation: "",
        },
      ],
    },
    {
      index: 1,
      name: "Group 2",
      parameters: [
        {
          index: 4,
          name: "Parameter 1",
          defaultValue: 20,
          minValue: 0,
          maxValue: 127,
          resolution: 127,
          presentation: "",
        },
        {
          index: 5,
          name: "Parameter 2",
          defaultValue: 80,
          minValue: 0,
          maxValue: 127,
          resolution: 127,
          presentation: "",
        },
      ],
    },
    {
      index: 2,
      name: "",
      parameters: [],
    },
    {
      index: 3,
      name: "",
      parameters: [],
    },
  ];

  macropreset.outputMappings = [
    {
      index: 0,
      ctrl: 8,
      startvalue: 10,
      sources: [
        {
          source: 0,
          mul: 3,
          div: 10,
        },
        {
          source: 1,
          mul: 5,
          div: 7,
        },
      ],
    },
    {
      index: 1,
      ctrl: 9,
      startvalue: 127,
      sources: [
        {
          source: 2,
          mul: 1,
          div: 1,
        },
      ],
    },
  ];

  const mach = machinedefinitions.getMachineById(macropreset.machineId);
  if (mach) {
    macropreset.addMissingOutputMappings(mach);
  }
}

async function initSoundPreset() {
  soundpreset.reset();

  soundpreset.id = "dummy-1234";
  soundpreset.name = "My sound preset";
  soundpreset.group = "My group";
  soundpreset.macroId = "db-1234";

  soundpreset.parameters = [
    {
      index: 0,
      value: 70,
    },
    {
      index: 1,
      value: 20,
    },
  ];

  soundpreset.recreateParameters(macropreset);
  soundpreset.resetToMacroParameters(macropreset);
}

async function getDroppedTextFile(file: File): Promise<string | null> {
  return new Promise((resolve) => {
    var reader = new FileReader();
    reader.onload = function (e2) {
      console.log("File arraybuffer", e2.target!.result);
      const txt = new TextDecoder().decode(e2.target!.result as ArrayBuffer);
      resolve(txt);
    };
    reader.onerror = function (e2) {
      console.warn("Error reading file", e2);
      resolve(null);
    };
    reader.readAsArrayBuffer(file);
  });
}

async function handleDropFileOnConnectionGroup(files: FileList) {
  console.log("Dropped files on connection group", files);

  const uploads = [];
  const errors = [];

  for (const file of files) {
    console.log("File", file.name, file.type, file.size);
    if (file.type.match(/text.*/) || file.type.match(/application.*/)) {
      const filedata = await getDroppedTextFile(file);
      console.log("File text content", filedata);
      if (filedata) {
        let j: any = {};
        try {
          j = JSON.parse(filedata);
        } catch (e) {}
        if (j) {
          // we have some json, try to load it as a macro preset
          const md = new MacroPreset();
          if (
            md.detect(j as ISerializedMacroPreset) &&
            md.deserialize(j as ISerializedMacroPreset) &&
            md.id
          ) {
            uploads.push({
              path: `macrodefinitions/${md.id}.json`,
              json: filedata,
            });
          } else {
            const sp = new SoundPreset();
            if (
              sp.detect(j as ISerializedSoundPreset) &&
              sp.deserialize(j as ISerializedSoundPreset) &&
              sp.id
            ) {
              uploads.push({
                path: `macrosoundpresets/${sp.id}.json`,
                json: filedata,
              });
            } else {
              if (
                j.tracks !== undefined &&
                j.machines !== undefined &&
                Array.isArray(j.tracks) &&
                Array.isArray(j.machines)
              ) {
                uploads.push({
                  path: `synthdefinitions.json`,
                  json: filedata,
                });
              } else {
                errors.push({
                  error: "Unknown json contents: " + file.name,
                  filename: file.name,
                });
              }
            }
          }
        }
      } else {
        errors.push({
          error: "Could not read file " + file.name,
          filename: file.name,
        });
      }
    } else {
      errors.push({
        error: "Unknown file type " + file.type,
        filename: file.name,
      });
    }
  }

  console.log("To upload", uploads);
  console.log("Errors", errors);

  for (const u of uploads) {
    console.log("Uploading", u);
    await updateSampleFile(u.path, u.json);
  }

  if (uploads.length > 0) {
    alert("Uploaded " + uploads.length + " files successfully!");
  }
  if (errors.length > 0) {
    alert(errors.map((e) => e.error).join(" "));
  }
}

async function handleDropFileOnMacroPresetGroup(files: FileList) {
  console.log("Dropped files on macro preset group", files);

  if (files.length > 0) {
    const file = files[0];
    console.log("File", file.name, file.type, file.size);
    if (file.type.match(/text.*/) || file.type.match(/application.*/)) {
      const json = await getDroppedTextFile(file);
      console.log("File text content", json);
      if (json) {
        loadMacroPresetFromJson(json);
      }
    }
  }
}

async function handleDropFileOnSoundPresetGroup(files: FileList) {
  console.log("Dropped files on sound preset group", files);

  if (files.length > 0) {
    const file = files[0];
    console.log("File", file.name, file.type, file.size);
    if (file.type.match(/text.*/) || file.type.match(/application.*/)) {
      const json = await getDroppedTextFile(file);
      console.log("File text content", json);
      if (json) {
        loadSoundPresetFromJson(json);
      }
    }
  }
}

async function initDropzone(
  id: string,
  drophandler: (files: FileList) => void = () => {},
) {
  const el = document.getElementById(id)!;
  let over: HTMLElement | undefined = undefined;

  el.addEventListener("dragover", (e) => {
    e.stopPropagation();
    e.preventDefault();
    e.dataTransfer!.dropEffect = "copy";
  });

  el.addEventListener("dragenter", (e) => {
    console.log("dragenter", e);
    e.stopPropagation();
    e.preventDefault();
    if (e.target === el) {
      el.classList.add("droppable");
      over = el;
    }
  });

  el.addEventListener("dragleave", (e) => {
    console.log("dragleave", e);
    e.stopPropagation();
    e.preventDefault();
    if (e.target === over) {
      el.classList.remove("droppable");
      over = undefined;
    }
  });

  el.addEventListener("drop", function (e) {
    e.stopPropagation();
    e.preventDefault();
    var files = e.dataTransfer!.files; // Array of all files
    el.classList.remove("droppable");
    console.log("dropped files", files);
    drophandler(files);
  });
}

async function initUI() {
  document.addEventListener("click", handleClick);
  document.addEventListener("change", handleChange);

  initDropzone("connectiongroup", handleDropFileOnConnectionGroup);
  initDropzone("macropresetgroup", handleDropFileOnMacroPresetGroup);
  initDropzone("previewgroup", handleDropFileOnSoundPresetGroup);

  basicUpdate();
  parametersUpdate();
  outputMappingUpdate();
  previewUpdate();
}

async function init() {
  console.log("Initializing preset editor");

  await loadCache();
  await initMachineDefinitions();
  await initMacroPreset();
  await initSoundPreset();
  await initUI();
}

window.addEventListener("load", init);
