// =====================================================================
// PartB_Thrower.ino
//
// Receives aim/throw commands over Serial from the vision system
// (vision/track_and_predict.py running on a PC or Raspberry Pi watching
// Part A's plate through a camera) and executes them:
//   - rotate the turret (on its wheels) to face the target azimuth
//   - raise/lower the hydraulic actuator to the target height
//   - spin up the flywheel launcher
//   - release the disc on command
//
// Serial commands (115200 baud, newline terminated):
//   AIM <angleDeg> <heightMm> <flywheelPWM>   -> replies AIMED or AIM_FAILED
//   THROW                                     -> releases the disc, replies THROWN
//   STOP                                      -> immediate stop of all motion
// =====================================================================

#include "Config.h"
#include "MotorControl.h"

void setup() {
  Serial.begin(115200);
  motorControlInit();
  Serial.println(F("Thrower ready. Commands: AIM a h p | THROW | STOP"));
}

void loop() {
  if (estopActive()) {
    stopAllMotion();
    return;
  }
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();

  if (line.startsWith("AIM")) {
    float angle, height;
    int pwm;
    if (sscanf(line.c_str(), "AIM %f %f %d", &angle, &height, &pwm) == 3) {
      bool ok1 = rotateToAngle(angle, 3000);
      bool ok2 = setHeight(height, 4000);
      spinFlywheels((uint8_t)constrain(pwm, 0, 255));
      delay(FLYWHEEL_SPINUP_MS);
      Serial.println((ok1 && ok2) ? F("AIMED") : F("AIM_FAILED"));
    } else {
      Serial.println(F("ERR bad AIM command"));
    }
  } else if (line == "THROW") {
    releaseDisc();
    delay(400);
    stopFlywheels();
    Serial.println(F("THROWN"));
  } else if (line == "STOP") {
    stopAllMotion();
    Serial.println(F("STOPPED"));
  }
}
