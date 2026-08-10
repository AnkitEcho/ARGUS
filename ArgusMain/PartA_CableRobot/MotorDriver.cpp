#include "MotorDriver.h"
#include "Config.h"
#include <Arduino.h>

// =====================================================================
// BACKEND 1: continuous-rotation (360-degree) hobby servos as winches
// =====================================================================
#if MOTOR_TYPE == MOTOR_TYPE_CONTINUOUS_SERVO
#include <Servo.h>

static Servo servos[4];

void motorsInit() {
  for (int i = 0; i < 4; i++) servos[i].attach(SERVO_PIN[i]);
  motorsStop();
}

void motorsSetCableSpeeds(const float speedsMmPerS[4]) {
  for (int i = 0; i < 4; i++) {
    float angVel = speedsMmPerS[i] / SPOOL_RADIUS_MM; // rad/s at the drum
    float frac = constrain(angVel / SERVO_MAX_ANGVEL_RAD_S, -1.0f, 1.0f);
    int us = SERVO_STOP_US[i] + (int)(frac * SERVO_MAX_DEFLECTION_US);
    servos[i].writeMicroseconds(us);
  }
}

void motorsRun() { /* nothing to do - servos run continuously between updates */ }

void motorsStop() {
  for (int i = 0; i < 4; i++) servos[i].writeMicroseconds(SERVO_STOP_US[i]);
}

// =====================================================================
// BACKEND 2: NEMA17 steppers via A4988/DRV8825 drivers (AccelStepper)
// =====================================================================
#else
#include <AccelStepper.h>

static AccelStepper steppers[4] = {
  AccelStepper(AccelStepper::DRIVER, STEP_PIN[0], DIR_PIN[0]),
  AccelStepper(AccelStepper::DRIVER, STEP_PIN[1], DIR_PIN[1]),
  AccelStepper(AccelStepper::DRIVER, STEP_PIN[2], DIR_PIN[2]),
  AccelStepper(AccelStepper::DRIVER, STEP_PIN[3], DIR_PIN[3]),
};

void motorsInit() {
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, LOW); // LOW = drivers enabled
  for (int i = 0; i < 4; i++) {
    steppers[i].setMaxSpeed(4000);
    steppers[i].setAcceleration(2000);
  }
}

void motorsSetCableSpeeds(const float speedsMmPerS[4]) {
  const float mmPerStep = (2.0f * PI * SPOOL_RADIUS_MM) / STEPPER_STEPS_PER_REV;
  for (int i = 0; i < 4; i++) {
    steppers[i].setSpeed(speedsMmPerS[i] / mmPerStep);
  }
}

void motorsRun() {
  for (int i = 0; i < 4; i++) steppers[i].runSpeed();
}

void motorsStop() {
  for (int i = 0; i < 4; i++) steppers[i].setSpeed(0);
}

#endif
