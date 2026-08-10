#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

void motorsInit();

// Commands each of the 4 motors to wind cable in/out at the given linear
// speed (mm/s). Positive = paying cable out (lengthening), negative =
// reeling cable in (shortening). Called once per control-loop tick.
void motorsSetCableSpeeds(const float speedsMmPerS[4]);

// Must be called every loop() iteration (not just every control tick) so
// the stepper backend can pulse steps at the right rate. No-op for servos.
void motorsRun();

void motorsStop();

#endif
