#include "html510.h"
#include "vive510.h"
#include "robot.h"
#include "commands.h"
#include "webpage.h"

// Define the pins corresponding to the motors and encoders
#define MOTOR_L_PWM 5
#define MOTOR_L_DIR 7
#define MOTOR_R_PWM 4
#define MOTOR_R_DIR 6
#define ENC_L_A 12
#define ENC_L_B 13
#define ENC_R_A 10
#define ENC_R_B 11
#define IR_L_ADD 0x30
#define IR_F_ADD 0x31
#define IR_R_ADD 0x32
#define IR_L_SHUT 39
#define IR_F_SHUT 38
#define IR_R_SHUT 37
#define VIVE_L_PIN 1
#define VIVE_R_PIN 2

#define SERVO_PIN 8

// Define PID values
#define KL 6, 0.2, 0.1
#define KR 6, 0.3, 0.1

Robot robot(
  MOTOR_L_PWM, MOTOR_L_DIR, ENC_L_A, ENC_L_B,
  MOTOR_R_PWM, MOTOR_R_DIR, ENC_R_A, ENC_R_B,
  IR_L_ADD, IR_F_ADD, IR_R_ADD,
  IR_L_SHUT, IR_F_SHUT, IR_R_SHUT,
  VIVE_L_PIN, VIVE_R_PIN, SERVO_PIN
);

CommandHandler handler(&robot);

Vive510 leftVive(VIVE_L_PIN);
Vive510 rightVive(VIVE_R_PIN);

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
  handler.setCommand(command);
  server.sendplain("");
}

// Command handler
void handleControl() {
  if (handler.getCommand() == "teleop") {
    // Get the command from the web server and schedule it
    String control = server.getText();
    handler.setControl(control);
  }
  server.sendplain("");
}

// Setup the software-enabled AP WiFi server
void setupWiFi() {
  WiFi.softAP(ssid, passwd);
  Serial.print("AP IP address: HTML//");
  Serial.println(WiFi.softAPIP());
  server.begin();
}
//
///////////////////////////////////////////
void setup() {
  Serial.begin(115200);

  setupWiFi();
  server.begin();

  // Attach handlers
  server.attachHandler("/", handleRoot);
  server.attachHandler("/command=", handleCommand);
  server.attachHandler("/control=", handleControl);

  robot.init();
  robot.setPID(KL, KR);

  String initialCommand = "followWall";
  handler.setCommand(initialCommand);
}

void loop() {
  // Fix the frame rate to 50 Hz
  static unsigned long millisLast = millis();
  if (millis() - millisLast > 1000 / FRAME_RATE) {
    millisLast = millis();
    // robot.update();
    // handler.run();
    // Serial.printf("Distance (LFR): %d, %d, %d\n", robot.getLeftDistance(), robot.getFrontDistance(), robot.getRightDistance());

    static int leftX, leftY, rightX, rightY;
    Pose currPose;

    if (leftVive.status() == VIVE_RECEIVING) {
      leftX += (leftVive.xCoord() - leftX) / 4;
      leftY += (leftVive.yCoord() - leftY) / 4;
      Serial.println("Left seen");
    } else {
      leftVive.sync(5);
    }

    if (rightVive.status() == VIVE_RECEIVING) {
      rightX += (rightVive.xCoord() - rightX) / 4;
      rightY += (rightVive.yCoord() - rightY) / 4;
      Serial.println("Right seen");
    } else {
      rightVive.sync(5);
    }

    currPose.x = (leftX + rightX) / 2.0;
    currPose.y = (leftY + rightY) / 2.0;
    currPose.theta = atan2(rightY - leftY, rightX - leftX);

    // Serial.printf("Left Coords: (%d, %d), Right Coords: (%d, %d), Pose: (%.1f, %.1f, %.1f)\n", leftX, leftY, rightX, rightY, currPose.x, currPose.y, currPose.theta);
  }
  // server.serve();
}
