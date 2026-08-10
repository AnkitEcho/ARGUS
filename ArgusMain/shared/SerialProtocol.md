# Serial Protocols

Part A and Part B do **not** talk to each other — Part B watches Part A
through a camera instead ("observing the movement of the plate"). Each part
only talks to whatever computer is driving it over USB serial.

## Part A — Cable Robot (host PC ⇄ Arduino, 115200 baud)

| Command | Meaning |
|---|---|
| `SET H x1 y1 x2 y2` | Set the HORIZONTAL-switch path's two endpoints (mm) |
| `SET V x1 y1 x2 y2` | Set the VERTICAL-switch path's two endpoints |
| `SET D x1 y1 x2 y2` | Set the DIAGONAL-switch path's two endpoints |
| `SET I cx cy ax ay` | Set the INFINITY path's centre + half-width/height |
| `HOME` | Re-zero cable-length tracking (plate must be at the board's origin) |

Replies: `OK`, `OK homed`, or `ERR ...` on a malformed command.

## Part B — Thrower (vision PC ⇄ Arduino, 115200 baud)

| Command | Meaning |
|---|---|
| `AIM <angleDeg> <heightMm> <flywheelPWM>` | Rotate + raise/lower + spin up |
| `THROW` | Release the disc |
| `STOP` | Immediate stop of all motion |

Replies: `AIMED` / `AIM_FAILED`, `THROWN`, `STOPPED`.

## Coordinate frame

Both parts should agree on one shared (x, y) frame in millimetres, covering
the plane of Part A's board:
- `PartA_CableRobot/Config.h` → `ANCHOR_X` / `ANCHOR_Y` defines it.
- `PartB_ThrowerRobot/vision/track_and_predict.py` → `WORLD_POINTS` and
  `THROWER_ORIGIN` must be measured in that same frame.

Getting this consistent is what lets Part B's camera-based world coordinates
line up with where Part A's plate actually is.
