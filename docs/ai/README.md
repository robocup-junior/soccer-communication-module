# AI Agent Documentation — RCJ Soccer Communication Module

This folder is the **entry point for AI agents** (and humans) who need to understand
and later safely modify this project. It was written by inspecting the working tree,
source code, git history, build/release tooling, the web flasher, and the new-hardware
3D render images. Where something could not be proven from the repository it is marked
as **inferred**, **unknown**, or **to verify from schematic**.

> ⚠️ The public top-level `README.md` and the `RCJ 2024 communication modules.pdf` are
> useful but **partly outdated**. They describe the older 2024 hardware (UART/RX-TX,
> A0/A1 channel pins, LOGV) and an Arduino-style workflow. The firmware in this repo now
> targets **ESP32-C5** and is built with **ESP-IDF**. Treat this `docs/ai/` folder and
> the live source code as the source of truth over the public docs.

> 📌 **Also read [`firmware/RCj_comm_module/AI_HANDOFF.md`](../../firmware/RCj_comm_module/AI_HANDOFF.md).**
> It is the handoff note written by the previous single-agent session that did the BLE
> latency, release-workflow, and web-flasher work. It has been cross-checked against the
> code and is **accurate and consistent with this docs set**; it adds the *rationale*
> behind several decisions (the since-revised 20 s BLE timeout, MAC-in-name for iPhones, NOTIFY
> over INDICATE, the manual USB RTS reset) and the **mobile-app expectations**. Key user
> rule from it: **never `git push` unless the user explicitly asks in that turn.**

## Project summary

A small board that mounts on a RoboCupJunior Soccer robot. It connects over **BLE** to a
mobile referee/control app (Nordic UART Service profile). The app sends match commands
(`PLAY`, `STOP`, penalty/damage, halftime, game over, score, name). The firmware drives
two digital **output pins** (`OUT1`/`OUT2`) that the robot reads to start/stop, and shows
the current match state on a 128×64 **SSD1306 OLED** (including a QR code of the BLE MAC
while waiting for a connection).

## Current status (as of fw v0.97)

- Firmware version **0.97** (`definitions.h`: `FW_VERSION_MAJOR=0`, `FW_VERSION_MINOR=97`),
  not yet tagged/released (latest released tag is `fw-v0.96`). v0.97 (working tree) bundles:
  `BLE_CONN_TIMEOUT` 20 s → 3 s (faster reconnect + faster fail-safe stop), and a clean
  release UART (bootloader/IDF logs OFF, console on UART0) with a `sdkconfig.debug` overlay
  for USB-C logging during development.
- Target chip: **ESP32-C5**, ESP-IDF **v5.5.4**, `espressif/arduino-esp32` `^3.3.0`, **NimBLE**.
- Runs on the new ESP32-C5 hardware in **legacy-compatibility mode**: same behavior as the
  old module, only with remapped pins. UART/`Serial` is still used (debug prints + legacy).
- New hardware (silkscreen **"V7 2026"**, branding "robofuze") adds RGB LED, buzzer,
  supercapacitor backup, three buttons, ON/OFF switch, and USB-C direct programming.
  **None of the RGB LED / buzzer / 3rd-button / supercap features are implemented in
  firmware yet.**
- Release/flashing is automated: Git tag → GitHub Actions → GitHub Release + GitHub Pages
  web flasher (Web Serial / esptool-js, ESP32-C5 USB reset path).

## Where to start reading

| Order | Document | Topic |
|-------|----------|-------|
| 1 | [00_project_overview.md](00_project_overview.md) | What the module does, old vs new HW |
| 2 | [01_repository_map.md](01_repository_map.md) | Folder/file layout, source vs generated |
| 3 | [02_firmware_architecture.md](02_firmware_architecture.md) | Modules, main loop, RTOS queue |
| 4 | [03_ble_protocol.md](03_ble_protocol.md) | UUIDs, message IDs, payloads, direction |
| 5 | [04_state_machine.md](04_state_machine.md) | States, transitions, output behavior |
| 6 | [05_hardware_mapping.md](05_hardware_mapping.md) | Pin map (C5 + legacy C6), to-verify |
| 7 | [06_display_and_user_feedback.md](06_display_and_user_feedback.md) | Screens, proposed LED/buzzer |
| 8 | [07_build_release_and_flasher.md](07_build_release_and_flasher.md) | Build, CI, web flasher |
| 9 | [08_mobile_app_interaction.md](08_mobile_app_interaction.md) | App behavior, command mapping |
| 10 | [09_known_issues_and_open_questions.md](09_known_issues_and_open_questions.md) | Risks, TODOs, unknowns |
| 11 | [10_future_change_plan.md](10_future_change_plan.md) | Staged plan for new HW features |
| 12 | [11_legacy_c6_firmware.md](11_legacy_c6_firmware.md) | Final legacy ESP32-C6 firmware: git layout, build, flashing, release plan |

## Most important source files

| File | Role |
|------|------|
| `firmware/RCj_comm_module/RCj_comm_module.ino` | Arduino-IDE entry point (`setup`/`loop`) |
| `firmware/RCj_comm_module/main/app_main.cpp` | ESP-IDF entry point (`app_main`), mirrors the `.ino` |
| `firmware/RCj_comm_module/definitions.h` | Versions, BLE/UART constants, **GPIO pin map** |
| `firmware/RCj_comm_module/ble.cpp` / `.h` | BLE server, UUIDs, RX/TX characteristics, callbacks |
| `firmware/RCj_comm_module/ble_processing.cpp` / `.h` | Message IDs enum, queue, command dispatch |
| `firmware/RCj_comm_module/state_machine.cpp` / `.h` | States, output pin control, timers |
| `firmware/RCj_comm_module/display.cpp` / `.h` | OLED screen rendering |
| `firmware/RCj_comm_module/functions.cpp` / `.h` | MAC, score/indicator state, GPIO init, button logic |
| `tools/prepare_firmware_release.py` | Builds release artifacts + flasher manifest |
| `.github/workflows/firmware-release.yml` | CI build/release/Pages deploy |
| `web/flasher/app.js` | Web Serial flasher logic |

## Current limitations / cautions for agents

- **Do not make functional firmware changes** until the new schematic is available and the
  pin map in `05_hardware_mapping.md` is verified.
- The `.ino` and `main/app_main.cpp` define **two parallel entry points** that must be kept
  in sync (see [02](02_firmware_architecture.md)). The CI build uses the ESP-IDF path
  (`main/`), not the `.ino`.
- `BUTTON_GPIO` is shared between the disconnect (long-press) and penalty (double-press)
  logic — changing one affects the other.
- `stm_init()` sets a timer to `660000 ms` with a `// DOTO why ist here this? WTF` comment —
  origin/purpose unconfirmed (see [04](04_state_machine.md)).
- The **V7/2026 schematic is in the repo**: `pcb_schematic/SCH_Schematic1_2026-05-31.pdf`,
  with a pin-by-pin reference at `docs/ai/ESP32C5_RCJ_modul_hardware_reference.md`
  (**authoritative hardware source**). The firmware pin map matches it for all implemented
  peripherals. `pcb_schematic/SCH.pdf` and the Altium/Eagle libraries are the **older 2024**
  board — reference only.
</content>
</invoke>
