#include "Kinematics.h"
#include "Config.h"
#include <math.h>
#include <Arduino.h>

void inverseKinematics(float x, float y, float outLengths[4]) {
  for (int i = 0; i < 4; i++) {
    float dx = x - ANCHOR_X[i];
    float dy = y - ANCHOR_Y[i];
    outLengths[i] = sqrtf(dx * dx + dy * dy);
  }
}

void clampToWorkspace(float &x, float &y) {
  float xMin = min(ANCHOR_X[0], ANCHOR_X[3]) + SAFETY_MARGIN_MM;
  float xMax = max(ANCHOR_X[1], ANCHOR_X[2]) - SAFETY_MARGIN_MM;
  float yMin = min(ANCHOR_Y[2], ANCHOR_Y[3]) + SAFETY_MARGIN_MM;
  float yMax = max(ANCHOR_Y[0], ANCHOR_Y[1]) - SAFETY_MARGIN_MM;
  x = constrain(x, xMin, xMax);
  y = constrain(y, yMin, yMax);
}
