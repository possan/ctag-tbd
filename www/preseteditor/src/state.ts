import { MachineDefinitions } from "./machinedefinition";
import { MacroPreset } from "./macropreset";
import { SoundPreset } from "./soundpreset";
import type { StateCache } from "./types";

export let cache: StateCache = {};
export const machinedefinitions = new MachineDefinitions();
export const macropreset = new MacroPreset();
export const soundpreset = new SoundPreset();
export const selection = {
  track: 0,
  macro: "",
  soundpreset: "",
};

export async function loadCache() {
  let tmp = localStorage.getItem("cache");
  if (!tmp) {
    return;
  }

  try {
    tmp = JSON.parse(tmp);
  } catch (err) {
    console.warn("Failed to load cache", err);
  }

  if (tmp) {
    cache = tmp as StateCache;
  }
}

export async function saveCache() {
  try {
    localStorage.setItem("cache", JSON.stringify(cache));
  } catch (err) {
    console.warn("Failed to save cache", err);
  }
}
