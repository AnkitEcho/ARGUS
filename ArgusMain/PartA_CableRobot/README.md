# Part A — Cable-Driven Plate Robot

A plate hangs from 4 fishing-line cables anchored at the corners of an acrylic
board. Coordinating how fast each motor winds/unwinds its cable moves the
plate anywhere in the board's plane. This is the same principle as a
polargraph/vertical-plotter, extended from 2 to 4 anchors for better in-plane
stability.

## Bill of materials
- Arduino Uno/Nano/Mega
- 4x continuous-rotation ("360°") servos, e.g. FS90R (or 4x NEMA17 + A4988/DRV8825
  drivers — see `Config.h`, recommended if you can get the parts)
- 4x small drum/spool (glue or 3D-print onto each servo horn), ~15-25mm diameter
- Fishing line (braided, low-stretch is best), 4 runs from each spool to the plate
- Acrylic board, lightweight plate/ring for the cable attachment point
- 4x small pulleys or smooth-edge eyelets at each corner if the spool isn't
  directly above the cable's exit point
- 2-position x2 mode switch (or a 4-position rotary switch) for pattern select
- Pushbutton for homing
- 5V/6V supply sized for 4 servos (a separate BEC/UBEC from the Arduino's own
  supply is strongly recommended — servos under load can brown out an Uno)

## Wiring (default pin map — edit in `Config.h` if you wire differently)
| Signal | Pin |
|---|---|
| Servo 0 (Top-Left) | D3 |
| Servo 1 (Top-Right) | D5 |
| Servo 2 (Bottom-Right) | D6 |
| Servo 3 (Bottom-Left) | D9 |
| Mode switch bit 0 | A0 (to GND when active, INPUT_PULLUP) |
| Mode switch bit 1 | A1 |
| Home button | D11 (to GND when pressed, INPUT_PULLUP) |

If you switch to steppers, see the `STEP_PIN`/`DIR_PIN`/`ENABLE_PIN` block in
`Config.h` instead.

## Physical setup — important
1. **Measure your board and enter the 4 anchor coordinates** in `Config.h`
   (`ANCHOR_X`/`ANCHOR_Y`), with (0,0) as wherever you want the plate's home
   position to be (usually board centre).
2. **Measure the actual spool radius** (the wound cable's radius, not the
   servo horn) and set `SPOOL_RADIUS_MM`. This directly scales every speed
   calculation — get it as accurate as you can.
3. A cable can only pull, never push — the plate's target must always stay
   inside the rectangle formed by the 4 anchors. `clampToWorkspace()` in
   `Kinematics.cpp` enforces a margin (`SAFETY_MARGIN_MM`) automatically
   whenever you set new path coordinates over Serial.
4. Pre-tension all 4 lines by hand before powering on so no cable starts slack.

## Calibrating continuous-rotation servos
These have no built-in feedback, so two things need tuning per servo:
1. **Stop point** — with the servo commanded at `SERVO_STOP_US[i]` (1500 by
   default), it should sit still. If it creeps, nudge that value up/down by a
   few microseconds until it's stationary.
2. **Max speed** — command full deflection
   (`SERVO_STOP_US[i] ± SERVO_MAX_DEFLECTION_US`), time how long one full
   drum revolution takes, and compute `2*PI / time_seconds` for
   `SERVO_MAX_ANGVEL_RAD_S`.

Because there's no feedback, small errors accumulate the longer it runs.
Press the home button (or send `HOME` over Serial) with the plate physically
at the board's origin whenever you notice drift. **If you can, switch to
NEMA17 steppers** (`MOTOR_TYPE_STEPPER` in `Config.h`) — step counting is
inherently drift-free and will make the traced shapes noticeably cleaner.

## Choosing your X/Y coordinates
Open the Arduino Serial Monitor at 115200 baud and send, e.g.:
```
SET H -250 0 250 0        # horizontal path from (-250,0) to (250,0)
SET V 0 -180 0 180        # vertical path
SET D -250 -180 250 180   # diagonal path
SET I 0 0 220 130         # infinity: centred at origin, 220mm x 130mm
HOME                      # re-zero (plate must be at board origin)
```
The mode switch selects which of these four is currently active; the
coordinates persist until you send a new `SET` for that mode.

## Safety
- Keep hands/hair clear of spinning spools.
- Fishing line under tension can snap and whip — wear eye protection while tuning.
