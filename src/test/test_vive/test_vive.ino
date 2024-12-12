#include "vive510.h"

#define VIVE_PIN 39

Vive510 vive(VIVE_PIN);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  vive.begin();
  Serial.println("Vive started");
}

void loop() {
  // put your main code here, to run repeatedly:
  if (vive.status() == VIVE_RECEIVING) {
    Serial.printf("X: %d, Y %d\n", vive.xCoord(), vive.yCoord());
  } else {
    vive.sync(5);
  }
  delay(100);
}
