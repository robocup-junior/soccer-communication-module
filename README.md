# Communication modules for RCJ Soccer SuperTeams

![RCJ Soccer communication module](./.readme_images/3D_PCB1_2026-05-21%20v41.png?raw=true)

## What / Why it is?
To make RCJ Soccer SuperTeam games more manageable for referees and to bring a simple and robust way of robot-to-robot / robot-to-referee communication, we would like to introduce these modules. This module does not count toward the weight limit.

The current module (board **V7 / 2026**) is built around an **ESP32-C5**. A referee controls it over **Bluetooth Low Energy (BLE)** from the mobile app, and the module relays the match state (start/stop, penalty, half-time, game over) to the robot. It has an OLED display, three buttons, an RGB LED, a buzzer, a supercapacitor power backup, and a USB-C port for power and firmware flashing.

## How to power it?
Power the module from your robot battery through the power header: connect **GND** to battery negative (−) and **VIN** to battery positive (+), with **no switch** in between. The input accepts **4.5 V – 50 V** (the on-board buck regulator is rated higher, but RoboCupJunior rules cap robot voltage at 50 V).

Alternatively you can supply **3.3 V** directly to the 3V3 pin, or power the board over **USB-C** (VBUS). The module must be able to draw at least **500 mA** at all times.

The module needs to be powered for the whole match — even when the robot is off the field (out of bounds, damage) — so it can keep a stable communication connection. A small on-board **supercapacitor** rides through brief supply dropouts (e.g. a battery dip).

## How to read the start/stop signal?
The robot must respond to the start/stop signal at all times for the duration of the game. There are two ways to read it:

- **Output pins** — read `OUT1` or `OUT2`, where **3.3 V = GO** and **0 V = STOP**.
- **UART** — the module also prints `PLAY` / `STOP` on its serial line (UART0 on the U3 header) whenever the state changes, for robots that prefer to read it over serial.

## How to use it for communication between robots?
Direct robot-to-robot communication is **planned for a future firmware release**. The current firmware relays the referee's start/stop (and penalty/half-time/game-over) state only — via the output pins or UART as described above. (The earlier channel-select `A0`/`A1` pins and the `LOGV` UART-level pin from the 2024 board are **not** present on this hardware.)

## How to put robots back in the game?
The module has a display that shows a countdown for the duration of a robot's penalty. Teams may put robots back in the game, according to the rules, when the penalty time is up on the display.

## How to mount it?
Mount the module on the robot so it can be easily connected/disconnected and the display stays visible at all times. We recommend mounting it on top of the robot and making a hub for it — either directly on your PCB or using protoboard/breadboard. We can also provide a limited number of hub boards that you can permanently attach to your robot.

![hub photo](./.readme_images/hub_image.png?raw=true)

## Mobile app
The referee/control app is available on Google Play:

**➡️ [RCJ Soccer app on Google Play](https://play.google.com/store/apps/details?id=com.robocup.rcj_soccer)**

The app's source code lives in its own repository: **[robocup-junior/soccer-referee-app](https://github.com/robocup-junior/soccer-referee-app)**.

Usage:
- **Double-click** for actions (e.g. double-click the robot button to start/stop/penalty a robot).
- **Hold** for settings (e.g. hold the robot button for connection settings).
- Use start/stop to reset all timers and start/stop all robots.
- **Double-click** a score to add a point; **hold** a score number for −1.

## Not working?
If you have problems or questions, check the existing issues and — if nobody has posted yours yet — open a [GitHub issue](https://github.com/robocup-junior/soccer-communication-module/issues/new), post on [the forum](https://junior.forum.robocup.org/c/robocupjunior-soccer/5), or ask on the [RoboCupJunior Discord server](https://discord.gg/45pxMQY4nJ).

## Firmware & flashing
The easiest way to flash the current module is the **web flasher** (no tools to install):

**➡️ [Web flasher](https://robocup-junior.github.io/soccer-communication-module/)** — connect the module over USB-C and flash from the browser.

> The web flasher uses the Web Serial API, so you need **desktop Chrome or Edge**.

Firmware releases are built automatically from Git tags matching `fw-*` or `v*`. Pushing a tag makes GitHub Actions build the ESP32-C5 firmware, publish a GitHub Release with the `.bin` files, and deploy the latest web flasher to GitHub Pages:

```sh
git tag fw-v0.97
git push origin fw-v0.97
```

## Legacy 2024 (ESP32-C6) modules
The older 2024 board uses an **ESP32-C6** (no USB-C connector) instead of the current ESP32-C5. It is still supported with a final, maintained firmware on the **[`legacy/esp32-c6`](https://github.com/robocup-junior/soccer-communication-module/tree/legacy/esp32-c6)** branch. These builds are published as **GitHub Release assets** (tags `legacy-c6-*`); there is no web flasher for them.

**Flashing (native USB — recommended):** The C6's USB D+/D− are broken out on the module's programming header. Wire them to a USB port (the module enumerates as a serial device), then flash the single merged image with full erase:

```sh
esptool --chip esp32c6 -p <PORT> write_flash --erase-all 0x0 rcj_comm_module-<ver>-merged.bin
```

A UART path also exists via the level-shifted RX/TX header pins, but it needs the `LOGV` pin powered at 3.3 V and manual download-mode entry (hold IO9/BOOT low, pulse EN), so native USB is easier. Either path can use the special flashing jig PCB.

## Could use some help with:
  * Make an adapter hub for EV3/SPIKE
  * Adding more libraries with footprint/schematic/3D for Altium, Eagle, KiCad, EasyEDA, OrcaCAD and more …
  * Reporting errors / testing

## Currently working on:
   * Robot-to-robot communication
   * RGB LED & buzzer feedback

# Hall of fame
* App by Mato Faltus
* Altium lib by Youssef Shaalan
