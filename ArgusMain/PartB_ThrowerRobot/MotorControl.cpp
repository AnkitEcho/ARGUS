#include "MotorControl.h"
#include "Config.h"
#include <Servo.h>

static Servo feedServo;

void motorControlInit() {
  pinMode(AZ_MOTOR_PWM_PIN, OUTPUT);
  pinMode(AZ_MOTOR_DIR_PIN, OUTPUT);
  pinMode(HEIGHT_RELAY_UP_PIN, OUTPUT);
  pinMode(HEIGHT_RELAY_DOWN_PIN, OUTPUT);
  pinMode(LIMIT_SWITCH_TOP_PIN, INPUT_PULLUP);
  pinMode(LIMIT_SWITCH_BOTTOM_PIN, INPUT_PULLUP);
  pinMode(ESTOP_PIN, INPUT_PULLUP);
  pinMode(FLYWHEEL_L_PWM_PIN, OUTPUT);
  pinMode(FLYWHEEL_R_PWM_PIN, OUTPUT);

  feedServo.attach(FEED_SERVO_PIN);
  feedServo.write(FEED_SERVO_REST_DEG);

  stopAllMotion();
}

bool estopActive() {
  return digitalRead(ESTOP_PIN) == LOW;
}

float readAzimuthDeg() {
  int raw = analogRead(AZ_FEEDBACK_POT_PIN);
  return (float)map(raw, 0, 1023, (long)AZ_ANGLE_MIN_DEG, (long)AZ_ANGLE_MAX_DEG);
}

float readHeightMm() {
  int raw = analogRead(HEIGHT_FEEDBACK_PIN);
  return HEIGHT_MIN_MM + (raw / 1023.0f) * (HEIGHT_MAX_MM - HEIGHT_MIN_MM);
}

static void driveAzimuth(int pwm) { // signed: + = one direction, - = the other
  digitalWrite(AZ_MOTOR_DIR_PIN, pwm >= 0 ? HIGH : LOW);
  analogWrite(AZ_MOTOR_PWM_PIN, constrain(abs(pwm), 0, 255));
}

bool rotateToAngle(float targetDeg, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (estopActive()) { driveAzimuth(0); return false; }

    float error = targetDeg - readAzimuthDeg();
    while (error > 180)  error -= 360;   // shortest-path wrap
    while (error < -180) error += 360;

    if (fabs(error) <= AZ_TOLERANCE_DEG) { driveAzimuth(0); return true; }

    int pwm = constrain((int)(error * AZ_KP), -255, 255);
    if (abs(pwm) < AZ_MIN_PWM_TO_MOVE) pwm = (pwm > 0) ? AZ_MIN_PWM_TO_MOVE : -AZ_MIN_PWM_TO_MOVE;
    driveAzimuth(pwm);
  }
  driveAzimuth(0);
  return false; // timed out
}

bool setHeight(float targetMm, unsigned long timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (estopActive()) {
      digitalWrite(HEIGHT_RELAY_UP_PIN, LOW);
      digitalWrite(HEIGHT_RELAY_DOWN_PIN, LOW);
      return false;
    }

    float error = targetMm - readHeightMm();
    if (fabs(error) <= HEIGHT_TOLERANCE_MM) {
      digitalWrite(HEIGHT_RELAY_UP_PIN, LOW);
      digitalWrite(HEIGHT_RELAY_DOWN_PIN, LOW);
      return true;
    }

    bool goUp = error > 0;
    if (goUp   && digitalRead(LIMIT_SWITCH_TOP_PIN)    == LOW) break; // hit hard limit
    if (!goUp  && digitalRead(LIMIT_SWITCH_BOTTOM_PIN) == LOW) break;

    digitalWrite(HEIGHT_RELAY_UP_PIN,   goUp ? HIGH : LOW);
    digitalWrite(HEIGHT_RELAY_DOWN_PIN, goUp ? LOW  : HIGH);
  }
  digitalWrite(HEIGHT_RELAY_UP_PIN, LOW);
  digitalWrite(HEIGHT_RELAY_DOWN_PIN, LOW);
  return false;
}

void spinFlywheels(uint8_t pwm) {
  analogWrite(FLYWHEEL_L_PWM_PIN, pwm);
  analogWrite(FLYWHEEL_R_PWM_PIN, pwm);
}

void stopFlywheels() {
  analogWrite(FLYWHEEL_L_PWM_PIN, 0);
  analogWrite(FLYWHEEL_R_PWM_PIN, 0);
}

void releaseDisc() {
  feedServo.write(FEED_SERVO_PUSH_DEG);
  delay(300);
  feedServo.write(FEED_SERVO_REST_DEG);
}

void stopAllMotion() {
  driveAzimuth(0);
  digitalWrite(HEIGHT_RELAY_UP_PIN, LOW);
  digitalWrite(HEIGHT_RELAY_DOWN_PIN, LOW);
  stopFlywheels();
}
