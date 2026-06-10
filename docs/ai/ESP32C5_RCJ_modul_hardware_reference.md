# ESP32C5_RCJ_modul — Hardware Reference for AI Coding Agents

**Schematic:** Schematic1
**Board:** Board1
**Version:** V1.0
**Created:** 2025-12-10 | **Last updated:** 2026-02-21
**Tool:** EasyEDA

---

## Overview

This is a compact robot controller module built around the **ESP32-C5-WROOM-1** (the schematic symbol is labeled `-N16R4`, but the flash actually fitted reads **8 MB** via JEDEC ID — see the note under "Microcontroller — U1"). It is designed to be mounted on a robot via two pin headers and programmed over USB-C. The board includes onboard buttons, an RGB LED, a buzzer, an I2C expansion port for an OLED display, and a supercapacitor-backed power system with a soft power switch.

---

## Microcontroller — U1

| Parameter | Value |
|-----------|-------|
| Part | ESP32-C5-WROOM-1 (schematic label `-N16R4`) |
| Architecture | RISC-V single-core, Wi-Fi 6, BLE 5 |
| Flash | **8 MB** (measured) — see note |
| PSRAM | 4 MB |
| Package | 38-pin module |

The module is the central processing unit. All peripherals connect to it through the GPIO assignments listed below.

> ⚠️ **Flash size: schematic says N16R4 (16 MB), but the physical chip reads 8 MB.**
> `esptool flash_id` on a real board (2026-05-31) reports manufacturer `0x20`, device
> `0x4017` → size byte `0x17` = 2²³ = **8 MB**, and the boot ROM auto-detects `8192k`. The
> firmware build is therefore configured for **8 MB** (`CONFIG_ESPTOOLPY_FLASHSIZE_8MB` in
> `firmware/RCj_comm_module/sdkconfig.defaults`). Treat 8 MB as ground truth; the `-N16R4`
> marking is either a BOM/labeling discrepancy or a different flash variant was stuffed.

---

## GPIO Pin Assignment

Read directly from the U1 (ESP32-C5-WROOM-1-N16R4) schematic symbol pin-by-pin.

| GPIO | Module Pin | Net Label | Function | Notes |
|------|-----------|-----------|----------|-------|
| IO2  | 4  | SDA    | I2C SDA           | OLED display data — H2 header |
| IO3  | 5  | SCL    | I2C SCL           | OLED display clock — H2 header |
| IO4  | 17 | RX1    | UART1 RX          | Secondary serial from robot (U3 connector) |
| IO5  | 16 | TX1    | UART1 TX          | Secondary serial to robot (U3 connector) |
| IO6  | 8  | B3     | Button B3         | SW2, pulled up to 3.3V via R4 10kΩ, active low |
| IO7  | 9  | B2     | Button B2         | SW1, pulled up to 3.3V via R5 10kΩ, active low |
| IO8  | 10 | OUT2   | Robot output 2    | U3 connector pin OUT2 |
| IO9  | 11 | OUT1   | Robot output 1    | U3 connector pin OUT1 |
| IO10 | 12 | B1     | Button B1         | SW5, pulled up to 3.3V via R6 10kΩ, active low |
| IO13 | 13 | GPIO13 | USB D-            | USB1 via R10 33Ω |
| IO14 | 14 | GPIO14 | USB D+            | USB1 via R11 33Ω |
| IO23 | 20 | B      | LED Blue          | LED1 via R7 470Ω, active high |
| IO24 | 22 | G      | LED Green ⚠️ *(lights **Red** — see LED1 note)* | LED1 via R8 470Ω, active high |
| IO26 | 27 | BUZZER | Buzzer driver     | Q1 base via R3 470Ω, drive high to activate |
| IO27 | 18 | R      | LED Red ⚠️ *(lights **Green** — see LED1 note)* | LED1 via R9 470Ω, active high |
| IO28 | 15 | GPIO28 | Robot GPIO        | H1 header pin 2, exposed to robot connector |
| RX0  | 24 | RX_OUT | UART0 RX          | U3 connector; primary serial / flashing |
| TX0  | 25 | TX_OUT | UART0 TX          | U3 connector; primary serial / flashing |

> **Note:** NC/IO15 (pin 19), IO0 (pin 6), IO1 (pin 7) are not connected or reserved. Always verify against the EasyEDA netlist. (IO26 / pin 27 **is** used — it drives the buzzer.)

---

## Buttons

Three tactile push buttons, all using **TS-1088-AR02016** footprint. All are **active low** (pulled up to 3.3V).

| Designator | Net Label | GPIO | Description |
|------------|-----------|------|-------------|
| SW5 | B1 | IO10 | Power/mode switch (TS-1002S-07026C, 4-pin) |
| SW1 | B2 | IO7  | General-purpose button |
| SW2 | B3 | IO6  | General-purpose button |

