# 10 — Future Change Plan (Staged)

A safe, staged plan for adding the new ESP32-C5 / V7-2026 hardware features **after** the
schematic is provided. Each stage is independently buildable, testable, and revertible.
**Do not start Stage 2+ until Stage 1 confirms the pin map.**

> Cross-cutting rules:
> - Keep BLE wire compatibility: **append** new `ble_msg_id` values before `BLE_MSG_MAX_ID`;
>   never reorder existing ones (see [03](03_ble_protocol.md)).
> - Mirror any setup/loop change in **both** `RCj_comm_module.ino` and `main/app_main.cpp`
>   (see [02](02_firmware_architecture.md)).
> - Bump `FW_VERSION_MINOR` in `definitions.h` to match each release tag.
> - One feature per branch/PR; keep diffs small for easy rollback.

## Stage 1 — Verify schematic & pin mapping (BLOCKER) — ✅ DONE (2026-05-31)

- ✅ Schematic in repo: `pcb_schematic/SCH_Schematic1_2026-05-31.pdf`; pin reference:
  `docs/ai/ESP32C5_RCJ_modul_hardware_reference.md`. All pins documented in
  [05_hardware_mapping.md](05_hardware_mapping.md).
- ✅ Confirmed current firmware pins **match** the V7 board (I2C, both buttons, both outputs).
- ✅ New-peripheral pins known: RGB R/G/B = IO27/IO24/IO23 (active-high, common-cathode),
  buzzer = IO26 (pin 27), B3 = IO6/SW2, UART1 = IO4/IO5, GPIO28 on H1.
- **Remaining before Stage 3+:** decide whether to keep or delete the commented C6 block in
  `definitions.h`.

## Stage 2 — Hardware abstraction / definitions

- Add the confirmed pins to `definitions.h` (e.g. `RGB_LED_GPIO`, `BUZZER_GPIO`,
  `BUTTON3_GPIO`, `VIN_SENSE_GPIO`) as **defines only**, plus `pinMode` setup in
  `module_init_gpios()` — no behavior yet.
- Consider introducing a thin `hardware.h`/HAL boundary so feature modules don't sprinkle
  raw GPIO numbers.
- **Test:** builds clean; existing behavior unchanged (outputs/display/BLE identical).
- **Rollback:** revert the definitions/pinMode additions.

## Stage 3 — RGB LED driver

- Add `rgb_led.cpp/.h` (or extend a `feedback` module). Pick the driver per LED type
  (addressable WS2812 via RMT, or 3× PWM channels for a discrete RGB).
- Drive it from state transitions in `state_machine.cpp` (hook into the existing
  `state_changed` path), with a confirmed color map (replace the proposed table in
  [06](06_display_and_user_feedback.md) with the agreed one).
- **Test:** observe color per state on hardware; ensure no timing impact on the 1 ms loop.
- **Rollback:** compile-time flag (`#define FEATURE_RGB_LED`) to disable.

## Stage 4 — Buzzer driver

- Add `buzzer.cpp/.h`. If passive, use LEDC/`tone`-style PWM; if active, simple on/off.
- Make beeps **non-blocking** (no `delay()` in the loop — use a millis-based scheduler) to
  avoid stalling BLE/display.
- Trigger on events (connect, play, stop, penalty, self-penalty ack) per agreed semantics.
- **Test:** verify beeps don't delay output-pin updates or BLE processing.
- **Rollback:** `#define FEATURE_BUZZER` guard.

## Stage 5 — Define third-button behavior

- Wire `BUTTON3_GPIO` and implement its agreed function (menu / channel / manual control —
  TBD with maintainer). Reuse/extend the debounce pattern in `functions.cpp`.
- Re-examine the **shared `BUTTON_GPIO`** disconnect+penalty logic; with three buttons,
  consider separating roles cleanly to remove the long-press/double-press overlap.
- **Test:** all button combinations; confirm no regressions to disconnect/self-penalty.
- **Rollback:** keep new button handling behind a guard; old two-button logic intact.

## Stage 6 — Supercapacitor / power-loss behavior

- Only if a measurable signal exists (VIN-sense ADC or power-good GPIO confirmed in Stage 1).
- On detected dip: enter a safe state (outputs LOW or hold last? — confirm with rules),
  optionally flash LED / beep / send a BLE notification, and persist minimal state if needed.
- Keep the BLE link alive across the dip (the whole point of the supercap).
- **Test:** bench power-interrupt test; measure ride-through time.
- **Rollback:** guard; default to no power monitoring (current behavior).

## Stage 7 — Display / feedback integration

- Unify OLED + RGB LED + buzzer behind a single per-state `feedback` update so all outputs
  stay consistent (one source of truth driven by `stm_update`).
- Add any new screens (e.g. menu for button 3, low-power warning).
- Fix the `STM_DISCONNECTED` redraw gating if score/name should update live.
- **Test:** visual review of every state across all feedback channels.

## Stage 8 — Docs, tests, build/release

- Update `docs/ai/05` (final pin map), `06` (final LED/buzzer maps), `09` (close resolved
  questions), and the public `README.md`.
- Add/refresh any host-side or on-target smoke tests if introduced.
- Verify CI build (`idf.py build` via the workflow) and the web flasher still work; tag a new
  `fw-vX.Y` release.

## Risk & rollback summary

| Risk | Mitigation |
|------|------------|
| BLE protocol break | Append-only enum; test against the real app before release |
| Loop stalls from blocking LED/buzzer/power code | Non-blocking, millis-based; measure loop time |
| Entry-point divergence | Edit `.ino` and `main/app_main.cpp` together (or retire `.ino`) |
| Wrong pin assumptions | Stage 1 gate; never guess from renders |
| Hard-to-revert changes | One feature per PR + compile-time `FEATURE_*` guards |
| C6 vs C5 confusion | Decide target support explicitly in Stage 1 |

## Source files reviewed

All `docs/ai/*`, firmware sources, workflow, README.
</content>
