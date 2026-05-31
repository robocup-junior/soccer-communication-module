# 08 — Mobile App Interaction

The mobile referee/control app is **not source-controlled** in this repo (only a binary
`mobileAPP/RCj2025_APK-...zip` is present and was not unpacked/analyzed). Everything below is
**inferred** from the firmware's BLE behavior, the public `README.md` ("How to control app"
section), and commit history. Treat the app's exact UI as **unknown / to confirm**.

## What can be inferred about the app

- It is a BLE central that connects to a peripheral advertising as `RCJs-m_<MAC>` and uses
  the **Nordic UART Service** (`6E400001-...`). See [03](03_ble_protocol.md).
- It can **scan the QR code** (BLE MAC) shown on the module's "wait for connection" screen
  to identify/select the right module.
- It sends one-byte-id command frames (`[msg_id][payload...]`) over the RX characteristic.
- It receives notifications on the TX characteristic: `PING` echoes, `FW_VERSION` reply, and
  the module-initiated `ASK_FOR_PENALTY` / `DISCONNECT` messages.
- Made by "Mato Faltus" (README Hall of Fame). A new app is "currently working on" per
  README; `mobileAPP` zip is dated 2025.

## BLE command mapping (app action → firmware)

| App action (from README "How to control app") | Likely BLE message | Firmware effect |
|------------------------------------------------|--------------------|-----------------|
| Double-click robot button → start | `BLE_MSG_PLAY` | `STM_PLAY`, OUT HIGH |
| Double-click robot button → stop | `BLE_MSG_STOP` | `STM_STOP`, OUT LOW |
| (Penalty action) | `BLE_MSG_DAMAGE` (+ uint32 ms) | `STM_DAMAGE` + countdown |
| Halftime | `BLE_MSG_HALF_BREAK` (+ uint32 ms) | `STM_HALF_TIME` + countdown |
| End game | `BLE_MSG_GAME_OVER` (+ scores) | `STM_GAME_OVER` |
| Double-click score → +1 | `BLE_MSG_SET_SCORE` (new my:opp) | updates score display |
| Hold score number → −1 | `BLE_MSG_SET_SCORE` (new my:opp) | updates score display |
| Set team/robot label | `BLE_MSG_SET_NAME` (2 chars) | updates indicator |
| Connection settings (hold robot button) | n/a (app-side) | — |
| "Reset all timers / start+stop all robots" | sequence of PLAY/STOP/timer msgs | per message |

> The README control hints are terse and describe **app-side gestures**, not the wire
> protocol. The mapping above is the best-effort correspondence; confirm against the app.

## Score control

- Firmware stores `my_score` / `opponent_score` (0–255 each, single byte).
- Updated only by `BLE_MSG_SET_SCORE` and `BLE_MSG_GAME_OVER`. The "double-click to +1 /
  hold to −1" logic lives **in the app**; the module just receives the resulting values.

## Penalty / self-penalty flow

Two distinct paths:

1. **Referee-initiated penalty (app → module):** `BLE_MSG_DAMAGE` with a 4-byte big-endian
   millisecond duration → `STM_DAMAGE` countdown, outputs LOW.
2. **Self-penalty request (module → app):** local **double-press** of `BUTTON_GPIO` or
   `BUTTON2_GPIO` while in `STM_PLAY` sends `BLE_MSG_ASK_FOR_PENALTY` to the app
   (`functions.cpp` → `ble_msg_procesing_ask_for_penalty()`). The firmware does **not**
   change its own state on a self-penalty request — it is up to the app/referee to respond
   (presumably by sending `BLE_MSG_DAMAGE`). Added in commit `e423b0d`.

## Connection assumptions

- No pairing/bonding/encryption in firmware; the app connects to an open GATT server.
- Connection params are module-requested (15–30 ms interval, 3 s supervision timeout as of
  fw v0.97; was 20 s through v0.96).
- On disconnect the module re-advertises immediately and the robot is stopped.
- A 5 s hold of `BUTTON_GPIO` forces a disconnect from the module side (notifies the app via
  `BLE_MSG_DISCONNECT` first).

## Unknowns to ask about later

- Exact app→firmware message set the **current** app emits (does it use `PING`/`FW_VERSION`?).
- Units/encoding the app uses for penalty/halftime durations (firmware expects **ms,
  big-endian uint32**).
- How the app reacts to `BLE_MSG_ASK_FOR_PENALTY` (auto-penalty vs referee prompt).
- Whether the app depends on the legacy INDICATE behavior (pre-`ca23261`) or the new NOTIFY.
- App-side handling of reconnection and of the `RCJs-m_<MAC>` name / QR identity.

## Source files reviewed

`README.md`, `ble_processing.cpp/.h`, `functions.cpp`, `ble.cpp`, commit `e423b0d`.
(`mobileAPP/RCj2025_APK-...zip` present but not unpacked.)
</content>
