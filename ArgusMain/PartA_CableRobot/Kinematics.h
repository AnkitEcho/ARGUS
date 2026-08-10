#ifndef KINEMATICS_H
#define KINEMATICS_H

// Computes the 4 cable lengths (mm) needed to place the plate at world
// position (x, y), given the anchor layout in Config.h.
void inverseKinematics(float x, float y, float outLengths[4]);

// Clamps (x, y) to stay within the anchor rectangle minus SAFETY_MARGIN_MM,
// so cables never need to "push" (go slack) to reach the target.
void clampToWorkspace(float &x, float &y);

#endif
