# 01 — Repository Map

## Top-level tree (tracked + notable untracked)

> **2026-05-31 cleanup:** the old 2024/v3 (C6) design artifacts were removed from `master`
> (`mobileAPP/`, the 2024 PDF, old `SCH.pdf`/`.epro`/Altium export, `pcb_library/`,
> `jlcpcb_gerbers/`, the `modul_v3.*` CAD, and the old module images). `master` now keeps only
> the **new V7** schematic + renders. The mobile app moved to its own repo
> (`robocup-junior/soccer-referee-app`, on Google Play). The legacy C6 hardware files still
> live in git history and on the `legacy/esp32-c6` branch.

```
.
├── README.md                          # Public docs — describes the CURRENT V7/2026 (C5) HW
├── LICENSE
├── .github/workflows/
│   └── firmware-release.yml           # CI (master): build C5 + GitHub Release + Pages flasher
│                                       #   (legacy/esp32-c6 also has firmware-release-legacy-c6.yml)
├── firmware/RCj_comm_module/          # The firmware (see below)
├── web/flasher/                       # Static Web-Serial flasher site (deployed to Pages)
├── tools/
│   └── prepare_firmware_release.py    # Packages build output + flasher manifest/version
├── pcb_schematic/
│   └── SCH_Schematic1_2026-05-31.pdf  # NEW V7/2026 schematic (ESP32C5_RCJ_modul, V1.0) — CURRENT
├── 3d_model/
│   └── hub_board.step                 # EV3/SPIKE hub accessory CAD (binary)
├── .readme_images/
│   ├── 3D_PCB1_2026-05-21 v4{,1,2}.png # renders of the NEW V7/2026 board (v41 = README/flasher hero)
│   └── hub_image.png                  # hub accessory photo
├── build/                             # (untracked) stray top-level build dir
└── docs/ai/                           # THIS documentation set
    └── ESP32C5_RCJ_modul_hardware_reference.md  # maintainer V7 pinout (authoritative HW source)
```

## Firmware folder detail

```
firmware/RCj_comm_module/
├── RCj_comm_module.ino     # Arduino-IDE sketch entry point (setup/loop)
├── definitions.h           # FW version, BLE/UART constants, GPIO pin map (C5 only; C6 map on legacy branch)
├── ble.cpp / ble.h         # BLE server, NUS UUIDs, RX/TX characteristics, server callbacks
├── ble_processing.cpp/.h   # ble_msg_t struct, ble_msg_id enum, RTOS queue, command dispatch
├── state_machine.cpp/.h    # stm_states enum, output-pin control, timers
├── display.cpp / display.h # SSD1306 OLED screen rendering (per state)
├── functions.cpp/.h        # MAC→string, score/indicator state, GPIO init, button handling
├── fonts.h                 # Bitmap fonts for the OLED
├── images.h                # XBM logo (RC_logo) for boot screen
├── CMakeLists.txt          # ESP-IDF top project file (sets IDF_TARGET=esp32c5)
├── sdkconfig.defaults      # ESP-IDF config: target C5, NimBLE peripheral, FreeRTOS 1000 Hz
├── .gitignore              # ignores build/, managed_components/, sdkconfig, .vscode/, etc.
├── main/
│   ├── app_main.cpp        # ESP-IDF entry point (app_main → initArduino + setup/loop)
│   ├── CMakeLists.txt      # Registers all ../*.cpp + bundled libs as the "main" component
│   └── idf_component.yml   # Pulls espressif/arduino-esp32 ^3.3.0
├── libraries/              # Vendored Arduino libraries (source, committed):
│   ├── ESP8266_and_ESP32_OLED_driver_for_SSD1306_displays/  # ThingPulse SSD1306 driver
│   ├── QRcodeDisplay/                                        # QR encoder + frames
│   └── QRcodeOled-2.0.0/                                     # QR-on-OLED adapter
├── build/                  # (untracked, gitignored) prior ESP-IDF build output
└── managed_components/     # (untracked, gitignored) IDF-fetched components (arduino-esp32, …)
```

## Source vs generated / vendored

| Category | Paths |
|----------|-------|
| **Firmware source (edit these)** | `firmware/RCj_comm_module/*.cpp`, `*.h`, `*.ino`, `main/app_main.cpp`, `main/CMakeLists.txt`, `CMakeLists.txt`, `sdkconfig.defaults`, `main/idf_component.yml` |
| **Vendored libs (avoid editing)** | `firmware/RCj_comm_module/libraries/**` (third-party OLED + QR code) |
| **Tooling** | `tools/prepare_firmware_release.py`, `.github/workflows/firmware-release.yml` |
| **Web flasher source** | `web/flasher/{index.html,app.js,styles.css,.nojekyll}` |
| **Generated / fetched (gitignored, do not commit/edit)** | `firmware/RCj_comm_module/build/`, `managed_components/`, `sdkconfig`, `dependencies.lock`, top-level `build/`, CI `dist/` |
| **Release artifacts (produced by CI, attached to Release)** | `dist/firmware/rcj_comm_module-<tag>{,-bootloader,-partition-table,-merged}.bin`, `dist/manifest.json`, `dist/version.json`, `dist/release-notes.md` |
| **Hardware design (V7, binary — for reference)** | `pcb_schematic/SCH_Schematic1_2026-05-31.pdf`, `3d_model/hub_board.step` |
| **Images** | `.readme_images/3D_PCB1_2026-05-21 v4{,1,2}.png` (V7 renders, v41 = hero), `hub_image.png` |

## Files present but not (fully) analyzed

These are binary or non-text and were not opened beyond noting their existence:

- `pcb_schematic/SCH_Schematic1_2026-05-31.pdf` (V7 schematic — captured in the HW reference doc)
- `3d_model/hub_board.step` (hub accessory CAD)

> The old 2024/C6 binaries (`SCH.pdf`, `.epro`, Altium export, Eagle/Altium libs, `modul_v3`
> CAD, JLCPCB gerbers, the 2024 PDF, the APK zip) were removed from `master` on 2026-05-31; see
> the cleanup note at the top. They remain in git history and on `legacy/esp32-c6`.

## Source files reviewed

`git ls-files`, directory tree, `firmware/RCj_comm_module/.gitignore`, the three new-board
render PNGs, `build/flasher_args.json`.
</content>
