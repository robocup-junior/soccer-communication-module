# AI Handoff: RCJ Soccer Communication Module Firmware and Flasher

This file summarizes the recent work so another AI agent can continue without reconstructing the context.

## Repository Context

- Repository path: `/home/mato/VSCode/soccer-communication-module`
- Firmware project path: `firmware/RCj_comm_module`
- Current branch: `master`
- Current firmware target: `ESP32-C5`
- Current firmware version in `definitions.h`: `0.96`
- Current release tag: `fw-v0.96`
- Important user rule: do not run `git push` unless the user explicitly asks for it in that turn.

At the last check, `git status --porcelain` showed unrelated untracked files:

```text
?? ".readme_images/3D_PCB1_2026-05-21 v4.png"
?? ".readme_images/3D_PCB1_2026-05-21 v41.png"
?? ".readme_images/3D_PCB1_2026-05-21 v42.png"
?? docs/
```

These were not touched during the flasher and firmware work.

## Recent Commit History

Relevant commits currently visible in `git log --oneline --decorate -n 8`:

```text
da1b91d (HEAD -> master, tag: fw-v0.96, origin/master, origin/HEAD) Bump firmware version to 0.96
20db907 Improve ESP32-C5 flasher reset
51a724e Fix ESP32-C5 web flasher reset
254ee77 fix: make release workflow repo explicit
4a9e18e Use GitHub Actions Pages deployment
40b3c8b Publish flasher site to gh-pages branch
5faed82 (tag: fw-v0.95) Add firmware release workflow and web flasher
ca23261 Improve BLE command latency and resilience
```

Verify remote state before assuming the release has run. If the tag is not on GitHub yet, the user must explicitly authorize pushing:

```sh
git push origin master
git push origin fw-v0.96
```

Do not run those commands without explicit user approval.

## Firmware BLE Work

The play/stop critical path is not server notify. It is:

1. Mobile app writes to the BLE RX characteristic.
2. `ble.cpp` `onWrite()` queues the message.
3. `ble_processing.cpp` maps message IDs such as `BLE_MSG_PLAY` and `BLE_MSG_STOP`.
4. `state_machine.cpp` changes module state and output pins.

Main firmware changes:

- TX characteristic changed from indication to notification.
  - Current code uses `BLECharacteristic::PROPERTY_NOTIFY`.
  - `ble_send_msg()` calls `pTxCharacteristic->notify()`.
  - Reason: indications require client acknowledgement and can serialize traffic, which is bad for fast multi-module play/stop control.

- RX characteristic supports write without response.
  - Current code uses `BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR`.
  - Android/iOS apps should use write-without-response for fast play/stop bursts.

- BLE message queue is larger and handles bursts better.
  - `BLE_QUEUE_MAX_SIZE` is now `16`.
  - `queue_ble_msg()` drops the oldest queued message when full, then queues the newest message.

- BLE connection parameters are requested for NimBLE connections.
  - `BLE_CONN_INTERVAL_MIN = 12`, which is 15 ms.
  - `BLE_CONN_INTERVAL_MAX = 24`, which is 30 ms.
  - `BLE_CONN_LATENCY = 0`.
  - `BLE_CONN_TIMEOUT = 2000`, which is 20 s because units are 10 ms.
  - The 20 s timeout is intentional. Earlier aggressive timeout behavior caused modules to disconnect after missed pings or latency spikes. The user wants modules to stay connected as strongly as possible.

- BLE advertised name now includes the MAC-derived suffix.
  - Current define: `#define BLE_NAME "RCJs-m_" + BLE_MAC_to_string()`
  - Reason: iPhones do not expose MAC addresses, so the app can identify modules from the BLE name.

- Firmware version was bumped:
  - `FW_VERSION_MAJOR = 0`
  - `FW_VERSION_MINOR = 96`

## Mobile App Expectations

The mobile app should:

- Write play/stop commands to the RX characteristic, not wait on module notifications for this path.
- Use write-without-response for the RX characteristic.
- Send play/stop to all open `BluetoothGatt` connections without waiting for each device callback before moving to the next device.
- Avoid indication-based retry logic for play/stop.
- Optional robustness: send the same play/stop command twice with a 5-10 ms spacing.

## GitHub Actions and Release Flow

Workflow file:

```text
.github/workflows/firmware-release.yml
```

Current behavior:

- Trigger: pushed tags matching `fw-*` or `v*`.
- Firmware build uses `espressif/esp-idf-ci-action@v1`.
- ESP-IDF version: `v5.5.4`.
- Target: `esp32c5`.
- Project path: `firmware/RCj_comm_module`.
- Release preparation runs:

```sh
python ../../tools/prepare_firmware_release.py \
  --project-dir . \
  --web-src ../../web/flasher \
  --output-dir ../../dist \
  --version ${{ github.ref_name }}
```

