# 02 — Firmware Architecture

## Module breakdown

| Module | Files | Responsibility |
|--------|-------|----------------|
| Entry point (Arduino) | `RCj_comm_module.ino` | `setup()` / `loop()` for Arduino-IDE builds |
| Entry point (ESP-IDF) | `main/app_main.cpp` | `app_main()` → `initArduino()` + setup/loop, for CI/IDF builds |
| Definitions | `definitions.h` | Versions, BLE/UART constants, GPIO pin map |
| BLE transport | `ble.cpp` / `ble.h` | NimBLE server, NUS UUIDs, RX/TX characteristics, callbacks, send/disconnect |
| BLE processing | `ble_processing.cpp` / `ble_processing.h` | `ble_msg_t`, `ble_msg_id` enum, RTOS queue, command dispatch |
| State machine | `state_machine.cpp` / `state_machine.h` | `stm_states`, output-pin control, timer |
| Display | `display.cpp` / `display.h` | SSD1306 rendering per state |
| Functions/util | `functions.cpp` / `functions.h` | MAC string, score/indicator globals, GPIO init, button handling |
| Assets | `fonts.h`, `images.h` | OLED fonts and boot logo (`RC_logo`) |

## Two parallel entry points (IMPORTANT)

There are **two** entry points that do the same four things in the same order:

`RCj_comm_module.ino` (Arduino IDE):
```c
void setup() { Serial.begin(UART_SPEED); display_init(); module_init_gpios(); stm_init(); ble_start_server(); }
void loop()  { ble_msg_processing(); stm_update(); check_disconnect_button(); check_penalty_button(); }
```

`main/app_main.cpp` (ESP-IDF — what CI actually builds):
```c
static void module_setup() { Serial.begin(UART_SPEED); display_init(); module_init_gpios(); stm_init(); ble_start_server(); }
static void module_loop()  { ble_msg_processing(); stm_update(); check_disconnect_button(); check_penalty_button(); }
extern "C" void app_main(void) { initArduino(); module_setup(); while (true) { module_loop(); delay(1); } }
```

> **Maintenance hazard:** any change to setup/loop logic must be mirrored in **both** files,
> or the Arduino-IDE and ESP-IDF builds will diverge. The release pipeline builds the
> **ESP-IDF** path (`main/app_main.cpp`); the `.ino` is for local Arduino-IDE use.

## Initialization sequence

1. `Serial.begin(UART_SPEED)` — `UART_SPEED = 115200`. Used for debug prints only.
2. `display_init()` — `SSD1306 display(0x3c, I2C_SDA_GPIO, I2C_SCL_GPIO)`, `display.init()`.
3. `module_init_gpios()` — `OUTPUT1_GPIO`/`OUTPUT2_GPIO` as `OUTPUT`; `BUTTON_GPIO`/`BUTTON2_GPIO` as `INPUT` (no explicit pull configured — see hardware doc).
4. `stm_init()` — current state stays `STM_INIT`; sets timer to `660000 ms` (see below / [04](04_state_machine.md)).
5. `ble_start_server()` — creates the message queue, BLE device, server, service, RX/TX characteristics, and starts advertising.

## Main loop behavior

`loop()` runs continuously (in IDF: `while(true) { ...; delay(1); }`). Each iteration:

1. **`ble_msg_processing()`** — pops at most **one** message from the queue (`xQueueReceive`
   with timeout `0`) and dispatches it. Non-blocking; returns immediately if empty.
2. **`stm_update()`** — runs the current state's handler and, on a state change, updates
   the output pins.
3. **`check_disconnect_button()`** — long-press (`DISCONNECT_HOLD_TIME = 5000 ms`) on
   `BUTTON_GPIO` triggers `ble_disconnect()`.
4. **`check_penalty_button()`** — double-press on `BUTTON_GPIO` **or** `BUTTON2_GPIO`
   triggers `ble_msg_procesing_ask_for_penalty()`.

There is **no explicit task split**: everything runs in the single `app_main`/loop thread.
BLE callbacks run in the NimBLE host context and only enqueue work, keeping the ISR/callback
path short.

## Concurrency / FreeRTOS

The only cross-context shared structure is the BLE message queue.

- Created in `ble_msg_processing_init()`:
  `xQueueCreate(BLE_QUEUE_MAX_SIZE /*16*/, sizeof(ble_msg_t))`.
- **Producer:** `ble.cpp` `MyCallbacks::onWrite()` → `queue_ble_msg()` (BLE host context).
- **Consumer:** `ble_processing.cpp` `ble_msg_processing()` (main loop).

`queue_ble_msg()` (added in `ca23261`) implements an **overwrite-on-full** policy: if the
queue is full it drops the **oldest** message (`xQueueReceive`) and retries the send, so the
newest referee command is never lost to a backlog. Both `ble.cpp` and `ble_processing.cpp`
hold their own `static QueueHandle_t ble_msg_queue`; `ble.cpp` fetches the handle via
`ble_msg_proccesing_get_queue()` during `ble_start_server()`.

