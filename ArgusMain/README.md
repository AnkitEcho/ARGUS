# Plate-Chasing Disc Thrower — Full Project

Two independent robots:

- **Part A** (`PartA_CableRobot/`) — a plate suspended by 4 fishing-line
  cables from motors at the corners of an acrylic board, traced through
  horizontal / vertical / diagonal / infinity-shaped paths at constant speed,
  with the exact coordinates of each path set by you.
- **Part B** (`PartB_ThrowerRobot/`) — a turret that rotates on its wheels,
  raises/lowers a hydraulic actuator, and fires a disc from a flywheel
  launcher, aimed by a camera that watches Part A's plate and predicts where
  it will be.

They're intentionally decoupled: Part B never receives data from Part A's
Arduino directly — it *observes* the plate optically, matching how you
described it, and the two share only a common (x, y) coordinate frame (see
`shared/SerialProtocol.md`).

```
RobotProject/
├── PartA_CableRobot/          Arduino sketch for the cable plate robot
│   ├── PartA_CableRobot.ino   main sketch (mode switch + serial commands)
│   ├── Config.h               pins, board geometry, motor type
│   ├── Kinematics.h/.cpp      (x,y) -> 4 cable lengths
│   ├── Trajectories.h/.cpp    horizontal/vertical/diagonal/infinity paths
│   ├── MotorDriver.h/.cpp     continuous-servo OR stepper backend
│   └── README.md              wiring, calibration, safety
│
├── PartB_ThrowerRobot/        Arduino sketch + vision system for the thrower
│   ├── PartB_Thrower.ino      main sketch (Serial AIM/THROW/STOP commands)
│   ├── Config.h                pins, calibration constants
│   ├── MotorControl.h/.cpp     azimuth/height/launcher closed-loop control
│   ├── vision/
│   │   ├── track_and_predict.py   camera tracking + prediction + serial
│   │   ├── tools/hsv_picker.py    marker-colour calibration helper
│   │   └── requirements.txt
│   └── README.md               wiring, calibration, safety
│
└── shared/
    └── SerialProtocol.md      command reference for both Arduinos
```

## How it fits together
1. **Part A's Arduino** runs standalone once flashed: a mode switch picks
   which pattern is active, and you set the exact coordinates for each
   pattern once over Serial (they're remembered until you change them again
   or lose power).
2. **A camera** (on a Pi, laptop, or any USB-webcam PC) watches the board and
   runs `vision/track_and_predict.py`, which:
   - detects the coloured marker on Part A's plate,
   - converts its pixel position to real-world (x, y) mm via a one-time
     homography calibration,
   - predicts a near-future position from recent velocity,
   - computes the azimuth/height/power Part B needs, and
   - sends `AIM ...` then `THROW` to **Part B's Arduino** over USB serial.
3. **Part B's Arduino** executes those commands with closed-loop control
   (potentiometer feedback on both azimuth and height) and reports back
   success/failure.

## Getting started, in order
1. Build Part A, flash `PartA_CableRobot.ino`, and get it tracing shapes
   reliably on its own (see `PartA_CableRobot/README.md`).
2. Build Part B, flash `PartB_Thrower.ino`, and bench-test `AIM`/`THROW`
   commands manually over the Serial Monitor before connecting the vision
   system (see `PartB_ThrowerRobot/README.md`).
3. Set up the camera, calibrate marker colour and board corners, measure
   `THROWER_ORIGIN`, and run `track_and_predict.py`.
4. Tune the height/power-vs-distance curve with real test throws.

## Safety
This project combines spinning motors, taut cables under tension, a
hydraulic actuator, and a device that launches a projectile. Please:
- Keep an accessible E-stop on Part B at all times.
- Test each subsystem individually, at low power, before combining them.
- Keep people (especially faces) clear of the disc's flight path and the
  cable robot's moving parts.
- Add physical/mechanical limit stops in addition to the software limits
  in this code — software should be a second line of defense, not the only one.
