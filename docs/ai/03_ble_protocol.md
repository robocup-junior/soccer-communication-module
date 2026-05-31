# 03 — BLE Protocol

The module is a BLE **peripheral** (GATT server) using the **Nordic UART Service (NUS)**
UUID scheme. NimBLE stack (`CONFIG_BT_NIMBLE_ENABLED=y`).

## Device name / advertising

- Advertised name: `BLE_NAME = "RCJs-m_" + BLE_MAC_to_string()`
  (`definitions.h` + `functions.cpp`), e.g. `RCJs-m_AA:BB:CC:DD:EE:FF`.
  - **Why the MAC is in the name (per `AI_HANDOFF.md`):** iPhones do not expose the BLE
    MAC to apps, so the app identifies/selects modules by the advertised **name** instead.
- The MAC is the **Bluetooth** MAC: `esp_read_mac(base_mac, ESP_MAC_BT)`.
- The same MAC string is encoded into the **QR code** on the "wait for connection" screen.
- Advertising restarts automatically on disconnect (`MyServerCallbacks::onDisconnect`).

## UUIDs (`ble.cpp`)

| Role | UUID | Properties |
|------|------|------------|
| Service | `6E400001-B5A3-F393-E0A9-E50E24DCCA9E` | NUS |
| RX (app → module, write) | `6E400002-B5A3-F393-E0A9-E50E24DCCA9E` | `WRITE` + `WRITE_NR` (write-without-response) |
| TX (module → app, notify) | `6E400003-B5A3-F393-E0A9-E50E24DCCA9E` | `NOTIFY` |

> Changed in commit `ca23261`: RX gained `WRITE_NR`, TX switched from `INDICATE` (with a
> `BLE2902` descriptor) to plain `NOTIFY`. **Any app expecting indications/CCCD on TX or
> requiring write-with-response on RX must be re-checked against this firmware.**

### Connection parameters (NimBLE only)

Requested on connect via `pServer->requestConnParams(...)` (`MyServerCallbacks::onConnect`
with `ble_gap_conn_desc`), from `definitions.h`:

| Constant | Value | Meaning |
|----------|-------|---------|
| `BLE_CONN_INTERVAL_MIN` | 12 | 15 ms (units of 1.25 ms) |
| `BLE_CONN_INTERVAL_MAX` | 24 | 30 ms |
| `BLE_CONN_LATENCY` | 0 | no slave latency |
| `BLE_CONN_TIMEOUT` | 300 | 3 s (units of 10 ms) — lowered from 2000/20 s in fw v0.97 |

> **`BLE_CONN_TIMEOUT` was lowered from 2000 (20 s) to 300 (3 s) in fw v0.97 (2026-05-31).**
> - History: the 20 s value was originally chosen on purpose (`AI_HANDOFF.md`) because
>   shorter timeouts caused modules to disconnect on missed pings / latency spikes.
> - **Problem found in testing (two effects of the same value):**
>   1. *Reconnect:* on link loss the phone's BLE stack kept the stale connection alive until
>      the supervision timeout expired — up to ~20 s — before reconnecting. Unacceptable in a
>      match.
>   2. *Safety/fairness:* on a **silent** link loss the module's `onDisconnect` only fires at
>      supervision-timeout expiry, so the robot could keep **playing for up to ~20 s** before
>      the module fail-safes to STOP.
> - **Change made:** `BLE_CONN_TIMEOUT = 300` (3 s). Trades a little robustness against brief
>   dropouts for much faster reconnect *and* faster fail-safe stop. The C5 supercap reduces
>   the original need for a long timeout (brief power dips no longer reset the module).
> - **Spec constraint:** supervision timeout (ms) must be `> (1 + latency) * interval_max * 2`.
>   With `latency=0` and `interval_max=30 ms`, the floor is ~60 ms, so 3 s is safely valid.
> - ⚠️ Validate on real hardware that 3 s does not cause spurious disconnects during normal
>   play (the original 20 s was anti-flap); revisit upward if flapping reappears.

## Message framing

Defined by `ble_msg_t` (`ble_processing.h`) and parsed in `MyCallbacks::onWrite`:

```
byte[0]      = msg_id          (one of ble_msg_id)
byte[1..N]   = payload data    (msg-specific; up to BLE_DATA_MAX_LENGTH = 10 bytes)
```

- `ble_msg_t.data_length` = received length − 1 (the id byte is not counted).
- Accepted only if `1 <= length <= BLE_DATA_MAX_LENGTH + 1` (i.e. id + up to 10 data bytes).
  Empty or oversized writes are silently ignored.
- Valid messages are pushed to the queue (overwrite-oldest if full). Dispatch happens later
  in the main loop (`ble_msg_processing`), one message per loop iteration.

## Message IDs (`ble_msg_id` enum, `ble_processing.h`)

Values are the enum ordinals (no explicit numbering), so order matters — **do not reorder**
without updating the app:

