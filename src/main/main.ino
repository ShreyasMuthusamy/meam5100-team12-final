#include "html510.h"
#include "vive510.h"
#include "robot.h"
#include "commands.h"
#include "webpage.h"

// Define the pins corresponding to the motors and encoders
#define MOTOR_L_PWM 2
#define MOTOR_L_DIR 1
#define MOTOR_R_PWM 4
#define MOTOR_R_DIR 5
#define ENC_L_A 8
#define ENC_L_B 3
#define ENC_R_A 16
#define ENC_R_B 17
#define IR_L_ADD 0x30
#define IR_F_ADD 0x31
#define IR_R_ADD 0x32
#define IR_L_SHUT 38
#define IR_F_SHUT 40
#define IR_R_SHUT 39
#define VIVE_L_PIN 12
#define VIVE_R_PIN 13
#define SERVO_PIN 21

// Define PID values
#define KL 5, 0.15, 0.1
#define KR 6, 0.3, 0.1

Robot robot(
  MOTOR_L_PWM, MOTOR_L_DIR, ENC_L_A, ENC_L_B,
  MOTOR_R_PWM, MOTOR_R_DIR, ENC_R_A, ENC_R_B,
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

  robot.init();
  robot.setPID(KL, KR);
  // scheduler.schedule("wallFollow");
}

void loop() {
  // Fix the frame rate to 50 Hz
  static unsigned long millisLast = millis();
  if (millis() - millisLast > 1000 / FRAME_RATE) {
    millisLast = millis();
    // robot.update();
    // robot.drive(20, 20);
  }
  // scheduler.run();
  // server.serve();
}
