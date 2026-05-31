# 11 — Legacy ESP32-C6 Firmware: Strategy & Release Plan

Plan for shipping a **final, maintained legacy firmware** for the old 2024 ESP32-C6 module
after `master` becomes ESP32-C5-only. **Nothing in this document has been executed** — no
branch created, no firmware changed, no tags pushed. Each step below is gated on explicit
maintainer approval (and the project rule: never `git push` unless asked in that turn).

## Decisions locked in (2026-05-31)

| Topic | Decision |
|-------|----------|
| Codebase compatibility | Drop C6 support from `master`; `master` is C5-only going forward |
| Legacy distribution | Final C6 binaries as **GitHub Release assets** + flashing docs; small "old module?" link from the C5 flasher page. No legacy web-flasher page. |
| Legacy maintenance | Long-lived **`legacy/esp32-c6` branch**; critical fixes cherry-picked from `master`; releases tagged **`legacy-c6-vX.Y`** |
| Legacy CI | **Auto-build** the C6 binaries on `legacy-c6-*` tags (GitHub Actions). Pages/web flasher stays C5-only. |
| C6 flashing access | Two documented paths: (a) the **special flashing/jig PCB**, (b) **direct wiring to the right module pin header**. Both expose native-USB **and** UART. The C6 module has **no USB-C connector**. |

## Old C6 board reference (from `pcb_schematic/SCH.pdf`, "modula_2024" V0.1)

| Item | Detail |
|------|--------|
| MCU | **ESP32-C6-MINI-1-N4** (4 MB flash, no PSRAM) |
| Power | `UA78M33CDCYR` LDO, VIN 5.3–25 V → 3.3 V, ~500 mA (no supercap) |
| Inputs | 2 buttons: SW1→`BUT_1`, SW2→`BUT_2` (10 kΩ pull-ups) |
| Outputs | `OUT_1`, `OUT_2` (1 kΩ series R6/R7) — robot start/stop |
| Display | I2C OLED (`SDA`/`SCL`) |
| Extra HW | **LIS3DHTR accelerometer** (I2C, unused by firmware); level-shifted UART via `TXS0102` + `LOGV` pin; USB D+/D- broken out on the U3 header (no connector) |
| Not present | RGB LED, buzzer, third button, supercap, USB-C — all C5-only |

> The current firmware only uses I2C OLED + 2 buttons + 2 outputs + BLE, all of which the C6
> board has. So today's `master` source, compiled for `esp32c6` with the C6 pin block, yields
> a **functionally complete** legacy build. The accelerometer stays unused.

## C6 pin map (the commented block in `definitions.h`)

```c
// ESP-C6
#define I2C_SDA_GPIO    6
#define I2C_SCL_GPIO    7
#define BUTTON_GPIO     18
#define BUTTON2_GPIO     9
#define OUTPUT1_GPIO    20
#define OUTPUT2_GPIO    19
```

These were the **working pins before the C5 migration** (commit `7f9fb81` added C5 and
commented out C6), so confidence is high. They are cross-consistent with the C6-MINI pinout
in `SCH.pdf` (SDA=IO6, SCL=IO7; USB on IO12/IO13; outputs on IO19/IO20). **Still must be
confirmed by a real build + smoke test on a C6 module before release** (see checklist).

## Git layout

```
master              ── ESP32-C5 only. Remove the commented C6 block from definitions.h.
                       Workflow triggers on fw-* / v*. Deploys C5 web flasher to Pages.

legacy/esp32-c6     ── Branched from the current C5-capable HEAD. On this branch:
                        • definitions.h: C6 pins ACTIVE (not commented), C5 block removed
                        • CMakeLists.txt: set(IDF_TARGET "esp32c6" ...)
                        • sdkconfig.defaults: CONFIG_IDF_TARGET="esp32c6" / *_ESP32C6=y
                        • workflow: trigger on legacy-c6-*, target esp32c6, NO Pages deploy
                       Tagged legacy-c6-vX.Y for each legacy release.
```

Rationale for a **branch, not just a tag**: the maintained policy means we may cherry-pick a
critical fix from `master` and cut a new `legacy-c6-v…` later. A branch makes that a normal
`git cherry-pick` + tag, with no need to recreate the C6 config each time.

## Build & artifacts (answers "is the .bin enough?")

A bare app `.bin` is **not** enough to flash a blank/erased chip. Three images are required
at target-specific offsets: **bootloader**, **partition table**, **app**. The existing
`tools/prepare_firmware_release.py` already merges them via `esptool merge_bin` into a single
image flashed at `0x0`. **That merged file is the one sufficient artifact.**

