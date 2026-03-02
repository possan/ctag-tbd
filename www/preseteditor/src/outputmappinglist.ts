import { basicUpdate, updateJsonPreview } from "./basicprops";
import {
  deleteSampleFile,
  putActiveMachineForTrack,
  putActiveMacroDefinitionForTrack,
  updateSampleFile,
} from "./device";
import { getDropdownValue, replaceDropdown, sortDropdown } from "./domutils";
import type { MacroPresetOutputMapping } from "./macropreset";
import { parametersUpdate } from "./parameterlist";
import { machinedefinitions, macropreset, selection } from "./state";

let editingMappingIndex = -1;

function updateMachineList() {
  const el = document.getElementById("targetdevice") as HTMLSelectElement;
  const items = machinedefinitions.machines
    .filter((m) => m.id)
    .map((m) => ({ value: m.id, label: m.name + " (" + m.id + ")" }));
  sortDropdown(items);
  replaceDropdown(el, items, macropreset.machineId);
}

function appendOutputMappingEditor(
  parent: HTMLElement,
  mapping: MacroPresetOutputMapping,
) {
  const el2 = document.createElement("div");
  el2.classList.add("outputmapping");
  el2.classList.add("editor");
  el2.innerHTML = `
    <p>
        <label>Output type: <span data-displayfield="mappingtype">?</span></label><br/>
        <label>Output CC/NRPM: <span data-displayfield="mappingcc">?</span></label><br/>
        <label>Synth default value: <span data-displayfield="devicedefault">?</span></label><br/>
        <label>Starting value: <input type="number" data-inputfield="startvalue" value="0" size="4" /></label><br/>
    </p>

    <p>
        Sources:<br/>

        <select data-source="1" data-x="y"><option>Cutoff</option></select> *
        <input data-multiplier="1" type="number" value="0" size="4"> /
        <input data-divider="1" type="number" value="0" size="4">
        <br/>

        <select data-source="2" data-x="y"><option>Resonance</option></select> *
        <input data-multiplier="2" type="number" value="0" size="4"> /
        <input data-divider="2" type="number" value="0" size="4">
        <br/>

        <select data-source="3" data-x="y"><option>-</option></select> *
        <input data-multiplier="3" type="number" value="0" size="4"> /
        <input data-divider="3" type="number" value="0" size="4">
        <br/>

        <select data-source="4" data-x="y"><option>-</option></select> *
        <input data-multiplier="4" type="number" value="0" size="4"> /
        <input data-divider="4" type="number" value="0" size="4">
        <br/>

        <select data-source="5" data-x="y"><option>-</option></select> *
        <input data-multiplier="5" type="number" value="0" size="4"> /
        <input data-divider="5" type="number" value="0" size="4">
        <br/>
    </p>

    <p>
        <button data-action="savemapping">Save mapping</button>
        <button data-action="deletemapping">Delete mapping</button>
    </p>
  `;

  const machmap = machinedefinitions.getMachineById(macropreset.machineId);
  const machmapparam =
    machmap && machmap.parameters && machmap.parameters[mapping.index];

  const parameters = [];
  parameters.push({
    value: "-1",
    label: "- Select -",
  });
  parameters.push({
    value: "-1",
    label: "None",
  });
  for (const pg of macropreset.parameterGroups) {
    if (pg.name) {
      for (const p of pg.parameters) {
        if (p.name) {
          parameters.push({
            value: `${p.index}`,
            label: pg.name + "/" + p.name,
          });
        }
      }
    }
  }

  let source1 = -1;
  let multiplier1 = 1;
  let divider1 = 1;
  let source2 = -1;
  let multiplier2 = 1;
  let divider2 = 1;
  let source3 = -1;
  let multiplier3 = 1;
  let divider3 = 1;
  let source4 = -1;
  let multiplier4 = 1;
  let divider4 = 1;
  let source5 = -1;
  let multiplier5 = 1;
  let divider5 = 1;
  if (mapping.sources && mapping.sources.length > 0) {
    source1 = mapping.sources[0].source;
    multiplier1 = mapping.sources[0].mul;
    divider1 = mapping.sources[0].div;
  }
  if (mapping.sources && mapping.sources.length > 1) {
    source2 = mapping.sources[1].source;
    multiplier2 = mapping.sources[1].mul;
    divider2 = mapping.sources[1].div;
  }
  if (mapping.sources && mapping.sources.length > 2) {
    source3 = mapping.sources[2].source;
    multiplier3 = mapping.sources[2].mul;
    divider3 = mapping.sources[2].div;
  }
  if (mapping.sources && mapping.sources.length > 3) {
    source4 = mapping.sources[3].source;
    multiplier4 = mapping.sources[3].mul;
    divider4 = mapping.sources[3].div;
  }
  if (mapping.sources && mapping.sources.length > 4) {
    source5 = mapping.sources[4].source;
    multiplier5 = mapping.sources[4].mul;
    divider5 = mapping.sources[4].div;
  }

  replaceDropdown(
    el2.querySelector('select[data-source="1"]') as HTMLSelectElement,
    parameters,
    `${source1}`,
  );
  replaceDropdown(
    el2.querySelector('select[data-source="2"]') as HTMLSelectElement,
    parameters,
    `${source2}`,
  );
  replaceDropdown(
    el2.querySelector('select[data-source="3"]') as HTMLSelectElement,
    parameters,
    `${source3}`,
  );
  replaceDropdown(
    el2.querySelector('select[data-source="4"]') as HTMLSelectElement,
    parameters,
    `${source4}`,
  );
  replaceDropdown(
    el2.querySelector('select[data-source="5"]') as HTMLSelectElement,
    parameters,
    `${source5}`,
  );
  (
    el2.querySelector('input[data-inputfield="startvalue"]') as HTMLInputElement
  ).value = mapping.startvalue.toString();
  (el2.querySelector('input[data-multiplier="1"]') as HTMLInputElement).value =
    multiplier1.toString();
  (el2.querySelector('input[data-multiplier="2"]') as HTMLInputElement).value =
    multiplier2.toString();
  (el2.querySelector('input[data-multiplier="3"]') as HTMLInputElement).value =
    multiplier3.toString();
  (el2.querySelector('input[data-multiplier="4"]') as HTMLInputElement).value =
    multiplier4.toString();
  (el2.querySelector('input[data-multiplier="5"]') as HTMLInputElement).value =
    multiplier5.toString();
  (el2.querySelector('input[data-divider="1"]') as HTMLInputElement).value =
    divider1.toString();
  (el2.querySelector('input[data-divider="2"]') as HTMLInputElement).value =
    divider2.toString();
  (el2.querySelector('input[data-divider="3"]') as HTMLInputElement).value =
    divider3.toString();
  (el2.querySelector('input[data-divider="4"]') as HTMLInputElement).value =
    divider4.toString();
  (el2.querySelector('input[data-divider="5"]') as HTMLInputElement).value =
    divider5.toString();

  (
    el2.querySelector('span[data-displayfield="mappingtype"]') as HTMLElement
  ).textContent = (machmapparam && machmapparam.type) ?? "??";

  (
    el2.querySelector('span[data-displayfield="mappingcc"]') as HTMLElement
  ).textContent =
    machmapparam && machmapparam.ctrl && machmapparam.ctrl.toString()
      ? machmapparam.ctrl.toString()
      : "??";

  (
    el2.querySelector('span[data-displayfield="devicedefault"]') as HTMLElement
  ).textContent =
    machmapparam && machmapparam.def ? machmapparam.def.toString() : "??";

  parent.appendChild(el2);
}

