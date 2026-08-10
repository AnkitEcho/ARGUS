# Part B — Vision-Aimed Disc Thrower

A turret that rotates in place on its wheels, adjusts a hydraulic actuator
for launch height, and fires a disc from a dual-flywheel launcher. It never
talks to Part A directly — a camera watches Part A's plate, predicts where
it's headed, and the Arduino here just executes AIM/THROW commands.

## Bill of materials
- Arduino Uno/Mega
- Motor driver for the rotation drive motor (e.g. BTS7960 or L298N) + the
  wheels/tires that turn the whole turret base on a lazy-Susan-style bearing
- Potentiometer geared to the turret's rotation axis (azimuth feedback)
- 12V linear hydraulic actuator (or electric linear actuator, if you'd
  rather not deal with a hydraulic pump) + relay module or H-bridge to
  drive it up/down
- Linear potentiometer (or 2 limit switches for a simpler open-loop version)
  on the actuator for height feedback
- 2x DC motors + wheels as the flywheel launcher, ESC or motor driver to
  spin them
- Micro servo to feed the disc between the flywheels on command
- Normally-closed E-stop button
- A Raspberry Pi, laptop, or any PC with a USB webcam, connected to the
  Arduino via USB serial, to run `vision/track_and_predict.py`

## Wiring (default pin map — edit `Config.h` if you wire differently)
| Signal | Pin |
|---|---|
| Azimuth motor PWM / DIR | D5 / D4 |
| Azimuth feedback pot | A0 |
| Height actuator UP / DOWN relay | D6 / D7 |
| Height feedback pot | A1 |
| Height top / bottom limit switches | D8 / D9 |
| Flywheel L / R PWM | D10 / D11 |
| Feed servo | D3 |
| E-stop (NC, wired to GND) | D12 |

## Serial protocol (from the vision PC to this Arduino)
```
AIM <angleDeg> <heightMm> <flywheelPWM>   ->  replies AIMED or AIM_FAILED
THROW                                     ->  replies THROWN
STOP                                      ->  replies STOPPED
```
See `../shared/SerialProtocol.md` for the full spec.

## Calibration checklist
1. **Azimuth feedback**: with the turret at a known angle, check
   `readAzimuthDeg()` via the Serial Monitor matches reality; adjust the
   `map()` range or potentiometer gearing if not.
2. **Azimuth direction/gain**: confirm `driveAzimuth()`'s `HIGH`/`LOW`
   matches "increasing angle"; tune `AZ_KP` and `AZ_MIN_PWM_TO_MOVE` so the
   turret settles smoothly without oscillating.
3. **Height feedback and limits**: verify the limit switches trigger just
   before the actuator's true mechanical limits (they're your hard safety
   stop if the potentiometer ever misreads).
4. **Launcher power curve**: `height_and_power_for_target()` in
   `track_and_predict.py` is a first guess mapping distance -> actuator
   height + flywheel speed. Do a series of test throws at known distances,
   record what height/PWM actually lands on target, and replace the formula
   (or a lookup table) with your measured data.

## Running the vision system
```
cd vision
pip install -r requirements.txt
python track_and_predict.py --calibrate         # click the 4 board corners once
python track_and_predict.py --port /dev/ttyUSB0 # then run for real
```
See the docstring at the top of `track_and_predict.py` for the full
calibration sequence (marker colour, board corners, thrower position).

## Safety — please read
- Spinning flywheels and a hydraulic actuator are genuinely dangerous.
  Keep the E-stop within reach at all times while testing.
- Keep people out of the disc's flight path and behind the turret until
  you've confirmed the throw direction/power on a low setting.
- Test the azimuth and height control loops with the flywheels powered
  OFF first, and only enable throwing once aiming is reliable.
- Add mechanical hard-stops on the rotation axis and actuator travel in
  addition to the software limits here — never rely on software alone.