The C6 legacy release will publish (same tooling, just target `esp32c6`):

| File | Purpose |
|------|---------|
| `rcj_comm_module-<ver>-merged.bin` | **Primary** — flash at `0x0` with full erase. Contains bootloader + partition table + app. |
| `rcj_comm_module-<ver>.bin` | App only (advanced / OTA-style) |
| `rcj_comm_module-<ver>-bootloader.bin` | Advanced (flash at the C6 bootloader offset) |
| `rcj_comm_module-<ver>-partition-table.bin` | Advanced (flash at `0x8000`) |
| `manifest.json`, `version.json` | Chip family, sizes, sha256 |

> `prepare_firmware_release.py` and the workflow are **target-agnostic**: they read chip and
> per-part offsets from `build/flasher_args.json` and derive `chipFamily = ESP32-C6`
> automatically. **No script changes needed** — only the IDF target. The C6 bootloader offset
> (which differs from the C5 `0x2000`) is handled by `merge_bin`, so the single merged image
> remains "flash at `0x0`."

## Flashing the legacy module (to be documented for users)

Both access methods reach the same module pins; pick whichever the user has:
- **(a) Flashing/jig PCB** — connect the module to the jig, then flash over native USB or UART.
- **(b) Direct pin-header wiring** — wire the right module header pins to a USB or USB-UART
  adapter.

Then either:
- **One-file (recommended):**
  `esptool --chip esp32c6 write_flash --erase-all 0x0 rcj_comm_module-<ver>-merged.bin`
- **Web/esptool-js (jig native-USB users):** point esptool-js at the merged bin, `0x0`,
  `eraseAll: true`, chipFamily `ESP32-C6`. (Optional future nicety; not built now.)
- **Parts (advanced):** flash bootloader/partition/app at their offsets via `esptool`.

### Flashing-method note (verified 2026-05-31 on real hardware)

**Use native USB — it is by far the easier path.** The C6's USB D+/D− (IO13/IO12) are
broken out on the **U3 header** (`USB_D+` / `USB_D-`, 6-pin `2541WV-06P`). Wiring those to a
USB port makes the module enumerate as **`/dev/ttyACM*`** (USB-Serial-JTAG); esptool then has
built-in auto-reset/download — no buttons. This is how the test board was flashed:
`esptool --chip esp32c6 -p /dev/ttyACM0 write_flash --erase-all 0x0 …-merged.bin` (worked
first try, hash verified).

**The level-shifted UART path is fiddly — note for whoever documents it:**
- On the schematic, the ESP **UART0** (`TXD0`/`RXD0` = `TX_ESP`/`RX_ESP`) goes through the
  **`TXS0102`** level shifter to **`TX_OUT`/`RX_OUT`** on the U3 header. The shifter is
  referenced/enabled by **`LOGV`** (must be 3.3 V) — if `LOGV` is unpowered the line is dead
  (symptom: esptool "No serial data received", *zero* bytes even on a plain reset).
- UART flashing also needs **manual download mode**: hold **IO9 (BOOT)** low, pulse **EN**,
  release IO9 — there is **no auto-reset circuit** on bare pin-header wiring (DTR/RTS not
  wired). Use **3.3 V logic** and a common GND.
- Given all that, prefer native USB for users; document UART only as a fallback.

## CI (legacy branch)

A workflow on `legacy/esp32-c6` (a copy/edit of `firmware-release.yml`) that:
- triggers on tags `legacy-c6-*`,
- builds with `espressif/esp-idf-ci-action` (ESP-IDF v5.5.4), **target `esp32c6`**,
- runs `prepare_firmware_release.py`,
- creates/updates a **GitHub Release** with the C6 bins + `manifest.json` + `version.json`
  + `release-notes.md`,
- **omits** the GitHub Pages jobs (Pages = C5 flasher only).

## Maintenance policy

- **First release = snapshot of current code** (already includes the BLE latency/resilience
  fix `ca23261`), plus the pre-release fixes in the checklist below.
- Afterward, cut a new `legacy-c6-v…` **only for a critical fix**, where *critical* =
  robot-fails-to-stop / robot-keeps-playing-after-link-loss / BLE-won't-connect /
  display-dead class bugs. Cosmetic/feature changes do **not** warrant a legacy release.
- Fix flow: land on `master` → `git cherry-pick` onto `legacy/esp32-c6` → tag `legacy-c6-vX.Y`.

## Pre-first-release checklist

