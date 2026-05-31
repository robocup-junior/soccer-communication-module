# 01 — Repository Map

## Top-level tree (tracked + notable untracked)

```
.
├── README.md                          # Public docs (partly OUTDATED — 2024 HW)
├── LICENSE
├── RCJ 2024 communication modules.pdf # Public manual for the 2024 module (present, not fully analyzed)
├── .github/workflows/
│   └── firmware-release.yml           # CI: build + GitHub Release + Pages flasher deploy
├── firmware/RCj_comm_module/          # The firmware (see below)
├── web/flasher/                       # Static Web-Serial flasher site (deployed to Pages)
├── tools/
│   └── prepare_firmware_release.py    # Packages build output + flasher manifest/version
├── mobileAPP/
│   └── RCj2025_APK-...zip              # Mobile app APK bundle (binary zip, not analyzed)
├── pcb_schematic/
│   ├── SCH.pdf                        # 2024 schematic (OLD HW)
│   ├── SCH_Schematic1_2026-05-31.pdf  # NEW V7/2026 schematic (ESP32C5_RCJ_modul, V1.0) — CURRENT
│   ├── easy_eda_pro_file.epro         # EasyEDA Pro project (binary)
│   └── converted_altium_file/...      # Altium PCB/schematic export (binary)
├── pcb_library/
│   ├── RCJ-COM 2024 eagle.lbr         # Eagle library (2024)
│   └── RCJ-COM 2024.IntLib            # Altium integrated library (2024)
├── jlcpcb_gerbers/
│   ├── Modul_v3_2024-04-05.zip        # Gerbers for 2024 board (binary)
│   └── PickAndPlace_modul_2024-04-05.xlsx
├── 3d_model/                          # modul_v3.{step,iges,f3d,zip}, hub_board.step (binary CAD)
├── .readme_images/
│   ├── rcjv3_dimensions.png, modul_2024.png, hub_image.png   # tracked (2024 module)
│   └── 3D_PCB1_2026-05-21 v4{,1,2}.png # UNTRACKED — renders of the NEW V7/2026 board
├── build/                             # (untracked) stray top-level build dir
└── docs/ai/                           # THIS documentation set
    └── ESP32C5_RCJ_modul_hardware_reference.md  # maintainer V7 pinout (authoritative HW source)
```

## Firmware folder detail

```
firmware/RCj_comm_module/
├── RCj_comm_module.ino     # Arduino-IDE sketch entry point (setup/loop)
├── definitions.h           # FW version, BLE/UART constants, GPIO pin map (C5 + commented C6)
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
| **Hardware design (2024, binary — for reference)** | `pcb_schematic/`, `pcb_library/`, `jlcpcb_gerbers/`, `3d_model/` |
| **Images** | `.readme_images/*` (tracked = 2024; untracked `3D_PCB1_2026-05-21 v4*.png` = new V7 board) |

## Files present but not (fully) analyzed

These are binary or non-text and were not opened beyond noting their existence:

- `RCJ 2024 communication modules.pdf`, `pcb_schematic/SCH.pdf` (PDF — describe 2024 HW)
- `pcb_schematic/easy_eda_pro_file.epro`, `converted_altium_file/**` (CAD binaries)
- `pcb_library/*.lbr`, `*.IntLib` (CAD libraries)
- `jlcpcb_gerbers/*.zip`, `*.xlsx`
- `3d_model/*.{step,iges,f3d,zip}`
- `mobileAPP/RCj2025_APK-...zip` (app binary)

## Source files reviewed

`git ls-files`, directory tree, `firmware/RCj_comm_module/.gitignore`, the three new-board
render PNGs, `build/flasher_args.json`.
</content>
