#ifndef CONFIG_H
#define CONFIG_H

// =====================================================================
// MOTOR TYPE SELECTION
// =====================================================================
// MOTOR_TYPE_CONTINUOUS_SERVO : 360-degree "continuous rotation" hobby
//    servos (e.g. FS90R, or a standard SG90 modified for continuous spin)
//    used as winches. Matches what you described, but is OPEN-LOOP:
//    there is no position feedback, so small errors accumulate over time.
//    Re-home the plate to the origin regularly (see HOME_BUTTON_PIN).
//
// MOTOR_TYPE_STEPPER : NEMA17 steppers + A4988/DRV8825 drivers.
//    Strongly recommended if you can swap parts -- step counting gives
//    you accurate, drift-free cable-length tracking, which this kind of
//    path-tracing robot really benefits from.
//
// Everything else in the codebase (Kinematics, Trajectories, the .ino)
// is identical either way -- only MotorDriver.cpp branches on this.
#define MOTOR_TYPE_CONTINUOUS_SERVO 1
#define MOTOR_TYPE_STEPPER          2
#define MOTOR_TYPE MOTOR_TYPE_CONTINUOUS_SERVO   // <-- change to MOTOR_TYPE_STEPPER if you switch hardware

// =====================================================================
// BOARD / ANCHOR GEOMETRY (millimetres)
// =====================================================================
// (0,0) is the plate's home/origin position -- centre of the board.
// Anchors are the 4 servo/pulley mounting points, in board-plane coords.
// Order: 0 = Top-Left, 1 = Top-Right, 2 = Bottom-Right, 3 = Bottom-Left
// MEASURE YOUR OWN BOARD and edit these four numbers.
const float ANCHOR_X[4] = { -400.0f,  400.0f,  400.0f, -400.0f };
const float ANCHOR_Y[4] = {  300.0f,  300.0f, -300.0f, -300.0f };

// Keep every path safely inside the anchor rectangle so all 4 cables stay
// taut (a cable robot can only pull, never push). This margin (mm) is
// enforced when you set new path coordinates over serial.
const float SAFETY_MARGIN_MM = 60.0f;

// Spool / drum radius in mm -- measure the actual winding drum, not the
// servo horn, since fishing line builds up a coil there.
const float SPOOL_RADIUS_MM = 10.0f;

// Steps per revolution for stepper motors (1.8deg motor = 200 full steps,
// times whatever microstepping your driver is set to).
const int STEPPER_STEPS_PER_REV = 200 * 16;

// =====================================================================
// PIN DEFINITIONS
// =====================================================================
#if MOTOR_TYPE == MOTOR_TYPE_CONTINUOUS_SERVO
  const uint8_t SERVO_PIN[4] = { 3, 5, 6, 9 };
  // Microsecond pulse that makes each servo STOP. Nominally 1500 but every
  // continuous servo needs individual calibration -- adjust until the
  // motor is silent and stationary at "speed 0".
  const int SERVO_STOP_US[4] = { 1500, 1500, 1500, 1500 };
  // Microsecond deflection from stop that gives (approximately) max speed.
  const int SERVO_MAX_DEFLECTION_US = 400;
  // Angular velocity (rad/s) you measured at max deflection -- calibrate
  // by timing how long a full unwrapped revolution takes at full speed.
  const float SERVO_MAX_ANGVEL_RAD_S = 6.0f;
#else
  const uint8_t STEP_PIN[4] = { 2, 4, 7, 12 };
  const uint8_t DIR_PIN[4]  = { 3, 5, 8, 13 };
  const uint8_t ENABLE_PIN  = 10; // shared driver-enable pin, LOW = enabled
#endif

// 2-bit mode-select switch (rotary switch or two DIP pins) -> 4 modes
const uint8_t MODE_PIN_0 = A0;
const uint8_t MODE_PIN_1 = A1;

// Push button: press while the plate is physically centred at the board's
// origin to re-zero the cable-length tracking (clears accumulated drift).
const uint8_t HOME_BUTTON_PIN = 11;

// =====================================================================
// MOTION PARAMETERS
// =====================================================================
const float PLATE_SPEED_MM_S = 60.0f;    // target constant traversal speed
const unsigned long CONTROL_DT_MS = 20;  // control loop period (50 Hz)

#endif
