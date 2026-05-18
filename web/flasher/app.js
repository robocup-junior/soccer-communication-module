(async () => {
  const versionEl = document.querySelector("#firmware-version");
  const chipEl = document.querySelector("#chip-family");
  const sizeEl = document.querySelector("#firmware-size");
  const hashEl = document.querySelector("#firmware-hash");
  const releaseLink = document.querySelector("#release-link");

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

  try {
    const response = await fetch("version.json", { cache: "no-store" });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const release = await response.json();
    const merged = release.files?.merged;

    versionEl.textContent = release.version || "unknown";
    chipEl.textContent = release.chipFamily || "ESP32-C5";
    sizeEl.textContent = formatSize(merged?.size);
    hashEl.textContent = merged?.sha256 ? merged.sha256.slice(0, 16) : "unknown";

    if (release.releaseUrl) {
      releaseLink.href = release.releaseUrl;
      releaseLink.hidden = false;
    }
  } catch (error) {
    versionEl.textContent = "unavailable";
    sizeEl.textContent = "unavailable";
    hashEl.textContent = "unavailable";
  }
})();
