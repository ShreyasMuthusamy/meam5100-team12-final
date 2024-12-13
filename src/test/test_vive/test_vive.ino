#include "vive510.h"

#define VIVE_PIN_1 1
#define VIVE_PIN_2 2

Vive510 vive1(VIVE_PIN_1);
Vive510 vive2(VIVE_PIN_2);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vive.begin();
  Serial.println("Vive started");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (vive1.status() == VIVE_RECEIVING) {
    Serial.printf("Vive 1: X: %d, Y %d\n", vive1.xCoord(), vive1.yCoord());
  } else {
    vive.sync(5);
  }
  if (vive2.status() == VIVE_RECEIVING) {
    Serial.printf("Vive 2: X: %d, Y %d\n", vive2.xCoord(), vive1.yCoord());
  } else {
    vive.sync(5);
  }
  delay(20);
}
