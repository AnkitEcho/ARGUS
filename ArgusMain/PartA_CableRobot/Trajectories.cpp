#include "Trajectories.h"
#include <math.h>

void getPointOnPath(PathType type, const PathParams &p, float u, float &x, float &y) {
  switch (type) {
    case PATH_HORIZONTAL:
    case PATH_VERTICAL:
    case PATH_DIAGONAL: {
      x = p.xStart + (p.xEnd - p.xStart) * u;
      y = p.yStart + (p.yEnd - p.yStart) * u;
      break;
    }
    case PATH_INFINITY: {
      // Lemniscate of Gerono: traces a figure-8 through the centre point.
      x = p.centerX + p.ampX * cosf(u);
      y = p.centerY + p.ampY * sinf(u) * cosf(u);
      break;
    }
  }
}

float getPathUMax(PathType type) {
  return (type == PATH_INFINITY) ? (2.0f * (float)PI) : 1.0f;
}