| ID | Name | Value | Direction | Handled in RX switch? |
|----|------|-------|-----------|------------------------|
| `BLE_MSG_PING` | ping | 0 | app → module (echo back) | yes |
| `BLE_MSG_FW_VERSION` | firmware version | 1 | app → module (reply) | yes |
| `BLE_MSG_SET_NAME` | set indicator | 2 | app → module | yes |
| `BLE_MSG_SET_SCORE` | set score | 3 | app → module | yes |
| `BLE_MSG_PLAY` | play | 4 | app → module | yes |
| `BLE_MSG_STOP` | stop | 5 | app → module | yes |
| `BLE_MSG_DAMAGE` | penalty/damage | 6 | app → module | yes |
| `BLE_MSG_HALF_BREAK` | halftime | 7 | app → module | yes |
| `BLE_MSG_GAME_OVER` | game over | 8 | app → module | yes |
| `BLE_MSG_DISCONNECT` | disconnect | 9 | **module → app** | no (outbound only) |
| `BLE_MSG_ASK_FOR_PENALTY` | self-penalty request | 10 | **module → app** | no (outbound only) |
| `BLE_MSG_MAX_ID` | sentinel | 11 | — | — |

## Per-message payload and side effects (`ble_msg_processing`)

| Message | Payload (data bytes) | Effect | Affects |
|---------|----------------------|--------|---------|
| `BLE_MSG_PING` | arbitrary, echoed | Replies with the **same** bytes (`data_length + 1`) on TX | link check |
| `BLE_MSG_FW_VERSION` | none | Replies `[id, FW_VERSION_MAJOR, FW_VERSION_MINOR]` (3 bytes) | — |
| `BLE_MSG_SET_NAME` | `data[0]`, `data[1]` = two ASCII chars | `module_set_indicator(c0 + c1)` | display only |
| `BLE_MSG_SET_SCORE` | `data[0]` = my score, `data[1]` = opponent score | sets both scores | display only |
| `BLE_MSG_PLAY` | none | `stm_set_state(STM_PLAY)` | **OUT1/2 → HIGH**, display |
| `BLE_MSG_STOP` | none | `stm_set_state(STM_STOP)` | **OUT1/2 → LOW**, display |
| `BLE_MSG_DAMAGE` | `data[0..3]` = uint32 **big-endian** milliseconds | `stm_set_timer(ms)`, `stm_set_state(STM_DAMAGE)` | OUT1/2 → LOW, countdown |
| `BLE_MSG_HALF_BREAK` | `data[0..3]` = uint32 **big-endian** milliseconds | `stm_set_timer(ms)`, `stm_set_state(STM_HALF_TIME)` | OUT1/2 → LOW, countdown |
| `BLE_MSG_GAME_OVER` | `data[0]` = my score, `data[1]` = opponent score | sets scores, `stm_set_state(STM_GAME_OVER)` | OUT1/2 → LOW, display |
| `BLE_MSG_DISCONNECT` | — (sent by module) | Sent in `ble_disconnect()` before dropping the link | — |
| `BLE_MSG_ASK_FOR_PENALTY` | — (sent by module) | Sent on local double-press, **only while `STM_PLAY`** | — |

### Which messages affect the robot PLAY/STOP outputs

- **HIGH (play):** `BLE_MSG_PLAY`.
- **LOW (stop):** `BLE_MSG_STOP`, `BLE_MSG_DAMAGE`, `BLE_MSG_HALF_BREAK`,
  `BLE_MSG_GAME_OVER`, and a BLE **disconnect** (→ `STM_DISCONNECTED`, which on the next
  state-change cycle leaves `robot_play=false`).

### Which messages affect the display only

`BLE_MSG_SET_NAME`, `BLE_MSG_SET_SCORE` (these do not change state; the new values appear
the next time a screen re-renders). `BLE_MSG_GAME_OVER` also updates the score.

> ⚠️ **Render-timing nuance:** `STM_DISCONNECTED` only redraws when `state_changed` is set,
> and `STM_PLAY`/`STM_STOP` redraw every loop. So a `SET_SCORE`/`SET_NAME` received while
> idle in `STM_DISCONNECTED` will not visibly update until the next state change. See
> [04](04_state_machine.md).

## Timer payload encoding

`BLE_MSG_DAMAGE` / `BLE_MSG_HALF_BREAK` decode the duration as **big-endian** uint32:

```c
stm_set_timer((uint32_t)data[0]<<24 | (uint32_t)data[1]<<16 | (uint32_t)data[2]<<8 | data[3]);
```

Value is in **milliseconds**; the display shows it as `seconds` or `m:ss` (see display doc).

## Error / unknown-message behavior

- Unknown `msg_id`: `default` case does nothing (the error `Serial.println` is commented out).
- Malformed length (0 or > 11 bytes total): dropped at `onWrite`, never queued.
- No negative ACK / error reply is ever sent to the app.

## Protocol risks & compatibility notes

- **Enum ordinals are the wire IDs.** Inserting/removing/reordering any `ble_msg_id` member
  shifts every later ID and breaks the app. If you must extend, **append before
  `BLE_MSG_MAX_ID`** and keep existing order.
- TX changed from INDICATE→NOTIFY and RX gained WRITE_NR (`ca23261`) — verify the shipped
  app matches.
- No authentication/pairing logic in firmware; any BLE client can write commands.
- `BLE_DATA_MAX_LENGTH = 10` caps payloads; current largest payload is 4 bytes (timers).
- `BLE_MSG_PING` echo length uses `data_length + 1`; very large echoes are bounded by the
  10-byte input cap.

## Source files reviewed

`ble.cpp/.h`, `ble_processing.cpp/.h`, `definitions.h`, `functions.cpp`, commit `ca23261`.

## Open questions

- Exact app-side expectation for `BLE_MSG_FW_VERSION` reply format (3 bytes assumed).
- Does the app rely on `BLE_MSG_DISCONNECT` notification arriving before link drop?
- Are there app→module messages the app sends that the firmware silently ignores today?
</content>
