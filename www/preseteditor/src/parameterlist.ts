import { basicUpdate, updateJsonPreview } from "./basicprops";
import { getDropdownValue, replaceDropdown } from "./domutils";
import type {
  MacroPresetParameter,
  MacroPresetParameterGroup,
} from "./macropreset";
import { outputMappingUpdate } from "./outputmappinglist";
import { previewUpdate } from "./preview";
import { machinedefinitions, macropreset, soundpreset } from "./state";

let editingParameterIndex = -1;

function appendParameterEditor(
  parent: HTMLElement,
  parameter: MacroPresetParameter,
) {
  const el3 = document.createElement("li");
  el3.classList.add("parameter");
  el3.classList.add("editor");
  el3.classList.add("editing");
  el3.dataset.parameter = `${parameter.index}`;
  el3.innerHTML = `
        <p>Name: <input type="text" data-inputfield="parametername" value="" /></p>
        <p>Default value: <input type="number" data-inputfield="parameterdefault" value="" /></p>
        <p>Range: <input type="number" data-inputfield="parameterrangemin" value="" />-<input type="number" data-inputfield="parameterrangemax" value="" /></p>
        <p>Resolution: <input type="number" data-inputfield="parameterresolution" value="" /></p>
        <p>Presentation: <select data-inputfield="parameterpresentation"></select></p>
        <p><button data-action="saveparameter">Save</button></p>
    `;

  let el4;

  el4 = el3.querySelector(
    'input[data-inputfield="parametername"]',
  ) as HTMLInputElement;
  if (el4) {
    el4.value = parameter.name;
  }

  el4 = el3.querySelector(
    'input[data-inputfield="parameterdefault"]',
  ) as HTMLInputElement;
  if (el4) {
    el4.value = `${parameter.defaultValue}`;
  }

  el4 = el3.querySelector(
    'input[data-inputfield="parameterrangemin"]',
  ) as HTMLInputElement;
  if (el4) {
    el4.value = `${parameter.minValue}`;
  }

  el4 = el3.querySelector(
    'input[data-inputfield="parameterrangemax"]',
  ) as HTMLInputElement;
  if (el4) {
    el4.value = `${parameter.maxValue}`;
  }

  el4 = el3.querySelector(
    'input[data-inputfield="parameterresolution"]',
  ) as HTMLInputElement;
  if (el4) {
    el4.value = `${parameter.resolution}`;
  }

  const presentations = [
    {
      value: "bignum",
      label: "Big number",
    },
    {
      value: "freq",
      label: "Frequency",
    },
    {
      value: "curve1",
      label: "Curve 1",
    },
  ];
  el4 = el3.querySelector(
    'select[data-inputfield="parameterpresentation"]',
  ) as HTMLSelectElement;
  if (el4) {
    replaceDropdown(el4, presentations, parameter.presentation);
  }

  parent.appendChild(el3);
}

function appendParameterItem(
  parent: HTMLElement,
  parameter: MacroPresetParameter,
) {
  const el2 = document.createElement("li");
  el2.classList.add("parameter");
  el2.dataset.parameter = `${parameter.index}`;
  // Param #<span data-field="id"></span>
  el2.innerHTML = `
        <b data-field="name">?</b>
        <span>
            Index: <span data-field="index"></span> -
            Default: <span data-field="defaultvalue"></span> -
            Range: <span data-field="rangemin"></span>-<span data-field="rangemax"></span> -
            Resolution: <span data-field="resolution"></span><br />
        </span>
    `;
  let el4;
  el4 = el2.querySelector('b[data-field="name"]');
  if (el4) {
    el4.textContent = parameter.name;
  }
  el4 = el2.querySelector('span[data-field="defaultvalue"]');
  if (el4) {
    el4.textContent = `${parameter.defaultValue}`;
  }
  el4 = el2.querySelector('span[data-field="index"]');
  if (el4) {
    el4.textContent = `${parameter.index}`;
  }
  el4 = el2.querySelector('span[data-field="rangemin"]');
  if (el4) {
    el4.textContent = `${parameter.minValue}`;
  }
  el4 = el2.querySelector('span[data-field="rangemax"]');
  if (el4) {
    el4.textContent = `${parameter.maxValue}`;
  }
  el4 = el2.querySelector('span[data-field="resolution"]');
  if (el4) {
    el4.textContent = `${parameter.resolution}`;
  }

  el2.dataset.action = "editparameter";

  parent.appendChild(el2);
}

function appendParameterGroup(
  parent: HTMLElement,
  parametergroup: MacroPresetParameterGroup,
  _index: number,
) {
  const el2 = document.createElement("div");
  el2.classList.add("paramgroup");
  el2.dataset.parametergroup = `${parametergroup.index}`;
  el2.innerHTML = `
        <p>
        Page name: <input type="text" data-inputfield="groupname" value="" />
        </p>
        <ul></ul>
    `;

  (
    el2.querySelector('input[data-inputfield="groupname"]') as HTMLInputElement
  ).value = parametergroup.name;

  const list = el2.querySelector("ul") as HTMLElement;
  if (parametergroup.parameters.length === 0) {
    list.innerHTML = "<li><i>No parameters yet</i></li>";
  }
  for (const p of parametergroup.parameters) {
    if (p.index === editingParameterIndex) {
      appendParameterEditor(list, p);
    } else {
      appendParameterItem(list, p);
    }
  }

  parent.appendChild(el2);
}

