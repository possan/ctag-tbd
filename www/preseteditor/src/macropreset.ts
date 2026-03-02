import type {
  ISerializedMachineDefinitionMachine,
  ISerializedMacroPreset,
  ISerializedMacroPresetOutputMapping,
  ISerializedMacroPresetOutputMappingSource,
  ISerializedMacroPresetParameterGroup,
  ISerializedMacroPresetParameterGroupParameter,
} from "./types";

export interface MacroPresetOutputMappingSource {
  source: number;
  mul: number;
  div: number;
}

export interface MacroPresetOutputMapping {
  index: number;
  ctrl: number;
  startvalue: number;
  sources: MacroPresetOutputMappingSource[];
}

export interface MacroPresetParameter {
  index: number;
  name: string;
  defaultValue: number;
  minValue: number;
  maxValue: number;
  resolution: number;
  presentation: string;
}

export interface MacroPresetParameterGroup {
  index: number;
  name: string;
  parameters: MacroPresetParameter[];
}

export class MacroPreset {
  id = "";
  machineId = "";
  name = "";
  parameterGroups: MacroPresetParameterGroup[] = [];
  outputMappings: MacroPresetOutputMapping[] = [];

  constructor() {
    this.reset();
  }

  reset() {
    this.machineId = "";
    this.name = "";
    this.parameterGroups = [];
    this.outputMappings = [];
  }

  updateGroupName(group: number, name: string) {
    this.parameterGroups[group].name = name;
  }

  addMissingSlots() {
    let gindex = this.parameterGroups.length;
    while (this.parameterGroups.length < 6) {
      this.parameterGroups.push({
        index: gindex,
        name: "",
        parameters: [],
      });
      gindex++;
    }

    let index = 0;
    for (const g of this.parameterGroups) {
      index += g.parameters.length;
      while (g.parameters.length < 4) {
        g.parameters.push({
          index: index,
          name: "",
          defaultValue: 0,
          minValue: 0,
          maxValue: 127,
          resolution: 32,
          presentation: "bignumber",
        });
        index++;
      }
    }
  }

  addMissingOutputMappings(
    machinedefinition: ISerializedMachineDefinitionMachine,
  ) {
    console.log("addMissingOutputMappings", machinedefinition);

    for (const mp of machinedefinition.parameters) {
      const existingMapping = this.outputMappings.find(
        (om) => om.ctrl === mp.ctrl,
      );
      console.log(
        "checking if anything maps to",
        mp,
        "existing match:",
        existingMapping,
      );
      if (!existingMapping) {
        this.outputMappings.push({
          index: this.outputMappings.length,
          ctrl: mp.ctrl,
          startvalue: mp.def || 0,
          sources: [],
        });
      }
    }

    for (const om of this.outputMappings) {
      const mp = machinedefinition.parameters.find((p) => p.ctrl === om.ctrl);
      if (!mp) {
        om.ctrl = 0;
        om.index = 0;
        om.sources = [];
        om.startvalue = 0;
      }
    }

    this.outputMappings = this.outputMappings.filter((om) => om.ctrl !== 0);

    this.outputMappings.sort((a, b) => {
      const aidx = a.ctrl;
      const bidx = b.ctrl;
      return (aidx == -1 ? 99999 : aidx) - (bidx == -1 ? 99999 : bidx);
    });

    this.addMissingSlots();
  }

  setParameterName(index: number, name: string) {
    for (const g of this.parameterGroups) {
      for (const p of g.parameters) {
        if (p.index === index) {
          p.name = name;
          return;
        }
      }
    }
  }

  updateParameter(index: number, editfn: (p: MacroPresetParameter) => void) {
    for (const g of this.parameterGroups) {
      for (const p of g.parameters) {
        if (p.index === index) {
          editfn(p);
        }
      }
    }
  }