**Must-fix (before the first legacy release):**
- [x] **Lower `BLE_CONN_TIMEOUT`** from `2000` (20 s) to **`300`** (3 s) — DONE on `master`
      in **fw v0.97** (2026-05-31). Safety/fairness: with 20 s, a robot could keep playing for
      up to ~20 s after a silent link loss before the module fail-safed to STOP; also fixes the
      slow phone reconnect. *(See [03](03_ble_protocol.md), [09](09_known_issues_and_open_questions.md),
      `project-ble-supervision-timeout`.)*
- [ ] **Cherry-pick the v0.97 timeout change onto `legacy/esp32-c6`** once that branch exists.

**Must-verify (gating, not bugs):**
- [x] **UART line is clean for serial-reading robots — DONE (2026-05-31).** Only `PLAY`/`STOP`
      are printed (other debug prints already commented); `Serial`→UART0 confirmed; release
      `sdkconfig.defaults` now silences bootloader/IDF/Arduino-HAL logs and pins console to
      UART0. A `sdkconfig.debug` overlay gives USB-C logging for development. **This config
      carries over to the legacy C6 branch** (verify it still applies for target esp32c6 —
      the symbols are target-independent). *(See [02](02_firmware_architecture.md#build-flavors-release-uart-clean-vs-debug-usb-verbose),
      [07](07_build_release_and_flasher.md).)*
- [x] Current source **compiles for `esp32c6`** — DONE (2026-05-31). Built on the
      `legacy/esp32-c6` branch with ESP-IDF v5.5.4 + arduino-esp32 ^3.3.0 + NimBLE; app image
      ~0xbe490 bytes, 26% free in the 1 MB app partition. Config: `IDF_TARGET=esp32c6`,
      `CONFIG_ESPTOOLPY_FLASHSIZE_4MB` (C6-MINI-1-N4 = 4 MB), C6 pin block active.
- [~] **C6 pin map — partially confirmed on real hardware (2026-05-31).** After flashing:
      **OLED renders** (I2C on IO6/IO7 ✅) and **BLE advertises + connects** ✅. Still to
      confirm: `OUT1`/`OUT2` (IO20/IO19) HIGH on PLAY / LOW on STOP & disconnect; both buttons
      (disconnect long-press IO18, penalty double-press IO18/IO9).
- [x] **Merged C6 bin flashes cleanly — DONE via native USB (2026-05-31).** `esptool --chip
      esp32c6 write_flash --erase-all 0x0 …-merged.bin` over the native USB-Serial-JTAG port
      (`/dev/ttyACM0`); hash verified. See the flashing-method note below.

**Not blocking (known, low risk — document, don't fix for legacy):**
- `stm_init()` `660000 ms` timer (`//DOTO … WTF`) — harmless; overwritten by DAMAGE/HALF_BREAK.
- `STM_DISCONNECTED` only redraws on state change (score/name updates not shown while idle) —
  cosmetic.
- Two entry points (`.ino` vs `main/app_main.cpp`) must stay in sync — maintenance hazard.
- Shared `BUTTON_GPIO` for disconnect + penalty — pre-existing behavior, unchanged on C6.

No true showstopper logic bug was found: the stop path is fail-safe (outputs LOW on
STOP/DAMAGE/HALF_BREAK/GAME_OVER and on disconnect), and the message queue keeps the newest
command when full.

## Staged execution (each step needs explicit go-ahead)

1. ✅ Land the `BLE_CONN_TIMEOUT` fix on `master` — done (300 / 3 s, fw v0.97).
2. Create `legacy/esp32-c6` from the C5-capable HEAD (then cherry-pick the v0.97 change).
3. On the legacy branch: activate C6 pins, set IDF target `esp32c6`, add the legacy workflow.
4. Verify a C6 build + smoke test on hardware.
5. Remove the commented C6 block from `master` (now redundant).
6. Tag `legacy-c6-v0.96` (or current version) → CI builds + publishes the Release.
7. Add a public-`README.md` "Legacy 2024 (ESP32-C6) modules" section + flashing docs, and a
   small link from the C5 flasher page.

## Open questions

- ~~Exact target value for `BLE_CONN_TIMEOUT`~~ — resolved: **300 (3 s)** in fw v0.97.
- Should the legacy release version track `master`'s `FW_VERSION` (e.g. `legacy-c6-v0.97`),
  or start its own legacy numbering?
- Do we want the optional esptool-js web path for jig users now, or defer indefinitely?

## Source files reviewed

`pcb_schematic/SCH.pdf`, `definitions.h`, `CMakeLists.txt`, `sdkconfig.defaults`,
`.github/workflows/firmware-release.yml`, `tools/prepare_firmware_release.py`,
`web/flasher/app.js`, `build/flasher_args.json`, and docs [03], [05], [07], [09], [10].
</content>
