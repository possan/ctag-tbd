import type {
  ISerializedMachineDefinition,
  ISerializedMachineDefinitionMachine,
  ISerializedMachineDefinitionTrack,
} from "./types";

export class MachineDefinitions implements ISerializedMachineDefinition {
  machines: ISerializedMachineDefinitionMachine[] = [];
  tracks: ISerializedMachineDefinitionTrack[] = [];

  constructor() {}

  deserialize(jsonroot: ISerializedMachineDefinition) {
    this.machines = jsonroot.machines || [];
    this.tracks = jsonroot.tracks || [];
    return true;
  }

  getMachineById(id: string): ISerializedMachineDefinitionMachine | undefined {
    return this.machines.find((m) => m.id === id);
  }
}
