#include "html510.h"
#include "vive510.h"
#include "robot.h"
#include "commands.h"
#include "webpage.h"

// Define the pins corresponding to the motors, encoders, and sensors
#define MOTOR_L_PWM 5
#define MOTOR_L_DIR 7
#define MOTOR_R_PWM 4
#define MOTOR_R_DIR 6
#define ENC_L_A 12
#define ENC_L_B 13
#define ENC_R_A 10
#define ENC_R_B 11
#define IR_L_SHUT 39
#define IR_F_SHUT 38
#define IR_R_SHUT 37
#define VIVE_L_PIN 1
#define VIVE_R_PIN 2
#define SERVO_PIN 8

// Define the I2C addresses
#define TOPHAT_ADDR 0x28
#define IR_L_ADDR 0x30
#define IR_F_ADDR 0x31
#define IR_R_ADDR 0x32

// Define PID values
#define KL 6, 0.2, 0.1
#define KR 6, 0.3, 0.1

// SDA and SCL pins for I2C
#define SDA_PIN 40
#define SCL_PIN 41

Robot robot(
  MOTOR_L_PWM, MOTOR_L_DIR, ENC_L_A, ENC_L_B,
  MOTOR_R_PWM, MOTOR_R_DIR, ENC_R_A, ENC_R_B,
  IR_L_ADDR, IR_F_ADDR, IR_R_ADDR,
  IR_L_SHUT, IR_F_SHUT, IR_R_SHUT,
  VIVE_L_PIN, VIVE_R_PIN, SERVO_PIN
);

CommandHandler handler(&robot);

Vive510 leftVive(VIVE_L_PIN);
Vive510 rightVive(VIVE_R_PIN);

///////////////////////////////////////////
// Top Hat Methods
int packet_count = 0;
int health = 100;

void send_I2C_byte(uint8_t data) {
  // Send data to slave
  Wire.beginTransmission(TOPHAT_ADDR);
  Wire.write(data);  // Send some test data
  uint8_t error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.printf("Error sending data: %d\n", error);
  }
}

uint8_t receive_I2C_byte() {  // data should have space declared from caller
  // Request data from slave
  uint8_t bytesReceived = Wire.requestFrom(TOPHAT_ADDR, 1);
  uint8_t byteIn = 0;

  if (bytesReceived > 0) {
    Serial.print("Received from slave: ");
    while (Wire.available()) {
      byteIn = Wire.read();
      Serial.printf("0x%02X ", byteIn);
    }
    Serial.println();
  } else return 0;

  return byteIn;  // return number of bytes read
}
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
  packet_count++;
}

// Command handler
void handleControl() {
  if (handler.getCommand() == "teleop") {
    // Get the command from the web server and schedule it
    String control = server.getText();
    handler.setControl(control);
  }
  server.sendplain("");
  packet_count++;
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

  Wire.begin(SDA_PIN, SCL_PIN, 40000);

  // Attach handlers
  server.attachHandler("/", handleRoot);
  server.attachHandler("/command=", handleCommand);
  server.attachHandler("/control=", handleControl);

  robot.init();
  robot.setPID(KL, KR);
}

void loop() {
  // Fix the frame rate to 50 Hz
  static unsigned long millisLast = millis();
  if (millis() - millisLast > 1000 / FRAME_RATE) {
    millisLast = millis();
    if (health > 0) {
      robot.update();
      handler.run();
    } else {
      String commandToSet = "stop";
      handler.setCommand(commandToSet);
      robot.fullSend(0, 0);
    }
    // Serial.printf("Distance (LFR): %d, %d, %d\n", robot.getLeftDistance(), robot.getFrontDistance(), robot.getRightDistance());
  }

  static unsigned long millisLastTH = millis();
  if (millis() - millisLastTH > 500) {
    millisLastTH = millis();
    send_I2C_byte(packet_count);
    packet_count = 0;
    health = receive_I2C_byte();
    Serial.println(health);
  }
  server.serve();
}
