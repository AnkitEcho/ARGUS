// =====================================================================
// PartA_CableRobot.ino
//
// Drives a 4-cable plate ("fishing wire") plotter. A mode switch selects
// HORIZONTAL / VERTICAL / DIAGONAL / INFINITY path shapes, and the exact
// endpoints/geometry of each shape are set over Serial. The plate is
// driven at an approximately constant speed along whichever path is active.
//
// Serial commands (115200 baud, newline terminated):
//   SET H x1 y1 x2 y2     endpoints (mm) for the HORIZONTAL-switch path
//   SET V x1 y1 x2 y2     endpoints for the VERTICAL-switch path
//   SET D x1 y1 x2 y2     endpoints for the DIAGONAL-switch path
//   SET I cx cy ax ay     centre + half-width/half-height for INFINITY path
//   HOME                  re-zero cable-length tracking (plate must be
//                          physically at the board's origin when sent)
//
// Wiring: see README.md in this folder.
// =====================================================================

#include "Config.h"
#include "Kinematics.h"
#include "Trajectories.h"
#include "MotorDriver.h"

// Defaults -- override any of these over Serial at runtime.
PathParams horizontalPath = { -200,    0,  200,    0, 0, 0,   0,   0 };
PathParams verticalPath   = {    0, -150,    0,  150, 0, 0,   0,   0 };
PathParams diagonalPath   = { -200, -150,  200,  150, 0, 0,   0,   0 };
PathParams infinityPath   = {    0,    0,    0,    0, 0, 0, 200, 120 };

float prevLengths[4];
float u = 0.0f;
int8_t direction = 1;          // for line ping-pong
PathType currentMode = PATH_HORIZONTAL;
unsigned long lastUpdate = 0;

PathType readModeSwitch() {
  int a = digitalRead(MODE_PIN_0); // active-low (INPUT_PULLUP)
  int b = digitalRead(MODE_PIN_1);
  int code = (b << 1) | a;
  switch (code) {
    case 0: return PATH_HORIZONTAL;
    case 1: return PATH_VERTICAL;
    case 2: return PATH_DIAGONAL;
    default: return PATH_INFINITY;
  }
}

PathParams &pathParamsFor(PathType t) {
  switch (t) {
    case PATH_HORIZONTAL: return horizontalPath;
    case PATH_VERTICAL:   return verticalPath;
    case PATH_DIAGONAL:   return diagonalPath;
    default:              return infinityPath;
  }
}

void reHome() {
  inverseKinematics(0, 0, prevLengths);
  u = 0;
  direction = 1;
}

void handleSerialCommands() {
  if (!Serial.available()) return;
  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line == "HOME") {
    reHome();
    Serial.println(F("OK homed"));
    return;
  }

  if (!line.startsWith("SET")) return;
  char code;
  float a, b, c, d;
  int n = sscanf(line.c_str(), "SET %c %f %f %f %f", &code, &a, &b, &c, &d);
  if (n != 5) { Serial.println(F("ERR bad command")); return; }

  switch (code) {
    case 'H': clampToWorkspace(a, b); clampToWorkspace(c, d); horizontalPath = { a, b, c, d, 0, 0, 0, 0 }; break;
    case 'V': clampToWorkspace(a, b); clampToWorkspace(c, d); verticalPath   = { a, b, c, d, 0, 0, 0, 0 }; break;
    case 'D': clampToWorkspace(a, b); clampToWorkspace(c, d); diagonalPath   = { a, b, c, d, 0, 0, 0, 0 }; break;
    case 'I': clampToWorkspace(a, b); infinityPath = { 0, 0, 0, 0, a, b, c, d }; break;
    default:  Serial.println(F("ERR unknown path code")); return;
  }
  Serial.println(F("OK"));
}

void setup() {
  Serial.begin(115200);
  pinMode(MODE_PIN_0, INPUT_PULLUP);
  pinMode(MODE_PIN_1, INPUT_PULLUP);
  pinMode(HOME_BUTTON_PIN, INPUT_PULLUP);

  motorsInit();
  reHome();
  currentMode = readModeSwitch();

  Serial.println(F("Cable Plate Robot ready."));
  Serial.println(F("SET H/V/D x1 y1 x2 y2 | SET I cx cy ax ay | HOME"));
}

void loop() {
  handleSerialCommands();

  if (digitalRead(HOME_BUTTON_PIN) == LOW) {
    reHome();
  }

  unsigned long now = millis();
  if (now - lastUpdate < CONTROL_DT_MS) {
    motorsRun();
    return;
  }
  float dt = (now - lastUpdate) / 1000.0f;
  lastUpdate = now;

  PathType mode = readModeSwitch();
  if (mode != currentMode) {
    currentMode = mode;
    u = 0;
    direction = 1;
  }
  const PathParams &pp = pathParamsFor(currentMode);

  // --- Advance the path parameter so linear plate speed stays ~constant ---
  float x, y, xNext, yNext;
  getPointOnPath(currentMode, pp, u, x, y);
  float probe = (currentMode == PATH_INFINITY) ? 0.01f : 0.001f;
  getPointOnPath(currentMode, pp, u + probe * direction, xNext, yNext);
  float dSdU = sqrtf(sq(xNext - x) + sq(yNext - y)) / probe;
  if (dSdU < 1e-3f) dSdU = 1e-3f;
  float du = (PLATE_SPEED_MM_S * dt) / dSdU;
  u += du * direction;

  float uMax = getPathUMax(currentMode);
  if (currentMode == PATH_INFINITY) {
    if (u >= uMax) u -= uMax;      // continuous loop
  } else {
    if (u >= uMax) { u = uMax; direction = -1; }  // ping-pong
    else if (u <= 0) { u = 0; direction = 1; }
  }

  // --- Convert the new plate position into per-motor cable speeds ---
  getPointOnPath(currentMode, pp, u, x, y);
  float newLengths[4];
  inverseKinematics(x, y, newLengths);

  float speeds[4];
  for (int i = 0; i < 4; i++) {
    speeds[i] = (newLengths[i] - prevLengths[i]) / dt;
    prevLengths[i] = newLengths[i];
  }
  motorsSetCableSpeeds(speeds);
  motorsRun();
}
