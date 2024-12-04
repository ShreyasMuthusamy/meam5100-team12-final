#include "html510.h"
#include "vive510.h"
#include "robot.h"
#include "commands.h"
#include "webpage.h"

// Define the pins corresponding to the motors and encoders
#define MOTOR_L_PWM 5
#define MOTOR_L_DIR 18
#define MOTOR_R_PWM 7
#define MOTOR_R_DIR 10
#define ENC_L_PIN 1
#define ENC_R_PIN 4
#define IR_L_ADD 0x30
#define IR_F_ADD 0x31
#define IR_R_ADD 0x32
#define IR_L_SHUT 0
#define IR_F_SHUT 1
#define IR_R_SHUT 2
#define VIVE_L_PIN 12
#define VIVE_R_PIN 13
#define SERVO_PIN 3

// Define PID values
#define KPL 5
#define KIL 0.5
#define KDL 0.1
#define KPR 5
#define KIR 0.5
#define KDR 0.1

// Set a cap on the maximum speed of the robot (for better robot control)
// Units: encoder counts per frame
#define MAX_SPEED 30

Robot robot(
  MOTOR_L_PWM, MOTOR_L_DIR, ENC_L_PIN,
  MOTOR_R_PWM, MOTOR_R_DIR, ENC_R_PIN,
  IR_L_ADD, IR_F_ADD, IR_R_ADD,
  IR_L_SHUT, IR_F_SHUT, IR_R_SHUT,
  VIVE_L_PIN, VIVE_R_PIN, SERVO_PIN
);
Scheduler scheduler(&robot);

///////////////////////////////////////////
// WiFi Methods
//
// WiFi Login Credentials
const char* ssid = "shreyasESP32";
const char* passwd = "";

// Instantiate the WiFi server
HTML510Server server(80);

// Root handler
void handleRoot() {
  // Send the body HTML to the client
  server.sendhtml(body);
}

// Command handler
void handleCommand() {
  // Get the command from the web server and schedule it
  String command = server.getText();
  scheduler.schedule(command, &robot);
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
void setup() {
  setupWiFi();
  server.begin();

  // Attach handlers
  server.attachHandler("/", handleRoot);
  server.attachHandler("/command=", handleCommand);

  robot.init();
  robot.setPID(KPL, KIL, KDL);
}

void loop() {
  // Fix the frame rate to 50 Hz
  static unsigned long millisLast = millis();
  if (millis() - millisLast > 1000 / FRAME_RATE) {
    millisLast = millis();
    robot.drive(0, 0);
  }
  scheduler.run();
  server.serve();
}
