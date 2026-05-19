import { ESPLoader, Transport } from "https://unpkg.com/esptool-js@0.6.0/bundle.js";

const CHIP_FAMILY = "ESP32-C5";
const RESET_MODE = "usb_reset";
const BAUD_RATE = 115200;

const versionEl = document.querySelector("#firmware-version");
const chipEl = document.querySelector("#chip-family");
const sizeEl = document.querySelector("#firmware-size");
const hashEl = document.querySelector("#firmware-hash");
const releaseLink = document.querySelector("#release-link");
const installButton = document.querySelector("#install-button");
const flashStatus = document.querySelector("#flash-status");
const flashPercent = document.querySelector("#flash-percent");
const flashProgress = document.querySelector("#flash-progress");
const flashLog = document.querySelector("#flash-log");

let firmwareManifest = null;
let isFlashing = false;

const formatSize = (bytes) => {
  if (!Number.isFinite(bytes)) return "unknown";
  const units = ["B", "KB", "MB"];
  let value = bytes;
  let unit = 0;
  while (value >= 1024 && unit < units.length - 1) {
    value /= 1024;
    unit += 1;
  }
  return `${value.toFixed(unit === 0 ? 0 : 1)} ${units[unit]}`;
};

const loadJson = async (path) => {
  const response = await fetch(path, { cache: "no-store" });
  if (!response.ok) throw new Error(`${path}: HTTP ${response.status}`);
  return response.json();
};

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

const setStatus = (message, state = "idle") => {
  flashStatus.textContent = message;
  flashStatus.dataset.state = state;
};

const setProgress = (percent) => {
  const normalized = Math.max(0, Math.min(100, Math.round(percent)));
  flashProgress.value = normalized;
  flashPercent.textContent = `${normalized}%`;
};

const appendLog = (message, withNewline = true) => {
  flashLog.textContent += `${message}${withNewline ? "\n" : ""}`;
  flashLog.scrollTop = flashLog.scrollHeight;
};

const terminal = {
  clean() {
    flashLog.textContent = "";
  },
  writeLine(data) {
    appendLog(data);
  },
  write(data) {
    appendLog(data, false);
  },
};

const selectedBuild = () => {
  const builds = firmwareManifest?.builds || [];
  return builds.find((build) => build.chipFamily === CHIP_FAMILY) || builds[0];
};

const loadReleaseInfo = async () => {
  const [version, manifest] = await Promise.all([
    loadJson("version.json"),
    loadJson("manifest.json"),
  ]);

  firmwareManifest = manifest;

  const build = selectedBuild();
  const merged = version.files?.merged;

  versionEl.textContent = version.version || manifest.version || "unknown";
  chipEl.textContent = build?.chipFamily || version.chipFamily || CHIP_FAMILY;
  sizeEl.textContent = formatSize(merged?.size);
  hashEl.textContent = merged?.sha256 ? merged.sha256.slice(0, 16) : "unknown";

  if (version.releaseUrl) {
    releaseLink.href = version.releaseUrl;
    releaseLink.hidden = false;
  }
};

const loadFirmwareParts = async (build) => {
  if (!build?.parts?.length) {
    throw new Error("Firmware manifest does not contain any flash parts.");
  }

  const parts = [];
  for (const part of build.parts) {
    setStatus(`Downloading ${part.path}...`, "busy");
    const response = await fetch(part.path, { cache: "no-store" });
    if (!response.ok) throw new Error(`${part.path}: HTTP ${response.status}`);

    const address = Number(part.offset);
    if (!Number.isFinite(address)) {
      throw new Error(`Invalid flash offset for ${part.path}.`);
    }

    const data = new Uint8Array(await response.arrayBuffer());
    parts.push({
      data,
      address,
    });
  }

  return parts;
};

const assertExpectedChip = (loader, build) => {
  const connectedChip = loader.chip?.CHIP_NAME;
  const expectedChip = build.chipFamily || CHIP_FAMILY;

  if (connectedChip && connectedChip !== expectedChip) {
    throw new Error(`Connected chip is ${connectedChip}, expected ${expectedChip}.`);
  }
};

const explainError = (error) => {
  if (error?.name === "NotFoundError") return "No serial port was selected.";
  if (error?.name === "SecurityError") return "Serial access was blocked by the browser.";

  const message = error?.message || String(error);
  if (message.includes("Failed to connect")) {
    return "Failed to enter ESP32-C5 bootloader over USB. Reconnect the module and try again.";
  }

  return message;
};

const disconnectQuietly = async (transport) => {
  try {
    if (transport?.device?.readable || transport?.device?.writable) {
      await transport.disconnect();
    }
  } catch (error) {
    console.debug("Serial disconnect failed", error);
  }
};

const hardResetViaUsbSerial = async (transport) => {
  appendLog("Hard resetting via USB serial RTS pulse...");
  await transport.setDTR(false);
  await transport.setRTS(true);
  await sleep(200);
  await transport.setRTS(false);
  await sleep(200);
  await transport.setDTR(false);
};

const flashFirmware = async () => {
  if (isFlashing) return;
  isFlashing = true;
  installButton.disabled = true;
  setProgress(0);
  setStatus("Preparing...", "busy");

  let transport = null;

  try {
    if (!("serial" in navigator)) {
      throw new Error("Web Serial is not available in this browser.");
    }

    if (!firmwareManifest) {
      await loadReleaseInfo();
    }

    const build = selectedBuild();
    const parts = await loadFirmwareParts(build);
    const flashSettings = build.flashSettings || firmwareManifest.flashSettings || {};

    setStatus("Select the USB serial port...", "busy");
    const port = await navigator.serial.requestPort();
    transport = new Transport(port, false);
    transport.setDeviceLostCallback(() => {
      appendLog("Serial device disconnected.");
    });

    const loader = new ESPLoader({
      transport,
      baudrate: BAUD_RATE,
      serialOptions: { bufferSize: 65536 },
      terminal,
      debugLogging: false,
      enableTracing: false,
    });

    setStatus("Connecting with USB reset...", "busy");
    await loader.main(RESET_MODE);
    assertExpectedChip(loader, build);

    setStatus("Flashing firmware...", "busy");
    await loader.writeFlash({
      fileArray: parts,
      flashMode: flashSettings.flashMode || "keep",
      flashFreq: flashSettings.flashFreq || "keep",
      flashSize: flashSettings.flashSize || "keep",
      eraseAll: true,
      compress: true,
      reportProgress: (_fileIndex, written, total) => {
        setProgress(total > 0 ? (written / total) * 100 : 0);
      },
    });

    setStatus("Resetting module...", "busy");
    try {
      await hardResetViaUsbSerial(transport);
    } catch (resetError) {
      appendLog(`Reset after flashing failed: ${explainError(resetError)}`);
    }

    setProgress(100);
    setStatus("Installation complete", "success");
  } catch (error) {
    console.error(error);
    appendLog(`Error: ${explainError(error)}`);
    setStatus("Installation failed", "error");
  } finally {
    await disconnectQuietly(transport);
    installButton.disabled = false;
    isFlashing = false;
  }
};

if (!("serial" in navigator)) {
  installButton.disabled = true;
  setStatus("Web Serial unavailable", "error");
}

installButton.addEventListener("click", flashFirmware);

try {
  await loadReleaseInfo();
} catch (error) {
  console.error(error);
  versionEl.textContent = "unavailable";
  sizeEl.textContent = "unavailable";
  hashEl.textContent = "unavailable";
  setStatus("Firmware metadata unavailable", "error");
}