**Firmware note:** Configure all button pins as `INPUT` with internal or external pull-up. Debounce in software (recommended: 20–50 ms). SW5 is physically a 4-pin slide/toggle switch (TS-1002S-07026C), while SW1 and SW2 are standard momentary tactile buttons.

---

## RGB LED — LED1

> ⚠️ **Red/Green are SWAPPED on the real board — schematic footprint bug.**
> The `TC5050RGBF08-3CJH-AF53A` part's physical pin order is **G, R, B**, but the LED1
> footprint in the schematic is wired as **R, G, B**, so the Red and Green pads are
> reversed on the PCB. The labels below (and the "Schematic" column) reflect the
> *schematic*; the **As-built** column is what actually lights up. Confirmed on hardware
> **2026-06-02** by flashing firmware that drives play→green and seeing red instead.
> Blue is unaffected.

| Parameter | Value |
|-----------|-------|
| Part | TC5050RGBF08-3CJH-AF53A |
| Type | Common cathode RGB LED |
| Current limiting | R7, R8, R9 — all 470Ω to 3.3V rail |

| GPIO  | Resistor | Schematic label | **As-built (real) color** |
|-------|----------|-----------------|---------------------------|
| IO27  | R9 470Ω  | Red             | 🟢 **Green** |
| IO24  | R8 470Ω  | Green           | 🔴 **Red** |
| IO23  | R7 470Ω  | Blue            | 🔵 Blue (correct) |

**Firmware note:** Each channel is driven independently; drive the GPIO high to turn the
color on, and PWM is supported on all three for brightness/mixing. **Because of the
footprint swap, firmware must drive IO24 to show red and IO27 to show green** (until the
footprint is corrected in a future board revision — the proper long-term fix).

---

## Buzzer — BUZZER1

| Parameter | Value |
|-----------|-------|
| Part | Passive buzzer, 2.7 kHz resonant frequency |
| Driver | NPN transistor Q1 (BC817-40) |
| Base resistor | R3 470Ω |
| Flyback diode | D1 (1N4148W-7-F) |
| GPIO | IO26 |

**Firmware note:** Drive IO26 high to activate. For passive buzzers, generate a PWM signal at or near 2.7 kHz for maximum volume. The transistor sinks current through the buzzer; D1 protects against inductive spikes.

---

## I2C Interface — OLED Display

| Parameter | Value |
|-----------|-------|
| Header | H2 (PZ254V-11-04P, 4-pin) |
| SDA | IO1 (pin 1 of H2) |
| SCL | IO2 (pin 2 of H2) |
| Power | 3.3V (pin 3 of H2) |
| GND | GND (pin 4 of H2) |

**Firmware note:** Standard I2C bus. Typical OLED displays (SSD1306, SH1106) operate at 3.3V and use address `0x3C` or `0x3D`. Use I2C clock speed up to 400 kHz (Fast Mode). No onboard pull-up resistors are visible on the schematic — the OLED module likely includes them, but verify when selecting a display.

---

## USB-C Connector — USB1

| Parameter | Value |
|-----------|-------|
| Part | TYPE-C 16PIN 2MD(073) |
| D+ | GPIO14 via R11 33Ω |
| D- | GPIO13 via R10 33Ω |
| CC resistors | R12, R13 — both 5.1kΩ to GND (USB-C UFP / device mode) |
| Protection diode | D2 (1N5819HW-7-F) on VBUS line |

The USB-C port is used for:
- **Flashing firmware** via USB Serial (USB-CDC or UART boot)
- **Power input** (VBUS → VIN rail via D2)

The 5.1kΩ CC resistors configure the connector as a **UFP (device)**, so it is recognized as a standard USB device by host computers. No host/OTG mode.

---

## Power Supply System

### Input Path

```
USB-C VBUS ──D2──► VIN rail
                        │
                   U4 (LMR51610XDBVR) Buck converter
                        │
                      3.3V rail ──► all logic
```

### Buck Converter — U4

| Parameter | Value |
|-----------|-------|
| Part | LMR51610XDBVR (Texas Instruments) |
| Topology | Synchronous step-down (buck) |
| Input | VIN (from USB-C or robot header) |
| Output | 3.3V |
| Inductor | U5 22µH |
| Input caps | C2 2.2µF, C3 2.2µF, C4 100nF |
| Output caps | C5 10µF, C6 10µF, C7 10µF |
| Feedback cap | C8 100nF |
| Feedback divider | R1 100kΩ (top), R2 32kΩ (bottom) |

The feedback divider sets the output voltage. Verify R1/R2 values against the LMR51610 datasheet if output voltage needs to be changed.

### Supercapacitor Backup — C1

| Parameter | Value |
|-----------|-------|
| Part | C1 supercapacitor |
| Capacitance | 15F |
| Purpose | Hold power rail during brief supply interruptions |
| Charge path | Via R14 15Ω (current-limiting resistor) |
| Discharge protection | D3 (SS34 Schottky diode) |
| Switch | MSS12C02LS-BB2.0 (U6) — slide switch for enabling/disabling backup |

