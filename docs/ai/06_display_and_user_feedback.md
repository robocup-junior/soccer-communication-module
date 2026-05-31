# 06 — Display and User Feedback

Rendering lives in `display.cpp` using the ThingPulse SSD1306 driver (128×64, I2C `0x3c`)
and the `QRcodeOled` library. Fonts are in `fonts.h`; the boot logo (`RC_logo`) is an XBM in
`images.h`.

## Helper functions (`display.cpp`)

- `get_time_string(seconds)` → `"S"` if < 60 s, else `"M:SS"` (zero-padded seconds).
- `get_score_string()` → `"<my>:<opp>"` from `module_get_my_score()`/`opponent_score()`.

## Screens (current behavior)

| Function | State | Content |
|----------|-------|---------|
| `display_screen_init()` | `STM_INIT` | `RC_logo` full-screen XBM + bottom-right `"v <MAJOR>.<MINOR>"` |
| `display_screen_wait_for_connection()` | `STM_DISCONNECTED` | 64×64 **QR code of the BLE MAC** (left), `"Wait for\nconnection"` text + MAC split over two lines (right) |
| `display_screen_play()` | `STM_PLAY` | `"PLAY"` top; large 2-char `indicator` (split into two glyphs at x=41 and x=85, `DialogInput_bold_50`); `score` at bottom |
| `display_screen_stop()` | `STM_STOP` | Two filled side bars (`fillRect`) framing the screen; `"STOP"` top; large `indicator`; `score` bottom |
| `display_screen_damage(time)` | `STM_DAMAGE` | `"PENALTY - <indicator>"` top; large countdown (`Dialog_plain_40`); `score` bottom |
| `display_screen_half_break(time)` | `STM_HALF_TIME` | `"HALFTIME - <indicator>"` top; large countdown; `score` bottom |
| `display_screen_game_over()` | `STM_GAME_OVER` | `"GAME OVER"` top; large **score** (`Dialog_plain_40`); `indicator` small at bottom |

Each screen does `display.clear()` → draw → `display.display()`. `invertDisplay()` calls are
present but commented out.

## QR / MAC connection behavior

- On the wait screen, `qrcode.init(64, 64)` then `qrcode.create(mac_string)` renders a QR of
  the **Bluetooth MAC** (`BLE_MAC_to_string()`), the same value embedded in the advertised
  name `RCJs-m_<MAC>`.
- The MAC is also printed as text: `mac.substring(0,9)` on line 1, `mac.substring(9)` on
  line 2 (splitting the `AA:BB:CC:DD:EE:FF` string).
- The mobile app is expected to scan the QR (or read the name) to identify/connect to the
  correct module.

## Score display

- Maintained by `my_score` / `opponent_score` globals (`functions.cpp`).
- Set via `BLE_MSG_SET_SCORE` (both bytes) and `BLE_MSG_GAME_OVER` (both bytes).
- Shown as `"my:opp"` on PLAY/STOP/PENALTY/HALFTIME screens (bottom) and large on GAME OVER.

## Indicator (team/robot label) display

- `module_indicator`, default `"--"` (`DEFAULT_INDICATOR`).
- Set via `BLE_MSG_SET_NAME` (2 ASCII chars).
- Rendered as two large glyphs on PLAY/STOP; inline in PENALTY/HALFTIME titles; small on
  GAME OVER. Only the first 1–2 characters are used.

## Planned RGB LED behavior — PROPOSED, NOT IMPLEMENTED

> The following are **design ideas only**. No RGB LED code exists; the pin/type is unknown
> (see [05](05_hardware_mapping.md)). Listed to seed future design discussion — do not build
> until the schematic confirms the LED.

| State | Proposed LED idea (unconfirmed) |
|-------|----------------------------------|
| `STM_DISCONNECTED` | slow blue/white pulse "waiting" |
| `STM_PLAY` | solid green |
| `STM_STOP` | solid red |
| `STM_DAMAGE` | amber/orange, maybe blinking with countdown |
| `STM_HALF_TIME` | amber steady |
| `STM_GAME_OVER` | dim white / off |

## Planned buzzer behavior — PROPOSED, NOT IMPLEMENTED

> Same caveat: no buzzer code exists; pin and passive/active type unknown.

| Event | Proposed buzzer idea (unconfirmed) |
|-------|------------------------------------|
| BLE connected | short single beep |
| `PLAY` | short rising beep |
| `STOP` | short low beep |
| Penalty start / countdown end | distinct beep pattern |
| Self-penalty request acknowledged | confirmation beep |
| Power dip (supercap) | optional warning tone |

## Source files reviewed

`display.cpp/.h`, `functions.cpp`, `fonts.h` (referenced), `images.h` (referenced).

## Open questions

- Confirm OLED geometry is 128×64 on the new board (assumed from `drawXbm` dimensions).
- Desired RGB LED / buzzer semantics from the maintainer (the tables above are guesses).
- Should display feedback and LED/buzzer be unified behind one `feedback`/HAL module?
</content>
