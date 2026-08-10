#ifndef CONFIG_H
#define CONFIG_H

// =====================================================================
// AZIMUTH (turret rotates on its wheels/tires to face the target)
// =====================================================================
const uint8_t AZ_MOTOR_PWM_PIN = 5;   // H-bridge PWM (speed)
const uint8_t AZ_MOTOR_DIR_PIN = 4;   // H-bridge direction
const uint8_t AZ_FEEDBACK_POT_PIN = A0; // potentiometer geared to the turret
const float AZ_ANGLE_MIN_DEG = 0.0f;
const float AZ_ANGLE_MAX_DEG = 360.0f;
const float AZ_KP = 3.0f;             // proportional gain, deg-error -> PWM
const float AZ_TOLERANCE_DEG = 1.0f;
const int   AZ_MIN_PWM_TO_MOVE = 60;  // overcomes static friction/stiction

// =====================================================================
// HEIGHT (hydraulic linear actuator, controlled via a relay/H-bridge)
// =====================================================================
const uint8_t HEIGHT_RELAY_UP_PIN   = 6;
const uint8_t HEIGHT_RELAY_DOWN_PIN = 7;
const uint8_t HEIGHT_FEEDBACK_PIN   = A1;  // linear potentiometer on the actuator
const float HEIGHT_MIN_MM = 0.0f;
const float HEIGHT_MAX_MM = 300.0f;
const float HEIGHT_TOLERANCE_MM = 3.0f;
const uint8_t LIMIT_SWITCH_TOP_PIN    = 8; // NC, wired to GND when triggered
const uint8_t LIMIT_SWITCH_BOTTOM_PIN = 9;

// =====================================================================
// LAUNCHER (dual flywheel + feed servo)
// =====================================================================
const uint8_t FLYWHEEL_L_PWM_PIN = 10;
const uint8_t FLYWHEEL_R_PWM_PIN = 11;
const uint8_t FEED_SERVO_PIN     = 3;
const int FEED_SERVO_REST_DEG = 0;
const int FEED_SERVO_PUSH_DEG = 90;
const unsigned long FLYWHEEL_SPINUP_MS = 800; // let flywheels reach speed before firing

// =====================================================================
// SAFETY
// =====================================================================
const uint8_t ESTOP_PIN = 12; // normally-closed e-stop button, LOW = pressed/stop

#endif
