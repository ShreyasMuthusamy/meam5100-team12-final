#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <arduino.h>

#define BEZIER_DISTANCE 10
#define NUM_POINTS 200
#define LA_THRESH 5

#define PI 3.141592

// Define the Pose struct
struct Pose {
  float x;
  float y;
  float theta;
};

class Trajectory {
  private:
    Pose p0, p3;
    float Ax, Ay, Bx, By, Cx, Cy, Dx, Dy;
    float lookahead;
    Pose getNextPathPose(Pose robotPose);

  public:
    Trajectory(Pose _p0, Pose _p3, float la): p0(_p0), p3(_p3), lookahead(la) { calculate(); }
    void calculate();
    float getCurvature(Pose robotPose);
    bool finished(Pose robotPose);
};

#endif GEOMETRY_H_
