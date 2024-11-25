#include "html510.h"
#include "vive510.h"
#include "webpage.h"

#define PI 3.141592

// Define the pins corresponding to the motors and encoders
#define MOTOR_L_PWM 5
#define MOTOR_L_DIR 18
#define MOTOR_R_PWM 7
#define MOTOR_R_DIR 10
#define ENC_L_PIN 1
#define ENC_R_PIN 4

// Motor LEDC values and PID frame rate
#define MOTOR_RES_BITS 12
#define MOTOR_RES ((1 << MOTOR_RES_BITS) - 1)
#define MOTOR_FREQ 100
#define FRAME_RATE 50

// Define PID values
#define KPL 5
#define KIL 0.5
#define KDL 0.1
#define KPR 5
#define KIR 0.5
#define KDR 0.1

// Set a cap on the maximum integral error
#define MAX_ERR_I 60

// Set a cap on the maximum speed of the robot (for better robot control)
// Units: encoder counts per frame
#define MAX_SPEED 30

#define BEZIER_DISTANCE 10

// Define structs
struct Pose {
  float x;
  float y;
  float theta;
};

struct Trajectory {
  float Ax; float Ay;
  float Bx; float By;
  float Cx; float Cy;
  float Dx; float Dy;
};

///////////////////////////////////////////
// States, setpoints, and pose estimation
//
long leftEncoderCounts, rightEncoderCounts;
byte leftFwd, rightFwd;
int vLeft, vRight, vLeftG = 0, vRightG = 0;

// Setup the motors
void setupMotors() {
  ledcAttach(MOTOR_L_PWM, MOTOR_FREQ, MOTOR_RES_BITS);
  ledcAttach(MOTOR_R_PWM, MOTOR_FREQ, MOTOR_RES_BITS);
  pinMode(MOTOR_L_DIR, OUTPUT);
  pinMode(MOTOR_R_DIR, OUTPUT);
}

// Interrupts to count the number of times the photointerrupter was interrupted
void IRAM_ATTR handleLeftEncoderInterrupt() {
  if (leftFwd) {
    leftEncoderCounts++;
  } else {
    leftEncoderCounts--;
  }
}

void IRAM_ATTR handleRightEncoderInterrupt() {
  if (rightFwd) {
    rightEncoderCounts++;
  } else {
    rightEncoderCounts--;
  }
}

// Setup the encoders with INPUT_PULLUP and to use the interrupts defined above
void setupEncoders() {
  pinMode(ENC_L_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_L_PIN), handleLeftEncoderInterrupt, RISING);
  pinMode(ENC_R_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_R_PIN), handleRightEncoderInterrupt, RISING);
}

// Update the "pose" information of the robot (not actual pose, but the values on which PID is done)
void updatePose() {
  static long prevLeftEncoderCounts = leftEncoderCounts;
  static long prevRightEncoderCounts = rightEncoderCounts;

  vLeft = leftEncoderCounts - prevLeftEncoderCounts;
  vRight = rightEncoderCounts - prevRightEncoderCounts;

  prevLeftEncoderCounts = leftEncoderCounts;
  prevRightEncoderCounts = rightEncoderCounts;
}

// Drive the left and right motors
void driveMotors(int left, int right) {
  leftFwd = left > 0;       // Set the variable that will control the positive or negative increment of the encoders
  digitalWrite(MOTOR_L_DIR, leftFwd);   // The inverters set the necessary voltages for the H-bridge, so only one pin is needed
  ledcWrite(MOTOR_L_PWM, map(abs(left), 0, 100, 0, MOTOR_RES));   // Map the duty cycle percentage to the motor resolution

  rightFwd = right > 0;
  digitalWrite(MOTOR_R_DIR, rightFwd);
  ledcWrite(MOTOR_R_PWM, map(abs(right), 0, 100, 0, MOTOR_RES));
}
//
///////////////////////////////////////////
// WiFi Methods
//
// WiFi Login Credentials
const char* ssid = "shreyasESP32";
const char* passwd = "";

// Instantiate the WiFi server
HTML510Server server(80);

// Joystick coordinates
int joyCoords[2];

// Root handler
void handleRoot() {
  // Send the body HTML to the client
  server.sendhtml(body);
}

// Command handler
void handleCommand() {
  // Get the command from the web server and schedule it
  String command = server.getText();
  // scheduler.schedule(command);
  server.sendplain("");
}

// Setup the software-enabled AP WiFi server
void setupWiFi() {
  Serial.begin(115200);
  WiFi.softAP(ssid, passwd);
  Serial.print("AP IP address: HTML//");
  Serial.println(WiFi.softAPIP());
  server.begin();
}
//
///////////////////////////////////////////
// Trajectory Generation and Following Methods
//
Trajectory generateTrajectory(Pose p0, Pose p3) {
  Trajectory res;

  // Calculate first control point
  Pose p1;
  p1.x = p0.x + BEZIER_DISTANCE * cos(p0.theta);
  p1.y = p0.y + BEZIER_DISTANCE * sin(p0.theta);

  // Calculate second control point
  Pose p2;
  p2.x = p3.x - BEZIER_DISTANCE * cos(p3.theta);
  p2.y = p3.y - BEZIER_DISTANCE * sin(p3.theta);

  // Calculate the coefficients of the Bezier cubic spline
  res.Ax = -p0.x + 3*p1.x - 3*p2.x + p3.x; res.Ay = -p0.y + 3*p1.y - 3*p2.y + p3.y;
  res.Bx = 3*p0.x - 6*p1.x + 3*p2.x; res.By = 3*p0.y - 6*p1.y + 3*p2.y;
  res.Cx = -3*p0.x + 3*p1.x; res.Cy = -3*p0.y + 3*p1.y;
  res.Dx = p0.x; res.Dy = p0.y;

  return res;
}
///////////////////////////////////////////
// Feedback and Motor Control Methods
//
int solvePIDleft(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;
  
  // Integral error
  static float errIL = 0.0;
  errIL += (float) err / FRAME_RATE;
  errIL = constrain(errIL, -MAX_ERR_I/KIL, MAX_ERR_I/KIL);
  
  // Derivative error
  static int oldValL;
  int errD = (current - oldValL) * FRAME_RATE;

  // Calculate PID control action
  int u = KPL * err + KIL * errIL - KDL * errD;

  oldValL = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
}

int solvePIDright(int current, int setpoint) {
  // Proportional error
  int err = setpoint - current;

  // Integral error
  static float errIR = 0.0;
  errIR += (float) err / FRAME_RATE;
  errIR = constrain(errIR, -MAX_ERR_I/KIR, MAX_ERR_I/KIR);
  
  // Derivative error
  static int oldValR;
  int errD = (current - oldValR) * FRAME_RATE;
  
  // Calculate PID control action
  int u = KPR * err + KIR * errIR - KDR * errD;

  oldValR = current;
  return constrain(u, -100, 100); // Maximum of 100% (or -100%) duty cycle
}
//
///////////////////////////////////////////
void setup() {
  setupWiFi();
  server.begin();

  // Attach handlers
  server.attachHandler("/", handleRoot);
  server.attachHandler("/command=", handleCommand);

  // Setup pins for output and input
  setupMotors();
  setupEncoders();
}

void loop() {
  // Fix the frame rate to 50 Hz
  static unsigned long millisLast = millis();
  if (millis() - millisLast > 1000 / FRAME_RATE) {
    millisLast = millis();
    // Update pose every frame
    updatePose();
    // Drive the motors using the new pose information
    driveMotors(solvePIDleft(vLeft, vLeftG), solvePIDright(vRight, vRightG));
  }
  server.serve();
}
