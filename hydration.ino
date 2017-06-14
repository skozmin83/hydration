#include "Planter.cpp"

Planter planter1(8, A0, "P8", 250, 320);
Planter planter2(9, A1, "P9", 250, 320);

void setup() {
    Serial.begin(9600);
    planter1.setup();
    planter2.setup();
}

void loop() {
    planter1.loop();
    planter2.loop();
}
