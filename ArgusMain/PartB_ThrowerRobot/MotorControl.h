#ifndef MOTORCONTROL_H
#define MOTORCONTROL_H
#include <Arduino.h>

void motorControlInit();

float readAzimuthDeg();
float readHeightMm();

// Closed-loop moves. Block (with an internal timeout) until within
// tolerance or the timeout expires. Return true on success.
bool rotateToAngle(float targetDeg, unsigned long timeoutMs);
bool setHeight(float targetMm, unsigned long timeoutMs);

void spinFlywheels(uint8_t pwm);
void stopFlywheels();
void releaseDisc();

void stopAllMotion();
bool estopActive();

#endif