function appendOutputMappingItem(
  parent: HTMLElement,
  mapping: MacroPresetOutputMapping,
) {
  const el2 = document.createElement("li");
  el2.dataset.outputmapping = `${mapping.index}`;

  const formula = [
    mapping.startvalue.toString(),
    ...(mapping.sources || [])
      .filter((s) => s.source !== undefined && s.source !== -1)
      .map((s) => {
        let sourcename = "[" + s.source + "]";
        for (const g of macropreset.parameterGroups) {
          for (const p of g.parameters) {
            if (p.index === s.source) {
              // sourcename = g.name + '.' +
              sourcename = p.name;
              // + '[' + JSON.stringify(p) + ']';
              break;
            }
          }
        }
        return `(${sourcename}*${s.mul}/${s.div})`;
      }),
  ].join(" + ");

  const machmap = machinedefinitions.getMachineById(macropreset.machineId);
  const machmapparam =
    machmap &&
    machmap.parameters &&
    machmap.parameters.find((p) => p.ctrl === mapping.ctrl);

  if (!machmapparam) {
    el2.classList.add("bad");
  }

  var cc = machmapparam?.ctrl ?? -1;
  var def = machmapparam?.def ?? -1;

  el2.innerHTML = `
    <b>${machmapparam ? machmapparam.name : "?" + mapping.ctrl}</b>
    <span> (CC ${cc}, default ${def}) = ${formula}</span>
  `;

  if (mapping.index === editingMappingIndex) {
    el2.classList.add("editing");
    appendOutputMappingEditor(el2, mapping);
  } else {
    el2.dataset.action = "editmapping";
    el2.dataset.outputmapping = `${mapping.index}`;
  }

  parent.appendChild(el2);
}