The workflow:

1. Builds firmware.
2. Generates release assets and the Pages site under `dist/`.
3. Uploads the `firmware-release` artifact.
4. Creates or updates the GitHub Release.
5. Deploys the web flasher with modern GitHub Pages Actions:
   - `actions/configure-pages@v5`
   - `actions/upload-pages-artifact@v4`
   - `actions/deploy-pages@v4`

GitHub Pages must be configured to use GitHub Actions as the Pages source. The older `gh-pages` branch approach was discussed but superseded.

## Release Preparation Script

Script:

```text
tools/prepare_firmware_release.py
```

It:

- Reads `build/flasher_args.json`.
- Copies:
  - application binary
  - bootloader binary
  - partition table binary
- Merges binaries into a single flash image at offset `0x0` using `python -m esptool merge_bin`.
- Copies the static web flasher from `web/flasher`.
- Copies `.readme_images/modul_2024.png` into the flasher assets if present.
- Generates:
  - `manifest.json`
  - `version.json`
  - `release-notes.md`
- Includes flash settings in the manifest:
  - `flashMode`
  - `flashFreq`
  - `flashSize`

Known warning:

- With `esptool v5.2.0`, the current script emits deprecation warnings for `merge_bin`, `--flash_mode`, `--flash_freq`, and `--flash_size`.
- It still works. Future cleanup can switch to `merge-bin`, `--flash-mode`, `--flash-freq`, and `--flash-size`.

## Web Flasher

Files:

```text
web/flasher/index.html
web/flasher/app.js
web/flasher/styles.css
```

The flasher was changed from the ready-made `esp-web-install-button` to a custom Web Serial flasher because the standard button failed on ESP32-C5 native USB with:

```text
Installation failed
Failed to initialize. Try resetting your device or holding the BOOT button while clicking INSTALL.
```

The module does not expose BOOT/RESET buttons, and standard tools can enter the bootloader over native USB.

Current implementation:

- Imports `esptool-js@0.6.0` from:

```js
https://unpkg.com/esptool-js@0.6.0/bundle.js
```

- Uses Web Serial:

```js
const port = await navigator.serial.requestPort();
const transport = new Transport(port, false);
```

- Enters ESP32-C5 bootloader explicitly:

```js
await loader.main("usb_reset");
```

- Downloads `manifest.json` and `version.json`.
- Loads the merged firmware binary listed in the manifest.
- Flashes with:

```js
eraseAll: true
compress: true
```

- The UI no longer has an erase checkbox. It shows fixed text:

```text
Flash memory will be fully erased first.
```

Final reset:

- `loader.after("hard_reset", true)` did not reliably boot the new firmware.
- The user reported that flashing worked, but the module only booted after physically unplugging and reconnecting.
- Investigation found `esptool-js@0.6.0` hard reset behavior did not produce the needed full RTS pulse in this path.
- Current code manually resets over USB serial:

```js
const hardResetViaUsbSerial = async (transport) => {
  appendLog("Hard resetting via USB serial RTS pulse...");
  await transport.setDTR(false);
  await transport.setRTS(true);
  await sleep(200);
  await transport.setRTS(false);
  await sleep(200);
  await transport.setDTR(false);
};
```

The user physically tested this change and reported that the flasher works.

## Local Test Procedure

Local release generation was tested with:

```sh
GITHUB_REPOSITORY=robocup-junior/soccer-communication-module \
GITHUB_SHA=$(git rev-parse HEAD) \
/tmp/rcj-esptool-venv/bin/python tools/prepare_firmware_release.py \
  --project-dir firmware/RCj_comm_module \
  --web-src web/flasher \
  --output-dir /tmp/rcj-flasher-dist \
  --version fw-local-test
```

Local web server used for browser testing:

```sh
python3 -m http.server 8765 --directory /tmp/rcj-flasher-dist
```

Test URL:

```text
http://127.0.0.1:8765/
```

JavaScript syntax check:

```sh
node --input-type=module --check < web/flasher/app.js
```

The user confirmed physical flashing success after the manual RTS reset change.

## Remaining Risks and Follow-Up

- Do not push without explicit user instruction.
- Check whether `fw-v0.96` has actually been pushed to GitHub and whether the Actions release completed.
- If the GitHub release failed, inspect the `Firmware Release` workflow run first.
- If the flasher later fails to load, check CDN availability for `https://unpkg.com/esptool-js@0.6.0/bundle.js`.
- Consider vendoring `esptool-js` if offline or long-term release stability becomes important.
- Consider updating `prepare_firmware_release.py` to the non-deprecated esptool v5 CLI names.
- Verify the mobile app uses write-without-response and parallel fan-out for play/stop.
- Preserve the 20 s BLE supervision timeout unless the user explicitly asks for more aggressive disconnect detection.

