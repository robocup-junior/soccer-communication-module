# RCJ Soccer Communication Module

<p align="center">
  <img src="./.readme_images/3D_PCB1_2026-05-21%20v41.png?raw=true" alt="RCJ Soccer communication module" width="600">
</p>

A small referee-to-robot interface for **RoboCupJunior Soccer**. A referee controls the module over **Bluetooth Low Energy (BLE)** from the mobile app, and the module relays the live match state — **PLAY, STOP, penalty, half-time, game over** — to the robot or host controller over simple digital outputs, UART, or USB serial.

The current module (board **V7 / 2026**) is built around an **ESP32-C5** and includes an OLED display, three buttons, an RGB LED, a buzzer, a USB-C port for power & flashing, and an on-board **supercapacitor** backup. It does **not** count toward the robot weight limit.

> 📦 **Got an older 2024 board?** That one uses an ESP32-**C6** — see [Legacy 2024 (ESP32-C6) modules](#-legacy-2024-esp32-c6-modules) below.

---

## 🚀 Get started

**1. Install the referee app** — Android, free on Google Play:

**➡️ [RCJ Soccer app on Google Play](https://play.google.com/store/apps/details?id=com.robocup.rcj_soccer)**

**2. Flash the module** — easiest is the **web flasher**, nothing to install:

**➡️ [Web flasher](https://robocup-junior.github.io/soccer-communication-module/)** — connect the module over USB-C and flash straight from your browser.

> The web flasher uses the Web Serial API, so use **desktop Chrome or Edge**.

**3. Wire it to your robot** — power the module from the robot battery and read one start/stop signal (see [below](#-connecting-it-to-your-robot)).

---

## 📱 Using the app

The app connects to the module over BLE and drives the whole match. The interaction pattern:

- **Double-click** for actions — e.g. double-click a robot to start/stop it, or to issue a penalty.
- **Hold** for settings — e.g. hold a robot button for connection settings.
- **Start/stop** resets all timers and starts/stops all robots at once.
- **Score:** double-click a score to add a point; hold the score number for −1.

The app's source code lives in its own repository: **[robocup-junior/soccer-referee-app](https://github.com/robocup-junior/soccer-referee-app)**.

---

## 🔌 Connecting it to your robot

### Read the start/stop signal (pick one method)

The robot must respond to the start/stop signal for the entire game. Use **one** of the following methods and react whenever the state changes:

| Method         | Pin                            | PLAY / GO        | STOP        |
|----------------|--------------------------------|------------------|-------------|
| **Output pin** | `OUT1` or `OUT2`               | **3.3 V** (HIGH) | **0 V** (LOW) |
| **UART**       | `TX0` (UART0 on the U3 header) | sends `PLAY`     | sends `STOP` |
| **USB serial** | USB-C                          | sends `PLAY`     | sends `STOP` |

Both outputs carry the same 3.3 V logic-level signal. For 5 V robot inputs, add a level shifter before the robot. Always share **GND** with the robot controller — never connect VIN to a robot GPIO or to the 3.3 V pin.

For a Raspberry Pi or another USB host, connect the module's USB-C port to the host's USB-A port. The ESP32-C5 enumerates as a serial device, typically `/dev/ttyACM0`, and prints one line (`PLAY` or `STOP`) whenever the referee app changes the match state.

On Linux, you can check the USB serial output with:

```sh
stty -F /dev/ttyACM0 115200 raw -echo
cat /dev/ttyACM0
```

The onboard buzzer gives a short confirmation beep whenever the referee app changes the match state.
The RGB status LED is dimmed green in PLAY and dimmed red when the robot output is stopped.

### Power it

Power the module **directly from the robot battery** so it stays alive for the whole match — even when the robot is switched off or removed from the field — to keep a stable BLE connection.

- **Battery:** connect **VIN** to battery **+** and **GND** to battery **−** (no switch in between). VIN accepts **4.5 V – 50 V** (the on-board buck regulator is rated higher, but RoboCupJunior rules cap robot voltage at 50 V).
- **USB-C:** the module can also run from USB-C (VBUS).
- **Regulated 3.3 V:** you may instead feed **3.3 V** to the 3V3 pin. If you do, design the supply to cover a short **startup demand of ~0.5 A at 3.3 V (≈1.65 W)**; steady-state draw is far lower.

### Supercapacitor backup

The back-side **supercapacitor** rides through short voltage dips during play (e.g. a battery sag). Switch the **supercapacitor switch ON before the match**, and **OFF when the module is not in use** so it doesn't stay charged in storage.

### Mounting

Mount the module where the **display stays visible** and the connector is **easy to reach** — typically on top of the robot via a hub, either on your own PCB or on protoboard/breadboard. We can also provide a limited number of **hub boards** you can permanently attach to your robot ([hub photo](./.readme_images/hub_image.png?raw=true)).

---

## ⏱️ During the match

### Penalties (fast & quiet)

To penalize a robot, **double-press the large button** on the module. The module sends a penalty request to the referee app, so the right robot is identified and the penalty timer starts — no shouting robot designators across the field. The firmware detects the double-press and sends the request **only while the robot is in the PLAY state**.

### Putting robots back in

The OLED shows a **countdown** for the duration of a robot's penalty. Teams may return robots to play, per the rules, once the penalty time is up on the display.

---

## 🤖 Robot-to-robot communication

Direct robot-to-robot communication is **planned for a future firmware release**. The current firmware relays the referee's match state (start/stop, penalty, half-time, game over) only — via the output pins or UART described above.

> The old 2024 board's channel-select `A0`/`A1` pins and the `LOGV` UART-level pin are **not** present on this hardware.

---

## 🛠️ Firmware & flashing (details)

For most users the [web flasher](https://robocup-junior.github.io/soccer-communication-module/) is all you need. Under the hood:

The firmware is an **ESP-IDF** project (Arduino runs as an IDF component). Releases are built automatically from Git tags matching `fw-*` or `v*` — pushing a tag makes GitHub Actions build the ESP32-C5 firmware, publish a GitHub Release with the `.bin` files, and deploy the latest web flasher to GitHub Pages:

```sh
git tag fw-v0.97
git push origin fw-v0.97
```

The release includes a single **merged image** (bootloader + partition table + app) plus the individual bins for `idf.py`/esptool users.

---

## 🧩 Legacy 2024 (ESP32-C6) modules

The older 2024 board uses an **ESP32-C6** (no USB-C connector) instead of the current ESP32-C5. It is still supported with a final, maintained firmware on the **[`legacy/esp32-c6`](https://github.com/robocup-junior/soccer-communication-module/tree/legacy/esp32-c6)** branch. Those builds are published as **GitHub Release assets** (tags `legacy-c6-*`); there is no web flasher for them.

**Flashing (native USB — recommended):** the C6's USB D+/D− are broken out on the programming header. Wire them to a USB port (the module enumerates as a serial device), then flash the merged image with full erase:

```sh
esptool --chip esp32c6 -p <PORT> write_flash --erase-all 0x0 rcj_comm_module-<ver>-merged.bin
```

A UART path also exists via the level-shifted RX/TX header pins, but it needs the `LOGV` pin powered at 3.3 V and manual download-mode entry (hold IO9/BOOT low, pulse EN), so native USB is easier. Either path can use the special flashing jig PCB.

---

## ❓ Not working?

Check the [open issues](https://github.com/robocup-junior/soccer-communication-module/issues) first and — if nobody has posted yours yet — open a [new GitHub issue](https://github.com/robocup-junior/soccer-communication-module/issues/new), post on [the forum](https://junior.forum.robocup.org/c/robocupjunior-soccer/5), or ask on the [RoboCupJunior Discord](https://discord.gg/45pxMQY4nJ).

## 🙌 Could use some help with

- An adapter hub for EV3 / SPIKE
- More component libraries (footprint / schematic / 3D) for Altium, Eagle, KiCad, EasyEDA, OrcaCAD, …
- Reporting errors & testing

## 🔭 Currently working on

- Robot-to-robot communication
- Additional RGB LED feedback patterns

## 🏆 Hall of fame

- App by Mato Faltus
- Altium lib by Youssef Shaalan

---

## 📄 Quick connection guide

A printable one-page (A5) summary of the pinout, signals, power, and wiring:

![RCJ Soccer Communication Module — quick connection guide](./.readme_images/rcj_soccer_comm_module_flyer_A5.png?raw=true)