function handleSelectOutputMapping(id: number) {
  editingMappingIndex = id;
  outputMappingUpdate();
}

function handleSaveMapping() {
  const root = document.querySelector(
    '*[data-outputmapping="' + editingMappingIndex + '"]',
  ) as HTMLElement;

  const startvalue = ~~(
    root.querySelector(
      'input[data-inputfield="startvalue"]',
    ) as HTMLInputElement
  ).value;
  const source1 = ~~getDropdownValue(
    root.querySelector('select[data-source="1"]') as HTMLSelectElement,
  );
  const source2 = ~~getDropdownValue(
    root.querySelector('select[data-source="2"]') as HTMLSelectElement,
  );
  const source3 = ~~getDropdownValue(
    root.querySelector('select[data-source="3"]') as HTMLSelectElement,
  );
  const source4 = ~~getDropdownValue(
    root.querySelector('select[data-source="4"]') as HTMLSelectElement,
  );
  const source5 = ~~getDropdownValue(
    root.querySelector('select[data-source="5"]') as HTMLSelectElement,
  );
  const multiplier1 = ~~(
    root.querySelector('input[data-multiplier="1"]') as HTMLInputElement
  ).value;
  const multiplier2 = ~~(
    root.querySelector('input[data-multiplier="2"]') as HTMLInputElement
  ).value;
  const multiplier3 = ~~(
    root.querySelector('input[data-multiplier="3"]') as HTMLInputElement
  ).value;
  const multiplier4 = ~~(
    root.querySelector('input[data-multiplier="4"]') as HTMLInputElement
  ).value;
  const multiplier5 = ~~(
    root.querySelector('input[data-multiplier="5"]') as HTMLInputElement
  ).value;
  const divider1 = ~~(
    root.querySelector('input[data-divider="1"]') as HTMLInputElement
  ).value;
  const divider2 = ~~(
    root.querySelector('input[data-divider="2"]') as HTMLInputElement
  ).value;
  const divider3 = ~~(
    root.querySelector('input[data-divider="3"]') as HTMLInputElement
  ).value;
  const divider4 = ~~(
    root.querySelector('input[data-divider="4"]') as HTMLInputElement
  ).value;
  const divider5 = ~~(
    root.querySelector('input[data-divider="5"]') as HTMLInputElement
  ).value;

  macropreset.updateOutputMapping(editingMappingIndex, (om) => {
    om.startvalue = startvalue;
    om.sources = [];
    if (source1 !== undefined && source1 !== -1) {
      om.sources.push({
        source: source1,
        mul: multiplier1,
        div: divider1,
      });
    }
    if (source2 !== undefined && source2 !== -1) {
      om.sources.push({
        source: source2,
        mul: multiplier2,
        div: divider2,
      });
    }
    if (source3 !== undefined && source3 !== -1) {
      om.sources.push({
        source: source3,
        mul: multiplier3,
        div: divider3,
      });
    }
    if (source4 !== undefined && source4 !== -1) {
      om.sources.push({
        source: source4,
        mul: multiplier4,
        div: divider4,
      });
    }
    if (source5 !== undefined && source5 !== -1) {
      om.sources.push({
        source: source5,
        mul: multiplier5,
        div: divider5,
      });
    }
  });

  editingMappingIndex = -1;
  outputMappingUpdate();
  updateJsonPreview();
}

