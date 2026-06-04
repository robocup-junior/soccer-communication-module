# 05 — Hardware Mapping

The V7/2026 schematic is now in the repo and the pin map is **confirmed**:
- Schematic: `pcb_schematic/SCH_Schematic1_2026-05-31.pdf` (EasyEDA "ESP32C5_RCJ_modul",
  Board1 / Schematic1, V1.0, MCU **ESP32-C5-WROOM-1** — schematic label `-N16R4`, but the
  fitted flash reads **8 MB** via JEDEC (build is configured for 8 MB; see the hardware
  reference's "Microcontroller — U1" note) / 4 MB PSRAM).
- Authoritative pin reference (maintainer-authored, read pin-by-pin from U1):
  [`ESP32C5_RCJ_modul_hardware_reference.md`](ESP32C5_RCJ_modul_hardware_reference.md).

> ✅ **The firmware (`definitions.h`) matches the V7 hardware for every implemented
> peripheral** (I2C, the two wired buttons, the two robot outputs, and the buzzer). The
> RGB LED, third button, and UART1 exist on the board but are **not yet used by firmware** — their
> confirmed pins are listed below.

## Current firmware pin map — ESP32-C5 (active)

```c
// ESP-C5  (definitions.h)
#define I2C_SDA_GPIO    2     // OLED SDA
#define I2C_SCL_GPIO    3     // OLED SCL
#define BUTTON_GPIO     10    // primary button (disconnect long-press + penalty double-press)
#define BUTTON2_GPIO     7    // secondary button (penalty double-press)
#define OUTPUT1_GPIO     9    // robot start/stop OUT1 (HIGH=play, LOW=stop)
#define OUTPUT2_GPIO     8    // robot start/stop OUT2 (mirrors OUT1)
```

| Function | C5 GPIO | Direction | V7 net / part | Notes |
|----------|---------|-----------|---------------|-------|
| I2C SDA (OLED) | 2 | — | SDA, H2 hdr | `SSD1306(0x3c, SDA, SCL)` ✅ matches HW |
| I2C SCL (OLED) | 3 | — | SCL, H2 hdr | I2C addr `0x3c` ✅ matches HW |
| OUT1 | 9 | output | OUT1, U3 hdr | robot GO/STOP signal ✅ matches HW (IO9) |
| OUT2 | 8 | output | OUT2, U3 hdr | duplicate of OUT1 ✅ matches HW (IO8) |
| BUTTON | 10 | input | **B1 = SW5** | disconnect (5 s hold) **and** penalty (double-press); SW5 is the 4-pin slide/toggle "power/mode" switch |
| BUTTON2 | 7 | input | **B2 = SW1** | penalty (double-press); SW1 momentary tactile |

> Note: firmware `BUTTON_GPIO` (10) is hardware **B1/SW5** (the 4-pin slide/toggle switch),
> and `BUTTON2_GPIO` (7) is **B2/SW1**. The third button **B3/SW2 = IO6** is not wired in
> firmware. Hardware provides 10 kΩ pull-ups (R4/R5/R6) on all three, so the buttons are
> active-low as the firmware assumes.

## Legacy pin map — ESP32-C6 (commented out, retained for reference)

```c
// ESP-C6  (commented in definitions.h)
// I2C_SDA_GPIO    6
// I2C_SCL_GPIO    7
// BUTTON_GPIO     18
// BUTTON2_GPIO     9
// OUTPUT1_GPIO    20
// OUTPUT2_GPIO    19
```

These are kept as comments from the previous (C6) hardware revision. They are **not**
compiled. Useful only if a C6 board must be reflashed.

## Display / I2C

- Driver: ThingPulse SSD1306 (`libraries/ESP8266_and_ESP32_OLED_driver_for_SSD1306_displays`).
- Instance: `static SSD1306 display(0x3c, I2C_SDA_GPIO, I2C_SCL_GPIO)` in `display.cpp`.
- Resolution assumed 128×64 (boot draws `drawXbm(0,0,128,64,RC_logo)`).
- The new board exposes a 4-pin OLED header silk-labeled **`GND VCC SCL SDA`** (visible in
  the front render) — consistent with an external/standard SSD1306 module.

## Button handling (current behavior)

`functions.cpp`:
- `module_init_gpios()` sets `BUTTON_GPIO` and `BUTTON2_GPIO` as `INPUT`
  (no `INPUT_PULLUP` in firmware — OK because the board has **external 10 kΩ pull-ups**
  R4/R5/R6 to 3.3 V; confirmed in the schematic).
- Buttons are **active-low**: pressed reads `LOW` (`digitalRead(...) == LOW`,
  and penalty logic uses `!digitalRead(...)`).
- **`check_disconnect_button()`**: holding `BUTTON_GPIO` LOW for `DISCONNECT_HOLD_TIME`
  (5000 ms) calls `ble_disconnect()`.
- **`check_penalty_button()`**: a debounced (`DEBOUNCE_DELAY` 50 ms) **double-press**
  (within `DOUBLE_PRESS_MAX_DELAY` 1000 ms) of `BUTTON_GPIO` **OR** `BUTTON2_GPIO` calls
  `ble_msg_procesing_ask_for_penalty()` (sends `BLE_MSG_ASK_FOR_PENALTY`, only while
  `STM_PLAY`).

> **Shared-pin caution:** `BUTTON_GPIO` participates in both the disconnect long-press and
> the penalty double-press logic. A long hold could also register press edges in the penalty
> detector. Keep this in mind when redefining button roles for the 3-button board.

## UART / serial

- `UART_SPEED = 115200`. `Serial.begin()` at boot; used for `"PLAY"`/`"STOP"` debug prints.
- The V7 board exposes **two UARTs** on the U3 6-pin header (2541WV-06P):
  - **UART0** — `RX0`/`TX0` (net `RX_OUT`/`TX_OUT`, U3 pins 3–4). Primary serial, also the
    flashing/boot console.
  - **UART1** — `RX1 = IO4`, `TX1 = IO5` (U3 pins 5–6). Secondary serial to the robot.
- The 2024 README's RX/TX/LOGV/A0/A1 "channel" scheme is **not** present; the V7 board has
  plain UART0/UART1 instead. Firmware does **not** use UART1 yet.

## New V7/2026 hardware — CONFIRMED from schematic (NOT yet in firmware)

Pins below are from `ESP32C5_RCJ_modul_hardware_reference.md` (read pin-by-pin from U1) and
`pcb_schematic/SCH_Schematic1_2026-05-31.pdf`. Components seen on the 3D renders
(`.readme_images/3D_PCB1_2026-05-21 v4*.png`, silk "V7 2026", branding "robofuze").

| Feature | GPIO / part | Active level / drive | Firmware status |
|---------|-------------|----------------------|-----------------|
| RGB LED Red | **IO27** via R9 470 Ω | active **high** | not implemented |
| RGB LED Green | **IO24** via R8 470 Ω | active high | not implemented |
| RGB LED Blue | **IO23** via R7 470 Ω | active high | not implemented |
| RGB LED part | LED1 `TC5050RGBF08-3CJH-AF53A` | common-cathode, PWM-capable per channel | — |
| Buzzer | **IO26** (pin 27) → Q1 (BC817-40) base via R3 470 Ω | drive high; **passive 2.7 kHz** → use PWM ~2.7 kHz | implemented for match-state change beeps |
| Third button **B3** | **IO6** = SW2 | active low, 10 kΩ pull-up (R4) | not wired |
| UART1 RX / TX | **IO4 / IO5** (U3 pins 5–6) | — | not used |
| GPIO28 | **IO28** on H1 header pin 2 | — | exposed to robot; could sense power if external ckt added |
| Supercapacitor | C1 **15 F**, charge via R14 15 Ω, D3 SS34 | hardware ride-through | no firmware action needed |
| Power switch | U6 `MSS12C02LS-BB2.0` slide switch | enables/disables supercap backup | hardware-only |
| Buck converter | U4 `LMR51610XDBVR`, L=22 µH | VIN → 3.3 V | hardware-only |
| USB-C | USB1, D-=IO13, D+=IO14 (33 Ω series), CC 5.1 kΩ (UFP) | native USB-CDC, flashing + power | flashing path |

## Residual items to verify (low risk; do not block firmware re-pinning)

1. **OLED pull-ups.** No I2C pull-ups on-board; the OLED module is assumed to provide them
   (matches current working behavior). Verify when selecting a display.
2. **RGB LED part description** says "common cathode" while channels are driven **active
   high through a series resistor to the GPIO** — consistent with common-cathode; confirm
   polarity on first bring-up.

> Buzzer GPIO confirmed by maintainer (2026-05-31): **IO26 (pin 27)** — no longer in doubt.

## Source files reviewed

`definitions.h`, `functions.cpp`, `display.cpp`,
`docs/ai/ESP32C5_RCJ_modul_hardware_reference.md`,
`pcb_schematic/SCH_Schematic1_2026-05-31.pdf`, the three V7 render PNGs.

## Open questions

- Intended firmware roles for B3 (IO6), the RGB LED, the buzzer, and UART1.
</content>
