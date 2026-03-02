import type { MacroPreset } from "./macropreset";
import type { ISerializedSoundPreset } from "./types";

export class SoundPresetParameter {
  index = 0;
  value = 0;
}

export class SoundPreset {
  id = "";
  name = "";
  group = "";
  macroId = "";
  parameters: SoundPresetParameter[] = [];

  constructor() {
    this.reset();
  }

  reset() {
    this.id = "";
    this.name = "";
    this.group = "";
    this.macroId = "";
    this.parameters = [];
  }

  detect(jsonroot: ISerializedSoundPreset) {
    if (!jsonroot) {
      return false;
    }
    if (!jsonroot.id || !jsonroot.name || !jsonroot.group || !jsonroot.macro) {
      return false;
    }

    if (jsonroot.values === undefined || !Array.isArray(jsonroot.values)) {
      return false;
    }

    return true;
  }

  deserialize(jsonroot: ISerializedSoundPreset) {
    if (jsonroot.id) {
      this.id = jsonroot.id;
    }
    if (jsonroot.name) {
      this.name = jsonroot.name;
    }
    if (jsonroot.group) {
      this.group = jsonroot.group;
    }
    if (jsonroot.macro) {
      this.macroId = jsonroot.macro;
    }
    if (jsonroot.values) {
      for (var k = 0; k < jsonroot.values.length; k++) {
        const param = new SoundPresetParameter();
        param.index = k;
        param.value = jsonroot.values[k];
        this.parameters.push(param);
      }
    }
    return true;
  }

  serialize(macropreset: MacroPreset): ISerializedSoundPreset {
    const values = [];
    let maxidx = 0;
    for (var k = 0; k < this.parameters.length; k++) {
      const index = this.parameters[k].index;
      const idx = macropreset.getParameterIndexForId(index);
      maxidx = Math.max(maxidx, idx);
      while (values.length < maxidx) {
        values.push(-1);
      }
      const p = this.parameters.find((p) => p.index === index);
      values[idx] = p?.value ?? -1;
    }

    return {
      id: this.id,
      name: this.name,
      group: this.group,
      macro: this.macroId,
      values,
    };
  }

  recreateParameters(macropreset: MacroPreset) {
    let validIndices = [];
    for (const pg of macropreset.parameterGroups) {
      for (const p2 of pg.parameters) {
        validIndices.push(p2.index);
      }
    }
    this.parameters = this.parameters.filter((p2) =>
      validIndices.includes(p2.index),
    );
    for (const pg of macropreset.parameterGroups) {
      for (const p of pg.parameters) {
        const sp = this.parameters.find((sp) => sp.index === p.index);
        if (!sp) {
          this.parameters.push({
            index: p.index,
            value: p.defaultValue || 0,
          });
        }
      }
    }
  }

  resetToMacroParameters(macropreset: MacroPreset) {
    for (const pg of macropreset.parameterGroups) {
      for (const p of pg.parameters) {
        const sp = this.parameters.find((sp) => sp.index === p.index);
        if (sp) {
          sp.value = p.defaultValue || 0;
        }
      }
    }
  }

  randomizeParameters(macropreset: MacroPreset) {
    for (const pg of macropreset.parameterGroups) {
      for (const p of pg.parameters) {
        const sp = this.parameters.find((sp) => sp.index == p.index);
        if (sp) {
          if (p.minValue || p.maxValue) {
            sp.value =
              p.minValue +
              Math.floor(Math.random() * (p.maxValue - p.minValue + 1));
          } else {
            sp.value = 0;
          }
        }
      }
    }
  }

  setParameterValue(paramindex: number, value: number) {
    const p = this.parameters.find((p) => p.index === paramindex);
    if (p) {
      p.value = value;
    }
  }
}