function handleDeleteMapping() {
  macropreset.deleteOutputMapping(editingMappingIndex);
  editingMappingIndex = -1;
  const mach = machinedefinitions.getMachineById(macropreset.machineId);
  if (mach) {
    macropreset.addMissingOutputMappings(mach);
  }
  outputMappingUpdate();
  updateJsonPreview();
}

async function handleUploadMacro() {
  // TODO: Implement upload macro logic here
  const id = macropreset.id;
  if (!id) {
    alert("Please enter an ID for the macro preset before uploading.");
    return;
  }

  const jsonstring = JSON.stringify(macropreset.serialize());
  const path = "macrodefinitions/" + id + ".json";
  updateSampleFile(path, jsonstring);
}

async function handleDeleteMacro() {
  // TODO: Implement delete macro logic here
  const id = macropreset.id;
  if (!id) {
    alert("Please enter an ID for the macro preset before deleting.");
    return;
  }

  if (
    !confirm(
      `Are you sure you want to delete the macro preset "${macropreset.name}"? This action cannot be undone.`,
    )
  ) {
    return;
  }

  const path = "macrodefinitions/" + id + ".json";
  deleteSampleFile(path);
}

async function handleActivateMacro() {
  const trackindex = selection.track;
  const machineid = macropreset.machineId;

  await putActiveMachineForTrack(trackindex, machineid);

  const jsonstring = JSON.stringify(macropreset.serialize());
  const path = "macrodefinitions/" + macropreset.id + ".json";
  await updateSampleFile(path, jsonstring);

  await putActiveMacroDefinitionForTrack(trackindex, macropreset.id ?? "");
}

export function outputMappingHandleClick(
  _evt: Event,
  ds: Record<string, string>,
) {
  if (ds.action === "editmapping") {
    handleSelectOutputMapping(~~ds.outputmapping);
  } else if (ds.action === "savemapping") {
    handleSaveMapping();
  } else if (ds.action === "deletemapping") {
    handleDeleteMapping();
  } else if (ds.action === "uploadmacro") {
    handleUploadMacro();
  } else if (ds.action === "deletemacro") {
    handleDeleteMacro();
  } else if (ds.action === "activatemacro") {
    handleActivateMacro();
  }
}

export function outputMappingUpdate() {
  updateMachineList();
  const el = document.querySelector("#outputmappinglist ul") as HTMLElement;
  el.innerHTML = "";
  for (const om of macropreset.outputMappings) {
    appendOutputMappingItem(el, om);
  }
}

export function outputMappingHandleChange(
  evt: Event,
  ds: Record<string, string>,
) {
  if (ds.selectfield === "targetdevice") {
    macropreset.machineId = getDropdownValue(evt.target as HTMLSelectElement);
    updateJsonPreview();
    const mach = machinedefinitions.getMachineById(macropreset.machineId);
    if (mach) {
      macropreset.addMissingOutputMappings(mach);
    }
    outputMappingUpdate();
    parametersUpdate();
    basicUpdate();
  }
}
