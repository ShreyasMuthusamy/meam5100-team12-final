#include "geometry.h"

Pose Trajectory::getNextPathPose(Pose robotPose) {
  Pose pathPose;

  float x, y, dx, dy;
  for (int i = 0; i < NUM_POINTS; i++) {
    x = Ax * pow((float) i/NUM_POINTS, 3) + Bx * pow((float) i/NUM_POINTS, 2) + Cx * (float) i/NUM_POINTS + Dx;
    y = Ay * pow((float) i/NUM_POINTS, 3) + By * pow((float) i/NUM_POINTS, 2) + Cy * (float) i/NUM_POINTS + Dy;
    float dist = sqrt(pow(x - robotPose.x, 2) + pow(y - robotPose.y, 2));
    if (lookahead - LA_THRESH < dist && dist < lookahead + LA_THRESH) { break; }
  }

  pathPose.x = x;
  pathPose.y = y;
  pathPose.theta = atan2(y - robotPose.y, x - robotPose.x);
  return pathPose;
}

void Trajectory::calculate() {
  // Calculate first control point
  Pose p1;
  p1.x = p0.x + BEZIER_DISTANCE * cos(p0.theta);
  p1.y = p0.y + BEZIER_DISTANCE * sin(p0.theta);

  // Calculate second control point
  Pose p2;
  p2.x = p3.x - BEZIER_DISTANCE * cos(p3.theta);
  p2.y = p3.y - BEZIER_DISTANCE * sin(p3.theta);

  // Calculate the coefficients of the Bezier cubic spline
  Ax = -p0.x + 3*p1.x - 3*p2.x + p3.x; Ay = -p0.y + 3*p1.y - 3*p2.y + p3.y;
  Bx = 3*p0.x - 6*p1.x + 3*p2.x; By = 3*p0.y - 6*p1.y + 3*p2.y;
  Cx = -3*p0.x + 3*p1.x; Cy = -3*p0.y + 3*p1.y;
  Dx = p0.x; Dy = p0.y;
}

float Trajectory::getCurvature(Pose robotPose) {
  Pose goalPose = getNextPathPose(robotPose);
  float L = sqrt(pow(goalPose.x - robotPose.x, 2) + pow(goalPose.y - robotPose.y, 2));
  float relTheta = goalPose.theta - robotPose.theta;
  return 2 * sin(relTheta) / L;
}

bool Trajectory::finished(Pose robotPose) {
  float dist = sqrt(pow(p3.x - robotPose.x, 2) + pow(p3.y - robotPose.y, 2));
  return dist < 5;
}