**Firmware note:** The supercapacitor provides ride-through during power dropout (e.g., when robot battery dips). No firmware action is required to benefit from it. GPIO28 is exposed on H1 and could be used to monitor power status if external circuitry is added.

---

## Robot Interface Connectors

### H1 — 4-pin Header (PZ254V-11-04P)

| Pin | Signal |
|-----|--------|
| 1 | GND |
| 2 | GPIO28 |
| 3 | 3.3V |
| 4 | VIN |

Used to expose the raw input voltage and GPIO28 to the robot mainboard.

### U3 — 6-pin Header (2541WV-06P)

| Pin | Signal | Direction |
|-----|--------|-----------|
| 1 | OUT1 | Output to robot |
| 2 | OUT2 | Output to robot |
| 3 | RX_OUT | UART0 TX (from ESP32 perspective: transmit out) |
| 4 | TX_OUT | UART0 RX (from ESP32 perspective: receive in) |
| 5 | RX1 | UART1 RX |
| 6 | TX1 | UART1 TX |

> **Important:** The net labels RX_OUT / TX_OUT reflect the signal direction **from the module's perspective**. When wiring to a robot MCU, cross RX↔TX as usual.

### H2 — 4-pin Header (PZ254V-11-04P)

Dedicated I2C header for OLED display (see I2C section above).

---

## Communication Interfaces Summary

| Interface | GPIOs | Connector | Usage |
|-----------|-------|-----------|-------|
| USB Serial (UART0) | IO13 (D-), IO14 (D+) | USB-C | Firmware flashing, debug console |
| UART0 | RX0 / TX0 | U3 pins 3–4 | Primary serial to robot |
| UART1 | RX1 / TX1 | U3 pins 5–6 | Secondary serial to robot |
| I2C | IO1 (SDA), IO2 (SCL) | H2 | OLED display |

---

## Component Reference Table

| Designator | Part | Value / Description |
|------------|------|---------------------|
| U1 | ESP32-C5-WROOM-1-N16R4 | Main MCU module |
| U4 | LMR51610XDBVR | Buck converter, 3.3V output |
| U5 | Inductor | 22µH, buck converter |
| U6 | MSS12C02LS-BB2.0 | Slide switch for supercap backup |
| USB1 | TYPE-C 16PIN 2MD(073) | USB-C connector |
| H1 | PZ254V-11-04P | 4-pin robot power/GPIO header |
| H2 | PZ254V-11-04P | 4-pin I2C display header |
| U3 | 2541WV-06P | 6-pin robot UART/IO header |
| LED1 | TC5050RGBF08-3CJH-AF53A | RGB LED |
| BUZZER1 | — | Passive buzzer, 2.7 kHz |
| Q1 | BC817-40 | NPN transistor, buzzer driver |
| SW1, SW2 | TS-1088-AR02016 | Tactile push buttons |
| SW5 | TS-1002S-07026C | 4-pin slide/toggle switch |
| C1 | — | 15F supercapacitor |
| D1 | 1N4148W-7-F | Flyback diode (buzzer) |
| D2 | 1N5819HW-7-F | VBUS protection diode |
| D3 | SS34 | Schottky diode, supercap discharge path |
| R1 | 100kΩ | Buck feedback top |
| R2 | 32kΩ | Buck feedback bottom |
| R3 | 470Ω | Buzzer transistor base |
| R4, R5, R6 | 10kΩ | Button pull-up resistors |
| R7, R8, R9 | 470Ω | RGB LED current limiting |
| R10, R11 | 33Ω | USB D+/D- series resistors |
| R12, R13 | 5.1kΩ | USB-C CC pull-down (UFP) |
| R14 | 15Ω | Supercap charge current limiter |
| C2, C3 | 2.2µF | Buck input capacitors |
| C4, C8 | 100nF | Decoupling |
| C5, C6, C7 | 10µF | Buck output capacitors |
| C9 | 10µF | ESP32 3.3V decoupling |

---

## Firmware Quick-Start Checklist

- [ ] Set IO27, IO24, IO23 as `OUTPUT` for R/G/B LED channels — ⚠️ **R/G swapped on the real board: IO27 lights green, IO24 lights red** (footprint bug, see "RGB LED — LED1")
- [ ] Set IO10, IO7, IO6 as `INPUT` (pull-up) for buttons B1/B2/B3 (active low)
- [ ] Set IO26 as `OUTPUT` for buzzer; use PWM at ~2700 Hz
- [ ] Initialize I2C on SDA=IO2, SCL=IO3 for OLED
- [ ] Set IO9, IO8 as `OUTPUT` for OUT1/OUT2 robot signals
- [ ] UART0 is the USB serial port (flashing + debug)
- [ ] UART1 (RX1/TX1) available for robot communication via U3 header
- [ ] GPIO28 exposed on H1 — available for robot-side signaling
