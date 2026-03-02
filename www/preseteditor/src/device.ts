// use device ip if running on localhost, otherwise assume same origin
let ROOT =
  window.location.href.indexOf("localhost") != -1
    ? "http://192.168.4.1/api/v1"
    : "/api/v1";

async function delay(ms: number) {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

export async function updateSampleFile(path: string, jsonstring: string) {
  try {
    const response = await fetch(
      `${ROOT}/samples?action=uploadconfig&path=${path}`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: jsonstring,
      },
    );
    await delay(100);
    return response.ok;
  } catch (e) {
    console.warn("err", e);
    return false;
  }
}

export async function deleteSampleFile(path: string) {
  await fetch(`${ROOT}/samples?action=manage`, {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify({
      action: "deleteconfig",
      path: `${path}`,
    }),
  });
  await delay(100);
}

export async function fetchSamplesResponse() {
  try {
    const response = await fetch(`${ROOT}/samples`);
    const data = await response.json();
    await delay(100);
    return data;
  } catch (e) {
    console.warn("err", e);
    return undefined;
  }
}

export async function fetchSynthDefinitionConfig() {
  try {
    const response = await fetch(
      `${ROOT}/samples?getconfig=synthdefinitions.json`,
    );
    const data = await response.json();
    await delay(100);
    return data;
  } catch (e) {
    console.warn("err", e);
    return undefined;
  }
}

export async function fetchSamplesFile(path: string) {
  try {
    const response = await fetch(`${ROOT}/samples?getconfig=${path}`);
    const data = await response.json();
    await delay(100);
    return data;
  } catch (e) {
    console.warn("err", e);
    return undefined;
  }
}

export async function refreshDeviceConfig() {
  try {
    await fetch(`${ROOT}/macroapi?action=reload`, {
      method: "POST",
    });
    await delay(100);
  } catch (e) {
    console.warn("err", e);
  }
}

export async function putActiveMachineForTrack(
  trackindex: number,
  machineid: string,
) {
  try {
    const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ track: trackindex, machine: machineid }),
    });
    await delay(100);
    return response.ok;
  } catch (e) {
    console.warn("err", e);
    return false;
  }
}

export async function putActiveMacroDefinitionForTrack(
  trackindex: number,
  macroid: string,
) {
  try {
    const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({ track: trackindex, macro: macroid }),
    });
    await delay(100);
    return response.ok;
  } catch (e) {
    console.warn("err", e);
    return false;
  }
}

export async function updateLiveParametersForTrack(
  trackindex: number,
  parameters: number[],
) {
  try {
    const response = await fetch(`${ROOT}/macroapi?action=update_track`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        track: trackindex,
        parameters: parameters,
      }),
    });
    await delay(100);
    return response.ok;
  } catch (e) {
    console.warn("err", e);
    return false;
  }
}