  updateOutputMapping(
    index: number,
    editfn: (om: MacroPresetOutputMapping) => void,
  ) {
    const om = this.outputMappings[index];
    editfn(om);
  }

  deleteOutputMapping(index: number) {
    this.outputMappings.splice(index, 1);
  }

  getParameterIdForIndex(index: number): number {
    let currentIndex = 0;
    for (const g of this.parameterGroups) {
      for (const p of g.parameters) {
        if (currentIndex === index) {
          return p.index;
        }
        currentIndex++;
      }
    }
    return -1;
  }

  getParameterIndexForId(index: number): number {
    for (const g of this.parameterGroups) {
      for (const p of g.parameters) {
        if (p.index === index && p.name) {
          return index;
        }
      }
    }
    return -1;
  }

  detect(jsonroot: ISerializedMacroPreset) {
    if (!jsonroot) {
      return false;
    }

    if (!jsonroot.id || !jsonroot.name || !jsonroot.machine) {
      return false;
    }

    if (jsonroot.groups === undefined || !Array.isArray(jsonroot.groups)) {
      return false;
    }

    if (jsonroot.mapping === undefined || !Array.isArray(jsonroot.mapping)) {
      return false;
    }

    return true;
  }

  deserialize(jsonroot: ISerializedMacroPreset) {
    if (jsonroot.id) {
      this.id = jsonroot.id;
    }
    if (jsonroot.name) {
      this.name = jsonroot.name;
    }
    if (jsonroot.machine) {
      this.machineId = jsonroot.machine;
    }
    if (jsonroot.groups) {
      this.parameterGroups = jsonroot.groups.map((g, gindex) => ({
        index: gindex,
        name: g.name,
        parameters: g.parameters.map((p, pindex) => ({
          index: pindex + gindex * 4,
          name: p.name,
          defaultValue: p.def,
          minValue: p.min || 0,
          maxValue: p.max || 0,
          resolution: p.res,
          presentation: p.ui || "",
        })),
      }));
    }
    if (jsonroot.mapping) {
      this.outputMappings = jsonroot.mapping.map((m, midx) => ({
        index: midx,
        ctrl: m.ctrl,
        startvalue: m.start,
        sources: (m.add || []).map((s) => {
          return {
            source: this.getParameterIdForIndex(s.src),
            mul: s.mul || 1,
            div: s.div || 1,
          };
        }),
      }));
    }
    this.addMissingSlots();
    return true;
  }

  serialize(): ISerializedMacroPreset {
    let groups: ISerializedMacroPresetParameterGroup[] = [];

    for (const g of this.parameterGroups) {
      let parameters: ISerializedMacroPresetParameterGroupParameter[] = [];
      for (const p of g.parameters) {
        if (p.name) {
          parameters.push({
            idx: p.index, // this.getParameterIndexForId(p.id),
            name: p.name,
            def: p.defaultValue,
            min: p.minValue,
            max: p.maxValue,
            res: p.resolution,
            ui: p.presentation,
          });
        }
      }

      if (parameters.length > 0) {
        groups.push({
          // id: g.id,
          name: g.name,
          parameters,
        });
      }
    }

    const mapping: ISerializedMacroPresetOutputMapping[] = [];

    for (const om of this.outputMappings) {
      if (om.sources && om.sources.length > 0) {
        const add: ISerializedMacroPresetOutputMappingSource[] = [];
        for (const os of om.sources || []) {
          const src = this.getParameterIndexForId(os.source);
          if (src >= 0) {
            add.push({
              src,
              mul: os.mul,
              div: os.div,
            });
          }
        }

        mapping.push({
          ctrl: om.ctrl,
          start: om.startvalue,
          add,
        });
      } else {
        mapping.push({
          ctrl: om.ctrl,
          start: om.startvalue,
          add: undefined,
        });
      }
    }

    return {
      id: this.id,
      name: this.name,
      machine: this.machineId,
      groups,
      mapping,
    };
  }
}
