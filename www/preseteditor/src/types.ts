export interface ISerializedMachineDefinitionMachineParameter {
  name: string;
  type: string;
  ctrl: number;
  def: number;
}

export interface ISerializedMachineDefinitionMachine {
  id: string;
  name: string;
  type: string;
  parameters: ISerializedMachineDefinitionMachineParameter[];
}

export interface ISerializedMachineDefinitionTrack {
  index: number;
  type: string;
  name: string;
  midichannel: number;
  drumnote: number;
  basecc: number;
  machines: string[];
  defaultbank?: string; // for romplers
}

export interface ISerializedMachineDefinition {
  tracks: ISerializedMachineDefinitionTrack[];
  machines: ISerializedMachineDefinitionMachine[];
}

export interface ISerializedSoundPreset {
  id: string;
  name: string;
  group: string;
  macro: string;
  values: number[];
}

export interface ISerializedMacroPresetParameterGroupParameter {
  idx: number;
  name: string;
  def: number;
  min: number;
  max: number;
  res: number;
  ui: string | undefined;
}

export interface ISerializedMacroPresetParameterGroup {
  name: string;
  parameters: ISerializedMacroPresetParameterGroupParameter[];
}

export interface ISerializedMacroPresetOutputMappingSource {
  src: number;
  mul: number;
  div: number;
}

export interface ISerializedMacroPresetOutputMapping {
  ctrl: number;
  start: number;
  add: ISerializedMacroPresetOutputMappingSource[] | undefined;
}

export interface ISerializedMacroPreset {
  id: string;
  name: string;
  machine: string;
  groups: ISerializedMacroPresetParameterGroup[];
  mapping: ISerializedMacroPresetOutputMapping[];
}

export interface StateCache {
  synthdefinition?: ISerializedMachineDefinition;
  macrodefinitions?: ISerializedMacroPreset[];
  soundpresets?: ISerializedSoundPreset[];
}
