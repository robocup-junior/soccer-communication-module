# 00 — Project Overview

## What this module is

The RCJ Soccer Communication Module is a small mountable board for RoboCupJunior Soccer
robots. It provides a referee-controlled start/stop (and penalty/halftime/game-over)
channel between a mobile referee app and the robot, plus a status display. The board does
not count toward the robot weight limit (per public `README.md`).

Core responsibilities:

1. Advertise and accept a **BLE** connection from the mobile referee/control app.
2. Receive match commands over BLE and react.
3. Drive two digital **output pins** (`OUT1`, `OUT2`) that the robot continuously reads:
   **3.3 V = GO/PLAY, 0 V = STOP** (public README confirms the electrical meaning).
4. Show the current state on a 128×64 OLED, including a QR code of the BLE MAC address
   while waiting for a connection, and a penalty/halftime countdown.
5. Offer local buttons for disconnect and a self-penalty request back to the app.

## Match-level behavior

| Phase | What happens |
|-------|--------------|
| Power-on | Boot/logo screen with firmware version, then "Wait for connection" screen with QR + MAC |
| Referee connects (app) | BLE link established; app drives state |
| `PLAY` | Outputs go HIGH; robot plays; display shows team indicator + score |
| `STOP` | Outputs go LOW; robot stops |
| Penalty / damage | Outputs LOW; display shows a countdown; robot returns when timer hits 0 |
| Halftime | Outputs LOW; display shows a countdown |
| Game over | Outputs LOW; display shows final score + indicator |
| Self-penalty request | Local double-press of the button asks the app for a penalty (only while playing) |
| BLE disconnect | Returns to "Wait for connection" state |

The module is expected to be **powered at all times during the match** (even off-field) to
keep the BLE link stable (public README). The new hardware adds a **supercapacitor** so the
module can survive a short power drop without losing the connection/state (maintainer note).

## Control flow: app → BLE → module → robot

```
Mobile referee app
      │  BLE write (Nordic UART RX characteristic)
      ▼
ble.cpp  MyCallbacks::onWrite()  ── parses [msg_id][data...] ──► FreeRTOS queue
      │
      ▼
ble_processing.cpp  ble_msg_processing()  ── dispatch by msg_id ──►
      ├─ stm_set_state(STM_PLAY/STOP/DAMAGE/HALF_TIME/GAME_OVER)
      ├─ module_set_indicator() / module_set_my_score() / module_set_opponent_score()
      └─ ble_send_msg() for PING echo / FW_VERSION reply
      │
      ▼
state_machine.cpp  stm_update()
      ├─ update_output_satet(): OUTPUT1/OUTPUT2 GPIO HIGH (play) or LOW (stop)
      └─ display_screen_*(): render current screen
      │
      ▼
Robot reads OUT1/OUT2 (3.3 V = GO, 0 V = STOP)
```

The reverse direction (module → app) carries: `BLE_MSG_PING` echo, `BLE_MSG_FW_VERSION`
reply, `BLE_MSG_ASK_FOR_PENALTY` (self-penalty request), and `BLE_MSG_DISCONNECT`
notification before disconnecting. See [03_ble_protocol.md](03_ble_protocol.md).

## Old hardware vs new ESP32-C5 hardware

| Aspect | Old 2024 module | New module (silkscreen "V7 2026", "robofuze") |
|--------|-----------------|-----------------------------------------------|
| MCU | ESP32-**C6** (legacy pin block still commented in `definitions.h`) | ESP32-**C5** (PCB-antenna module) |
| Programming | External (per old docs) | **USB-C** direct, onboard |
| Power | BAT+ / GND (5.3–25 V) or 3V3 | Same + **supercapacitor backup**, ON/OFF slide switch |
| Inputs | Buttons (2 used in firmware) | **3 buttons** (silk "1", "2", "3"/"MF") |
| Feedback | OLED display | OLED + **RGB LED** + **buzzer** |
| Robot I/O | RX/TX UART, A0/A1 channels, LOGV, OUT1/OUT2 | OUT1/OUT2 confirmed in firmware; UART/RX-TX status **to verify** |

> The old/UART-based robot-to-robot communication (RX, TX, LOGV, A0/A1 channels) is
> described only in the public README/PDF. The **current firmware does not implement any
> RX/TX robot-to-robot UART protocol** — `Serial` is used at 115200 baud only for debug
> prints (`"PLAY"`/`"STOP"`) and as a side effect of `Serial.begin()`. The V7 schematic
> shows the board keeps UART headers — UART0 (RX0/TX0, primary/flashing) and UART1
> (RX1=IO4, TX1=IO5, secondary to robot) on the U3 connector — but the legacy LOGV/A0/A1
> channel scheme is gone. See [05_hardware_mapping.md](05_hardware_mapping.md).

## Current compatibility mode

The maintainer describes the firmware as running on ESP32-C5 in a **limited compatibility
mode**: it behaves like the old hardware with changed pins. Concretely, the only behavioral
delta vs the old code path that is implemented is:

- Pin remap to ESP32-C5 (`definitions.h`, C6 block commented out).
- BLE switched to NimBLE with tuned connection parameters and write-without-response
  (commit `ca23261` "Improve BLE command latency and resilience").
- A self-penalty request via double-press (commit `e423b0d`).

## Planned hardware features (NOT implemented yet)

These are on the board (confirmed in the V7 schematic) but have **no firmware support yet**;
pins are now known (see [05_hardware_mapping.md](05_hardware_mapping.md)):

- **RGB LED** — discrete common-cathode, active-high: R=IO27, G=IO24, B=IO23 (470 Ω each).
  ⚠️ **R/G are swapped on the real board** (footprint bug): IO27 lights **green**, IO24
  lights **red** (blue is fine). See [05](05_hardware_mapping.md) / the hardware reference.
- **Buzzer** — passive 2.7 kHz via NPN Q1 on **IO26** (pin 27); needs PWM tone.
- **Supercapacitor backup** — 15 F (C1); hardware ride-through, no firmware action required.
- **Third button** B3 = **IO6** (SW2) — firmware currently wires only two buttons
  (`BUTTON_GPIO`=IO10/B1, `BUTTON2_GPIO`=IO7/B2).
- **Slide power switch** (U6) — enables/disables supercap backup; firmware-invisible.

Do not implement these until the schematic confirms pins. See
[10_future_change_plan.md](10_future_change_plan.md).

## Source files reviewed

`README.md`, `firmware/RCj_comm_module/*` (all `.ino/.cpp/.h`), `main/app_main.cpp`,
`.readme_images/3D_PCB1_2026-05-21 v4*.png`, git history (`7f9fb81`, `ca23261`, `e423b0d`).

## Open questions

- Does the new board still expose a robot-to-robot UART (RX/TX/LOGV/A0/A1) header?
- Must old-module BLE compatibility be preserved bit-for-bit with the existing app?
- Is the supercapacitor purely hardware ride-through, or should firmware detect a power dip?
</content>
