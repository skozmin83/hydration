#include "Planter.cpp"

Planter planter1(8, A0, 10, "P8-big", 30000, 250, 310);
Planter planter2(9, A1, 11, "P9-med", 25000, 250, 330);
Planter planter3(7, A2, 12, "P7-sml", 25000, 250, 350);

void setup() {
    Serial.begin(9600);
    planter1.setup();
    planter2.setup();
    planter3.setup();
}

void loop() {
  // todo use https://www.gammon.com.au/serial for non-blocking read to get plan config/status
    planter1.loop();
    planter2.loop();
    planter3.loop();
}
