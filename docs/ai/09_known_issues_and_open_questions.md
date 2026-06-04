# 09 — Known Issues and Open Questions

Consolidated risks, contradictions, TODOs, and unknowns. Resolve the **blockers** before
implementing any new-hardware feature.

## Public docs vs code

The public `README.md` was **rewritten on 2026-05-31** to match the V7/2026 (C5) hardware, so
the old contradictions below are now **reconciled** — kept here as history.

| Topic | Old README / 2024 PDF said | Reality (now reflected in README) |
|-------|-----------------------------|-----------------------------------|
| MCU | (implied 2024 ESP32-C6 hardware) | ESP32-**C5** (`master`); C6 lives on `legacy/esp32-c6` |
| Robot-to-robot comms | "Use RX, TX for UART… LOGV… A0/A1 channels (00/01/10/11)" | **No robot-to-robot protocol in firmware** (planned); no LOGV/A0/A1 on V7. README now says so. |
| Start/stop readout | OUT pins only | OUT pins **or** `PLAY`/`STOP` on UART0 |
| Board version | "RCJ Soccer SuperTeams 2024", `modul_v3` | New board silk **"V7 2026"** ("robofuze"); README title de-yeared |
| Schematic | `pcb_schematic/SCH.pdf` (2024) | V7 schematic `SCH_Schematic1_2026-05-31.pdf` present; old 2024 files removed |

## TODO / suspicious comments in code

- `state_machine.cpp:88` — `stm_set_timer(660000); //DOTO why ist here this? WTF`.
  An 11-minute timer set at init with no clear purpose. **Do not remove blindly.**
- `ble.cpp` — `//stm_set_state(STM_PLAY);` commented out in `onConnect` (connect no longer
  auto-plays — intentional).
- `update_output_satet()` — typo in function name (`satet` → `state`); cosmetic.
- Numerous commented-out `Serial.println` debug lines (`ble.cpp`, `ble_processing.cpp`,
  `functions.cpp`).

## Recently resolved

- ✅ **BLE supervision timeout lowered 20 s → 3 s** (`BLE_CONN_TIMEOUT` 2000 → **300**) in
  **fw v0.97** (2026-05-31). Fixed two effects of the old 20 s value: (1) phone reconnect
  took up to ~20 s after link loss; (2) safety — on a *silent* link loss the robot could keep
  **playing for up to ~20 s** before the module's `onDisconnect` fail-safed to STOP. The 20 s
  value had been intentional anti-flap; the C5 supercap reduces that need.
  **Still to do:** (a) validate 3 s on real hardware doesn't cause spurious disconnects during
  normal play; (b) cherry-pick to the `legacy/esp32-c6` branch before the first legacy release.
  See [03_ble_protocol.md](03_ble_protocol.md), [11_legacy_c6_firmware.md](11_legacy_c6_firmware.md).

## Functional issues / sharp edges

1. **Two entry points must stay in sync** (`RCj_comm_module.ino` vs `main/app_main.cpp`).
   Editing one and not the other silently diverges the Arduino-IDE and CI builds.
2. **Shared button pin** — `BUTTON_GPIO` (10) drives both disconnect long-press and penalty
   double-press; redefining button roles affects both paths.
3. **Render gating** — `STM_DISCONNECTED` only redraws on state change, so `SET_SCORE`/
   `SET_NAME` updates received while idle don't show until a state change.
4. **No timer auto-expiry transition** — penalty/halftime countdown reaching 0 does not
   change state; the app must send the next command.
5. **Unprotected shared state** — `current_state`/`state_changed` are written from both the
   loop and the BLE host context without locks (benign so far, but a latent race).
6. **Buttons set as `INPUT`** (not `INPUT_PULLUP`) — relies on board/external pull-ups;
   verify against the schematic to avoid floating inputs.
7. **No BLE security** — open GATT server; any central can send commands. Acceptable for the
   use case but worth noting.
8. **`FW_VERSION` macro** — `FW_VERSION = MAJOR*0xFF + MINOR` (note `*0xFF`, not `*100` or
   `<<8`); the over-BLE reply sends `MAJOR` and `MINOR` separately, so this combined macro's
   only consumer/intent is unclear.

## Schematic details — NOW RESOLVED

The V7/2026 schematic (`pcb_schematic/SCH_Schematic1_2026-05-31.pdf`) and the hardware
reference (`docs/ai/ESP32C5_RCJ_modul_hardware_reference.md`) are now in the repo. The
previously-blocking unknowns are resolved (see [05_hardware_mapping.md](05_hardware_mapping.md)):

- **Firmware pins match the V7 board** for all implemented peripherals (I2C IO2/IO3,
  OUT1=IO9, OUT2=IO8, BUTTON=IO10/B1/SW5, BUTTON2=IO7/B2/SW1). The earlier "firmware pin map
  is wrong for V7" concern came from a since-corrected draft of the reference doc and is
  **not** an issue.
- **RGB LED**: discrete common-cathode, active-high, R=IO27 / G=IO24 / B=IO23 (470 Ω each),
  PWM-capable.
- **Buzzer**: passive 2.7 kHz via NPN Q1; GPIO **IO26 (pin 27)** — confirmed by maintainer.
- **Third button** B3 = IO6 (SW2), active-low w/ 10 kΩ pull-up. Intended function still TBD.
- **Button pulls**: external 10 kΩ (R4/R5/R6), so `INPUT` (no internal pull-up) is fine.
- **UART**: UART0 (RX0/TX0, flashing/primary) and UART1 (RX1=IO4, TX1=IO5) on U3 header.
- **USB**: native USB-CDC on IO13/IO14 (the flashing/reset path) — do not reassign.

Residual low-risk verifications (do not block re-pinning) are listed in
[05_hardware_mapping.md](05_hardware_mapping.md#residual-items-to-verify-low-risk-do-not-block-firmware-re-pinning)
(OLED pull-ups, RGB polarity on bring-up).

## Open product/process questions for the maintainer

1. Must BLE protocol stay **byte-compatible** with the currently shipped app? (Enum ordinals
   are the wire IDs — any reorder breaks it.)
2. Must **old-module (C6) compatibility** be preserved, or is C5 now the only target? Can the
   commented C6 pin block be removed? — **Decided:** C5-only on `master`; C6 lives on a
   `legacy/esp32-c6` branch. See [11_legacy_c6_firmware.md](11_legacy_c6_firmware.md).
3. Is the **UART robot-to-robot** feature still required/planned, or fully deprecated?
4. Supercapacitor: pure hardware ride-through, or should firmware **detect/announce** a power
   dip (LED/buzzer/BLE notification, safe-state)?
5. Desired **RGB LED** semantics per state/event and any future buzzer patterns (the tables in
   [06](06_display_and_user_feedback.md) are guesses). Pins are now known (RGB R/G/B =
   IO27/IO24/IO23; buzzer = IO26).
6. Intended role of the **third button** B3 = **IO6 / SW2** (menu? channel? manual play/stop?).
7. Purpose of the **`660000 ms`** init timer — keep, fix, or remove?
8. Should the `.ino` entry point be retired in favor of the IDF `main/` path?

## Build/CI notes

- ESP-IDF was not available in this checkout; a fresh build was not verified here.
- The workflow checks out `submodules: recursive`, but **no `.gitmodules` exists** — the
  recursive flag is currently a no-op. Confirm whether submodules are planned.

## Source files reviewed

All firmware sources, workflow, flasher, README, render images, git history.
</content>