function handleMoveParameter(_id: string, _direction: number) {
  // editingMappingIndex = id;
  // recreateOutputMappingList();
}

function handleSelectParameter(idx: number) {
  editingParameterIndex = idx;
  parametersUpdate();
}

function handleSaveParameter() {
  const root = document.querySelector(
    `*[data-parameter="${editingParameterIndex}"]`,
  ) as HTMLElement;

  const nameInput = root.querySelector(
    'input[data-inputfield="parametername"]',
  ) as HTMLInputElement;
  const defaultValueInput = root.querySelector(
    'input[data-inputfield="parameterdefault"]',
  ) as HTMLInputElement;
  const minValueInput = root.querySelector(
    'input[data-inputfield="parameterrangemin"]',
  ) as HTMLInputElement;
  const maxValueInput = root.querySelector(
    'input[data-inputfield="parameterrangemax"]',
  ) as HTMLInputElement;
  const resolutionInput = root.querySelector(
    'input[data-inputfield="parameterresolution"]',
  ) as HTMLInputElement;
  const presentationInput = getDropdownValue(
    root.querySelector(
      'select[data-inputfield="parameterpresentation"]',
    ) as HTMLSelectElement,
  );

  macropreset.updateParameter(editingParameterIndex, (param) => {
    param.name = nameInput.value;
    param.defaultValue = ~~defaultValueInput.value;
    param.minValue = ~~minValueInput.value;
    param.maxValue = ~~maxValueInput.value;
    param.resolution = ~~resolutionInput.value;
    param.presentation = presentationInput;
  });

  editingParameterIndex = -1;
  parametersUpdate();
  basicUpdate();
  previewUpdate();
}

export function handleCreateOnttoOneMapping() {
  console.log("Creating 1:1 mapping parameters");

  const machmap = machinedefinitions.getMachineById(macropreset.machineId);
  if (!machmap) {
    console.error("Machine definition not found for id", macropreset.machineId);
    return;
  }

  macropreset.id = `${machmap.id}-allparams`;
  macropreset.name = `${machmap.name} All param`;
  macropreset.parameterGroups = [];
  macropreset.addMissingSlots();

  for (const [pidx, p] of machmap.parameters.entries()) {
    console.log("Adding parameter", p, pidx);
    const gidx = Math.floor(pidx / 4);
    const pidx2 = pidx % 4;

    macropreset.parameterGroups[gidx].name = "Page " + (gidx + 1);

    const p2 = macropreset.parameterGroups[gidx].parameters[pidx2];
    p2.name = p.name;
    p2.minValue = 0;
    p2.maxValue = 127;
    p2.defaultValue = p.def;
    p2.resolution = 64;
    p2.presentation = "bignum";

    macropreset.outputMappings[pidx].ctrl = p.ctrl;
    macropreset.outputMappings[pidx].startvalue = 0;
    macropreset.outputMappings[pidx].sources = [
      {
        source: pidx,
        div: 1,
        mul: 1,
      },
    ];
  }

  soundpreset.id = `${machmap.id}-all-def`;
  soundpreset.name = `${machmap.name} All default`;
  soundpreset.macroId = macropreset.id;

  basicUpdate();
  parametersUpdate();
  outputMappingUpdate();
  previewUpdate();
  updateJsonPreview();
}

export function parametersHandleClick(_evt: Event, ds: Record<string, string>) {
  if (ds.action === "editparameter") {
    handleSelectParameter(~~ds.parameter);
  } else if (ds.action === "saveparameter") {
    handleSaveParameter();
  } else if (ds.action === "moveup") {
    handleMoveParameter(ds.parameter, -1);
  } else if (ds.action === "movedown") {
    handleMoveParameter(ds.parameter, 1);
  } else if (ds.action === "createontoonemapping") {
    handleCreateOnttoOneMapping();
  }
}

export function parametersUpdate() {
  const el = document.getElementById("parametergrouplist") as HTMLElement;
  console.log(
    "Recreating audio parameter list",
    macropreset.parameterGroups,
    editingParameterIndex,
  );

  el.innerHTML = "";
  for (const [index, pg] of macropreset.parameterGroups.entries()) {
    appendParameterGroup(el, pg, index);
  }
}

export function parameterHandleChange(evt: Event, ds: Record<string, string>) {
  if (ds.inputfield === "groupname") {
    macropreset.updateGroupName(
      ~~ds.parametergroup,
      (evt.target as HTMLInputElement).value,
    );
    basicUpdate();
    previewUpdate();
  }
}