Other shared globals (`current_state`, `robot_play`, `timer_stop`, `module_indicator`,
`my_score`, `opponent_score`, `device_connected`) are accessed without locks. In practice
state mutations happen either in the loop or in BLE callbacks; `stm_set_state()` is called
from both `ble_processing` (loop) and `MyServerCallbacks::onDisconnect` (BLE context) —
a benign data race on `current_state`/`state_changed` that has not caused observed issues
but is worth noting for any future hardening.

## Important global state

| Variable | File | Meaning |
|----------|------|---------|
| `current_state` | `state_machine.cpp` | Active `stm_states` value |
| `state_changed` | `state_machine.cpp` | One-shot flag: forces re-render + output update next `stm_update()` |
| `robot_play` | `state_machine.cpp` | Drives `OUTPUT1/2` HIGH/LOW |
| `timer_stop` | `state_machine.cpp` | `millis()` deadline for penalty/halftime countdown |
| `device_connected` | `ble.cpp` | BLE connection state |
| `module_indicator` | `functions.cpp` | 2-char team/robot label (default `"--"`) |
| `my_score`, `opponent_score` | `functions.cpp` | Scoreboard values |

## Legacy UART / Serial behavior

- `Serial.begin(115200)` is called at startup.
- `update_output_satet()` (`state_machine.cpp:32,36`) prints `"PLAY"` / `"STOP"` to `Serial`
  on every output change. **These are the only active serial prints in the firmware** — they
  double as a UART status channel for robots that read the serial line instead of the OUT
  pins.
- All other debug prints are **already commented out**: `ble.cpp:45,64,147,149`,
  `ble_processing.cpp:71`, `functions.cpp:107,121`. (Verified 2026-05-31.) The vendored
  `libraries/**/examples/*.ino` prints are not compiled; the OLED lib's `"[deprecated]"`
  prints live in `drawLogBuffer()`/`setLogBuffer()`, which the firmware never calls.
- **No robot-to-robot UART protocol** (RX/TX framing, channel select) exists in the current
  firmware, despite the public README describing RX/TX/LOGV/A0/A1 on the 2024 board.

### UART output cleanliness (for robots that read serial, not pins)

Verified against the generated build config (`build/config/sdkconfig.h`, 2026-05-31):

1. **✅ `Serial` routes to UART0 (the U3 `TX_OUT` the robot reads).** `ARDUINO_USB_CDC_ON_BOOT`
   is **not** defined, so `Serial` = `HardwareSerial(0)` = UART0; and `CONFIG_ESP_CONSOLE_UART_DEFAULT=y`
   with `CONFIG_ESP_CONSOLE_UART_NUM=0`. So `PLAY`/`STOP` do reach the robot's UART pins.
   (`ARDUHAL` log level is already ERROR-only, so the Arduino layer adds no chatter.)
2. **✅ Boot log silenced for release (fixed 2026-05-31).** `sdkconfig.defaults` now sets
   `CONFIG_BOOTLOADER_LOG_LEVEL_NONE=y`, `CONFIG_LOG_DEFAULT_LEVEL_NONE=y`,
   `CONFIG_ARDUHAL_LOG_DEFAULT_LEVEL_NONE=y`, and pins the console to UART
   (`CONFIG_ESP_CONSOLE_UART_DEFAULT=y`). So a release build's UART0 line carries only the
   Arduino `Serial` output (`PLAY`/`STOP`) — those are plain UART writes, not log macros, so
   they survive. The ROM's very first line (`ESP-ROM:…`, reset cause) is emitted before
   sdkconfig applies and can only be removed via eFuse — **accepted as-is for now**
   (maintainer decision 2026-05-31): it's one short burst once at power-on, not during play.

### Build flavors: release (UART, clean) vs debug (USB, verbose)

| | Release (default) | Debug overlay |
|---|---|---|
| Config | `sdkconfig.defaults` only | `sdkconfig.defaults` + `sdkconfig.debug` |
| IDF/bootloader logs | OFF (`*_LOG_LEVEL_NONE`) | INFO |
| IDF console route | UART0 | **USB-C** (`CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG`) |
| Arduino `Serial` (`PLAY`/`STOP`) | UART0 | UART0 (unchanged) |
| Built by | CI on every tag | local dev only |

Debug build command:
`idf.py -D SDKCONFIG_DEFAULTS="sdkconfig.defaults;sdkconfig.debug" build flash monitor`
(later defaults file wins on conflicting choices). This keeps the UART0 line free for the
robot while a developer watches logs over USB-C. `Serial`'s `PLAY`/`STOP` stays on UART0 in
both flavors; mirroring it to USB too would require Arduino USB-CDC build flags
(`ARDUINO_USB_CDC_ON_BOOT`/`ARDUINO_USB_MODE`) in CMake — not done.

## Source files reviewed

`RCj_comm_module.ino`, `main/app_main.cpp`, `definitions.h`, `ble.cpp/.h`,
`ble_processing.cpp/.h`, `state_machine.cpp/.h`, `display.cpp/.h`, `functions.cpp/.h`,
git commit `ca23261`.

## Open questions

- Should the loop adopt a dedicated FreeRTOS task / explicit synchronization for state?
- Is the `.ino` still maintained, or should it be retired in favor of the IDF `main/` path?
</content>
