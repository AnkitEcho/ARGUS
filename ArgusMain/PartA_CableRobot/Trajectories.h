#ifndef TRAJECTORIES_H
#define TRAJECTORIES_H

enum PathType { PATH_HORIZONTAL = 0, PATH_VERTICAL = 1, PATH_DIAGONAL = 2, PATH_INFINITY = 3 };

// HORIZONTAL / VERTICAL / DIAGONAL are straight lines between two points,
// traced back and forth (ping-pong), parameterised by u in [0, 1].
// INFINITY is a figure-8 (lemniscate) centred on (centerX, centerY) with
// half-width ampX and half-height ampY, parameterised by angle u in
// [0, 2*PI), traced continuously (no ping-pong needed -- it's a closed loop).
struct PathParams {
  float xStart, yStart;
  float xEnd,   yEnd;
  float centerX, centerY;
  float ampX, ampY;
};

// Writes the plate (x, y) for path parameter u onto x/y.
void getPointOnPath(PathType type, const PathParams &p, float u, float &x, float &y);

// Upper bound of u for this path type (1.0 for lines, 2*PI for infinity).
float getPathUMax(PathType type);

#endif
