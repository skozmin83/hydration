#include "Planter.cpp"

Planter planter1(8, A0, 320, "P8");
Planter planter2(9, A1, 320, "P9");

void setup() {
  Serial.begin(9600);
  planter1.setup();
  planter2.setup();
}

void loop() {
  planter1.loop();
  planter2.loop();
}
