#include "Arduino.h"
//#include "Planter.h"

static const uint32_t MIN_PLANT_TIME_INTERVAL = 600000; // let soil to soak for 10 mins
static const uint32_t PLANT_TIME = 10000; // plant for 30 secs once per 10 mins
static const uint32_t CHECK_INTERVAL = 300000; // check every 5 mins
//static const uint32_t CHECK_INTERVAL = 10000; // check every 5 mins

class Planter {
private:
    char *id;
    int cyclesRun = 0;
    int motorPin;
    int sensorPin;
    int sensorPowerPin;
    int hydratedLevel; // at this level planter won't try to plant it anymore
    int deHydratedLevel; // at this level planter will try to plan it
    int plantTime; // planting time 10 secs, then stop
    //int delayBetweenPlanting = 2 * 60 * 1000; // we want water to absorb for 2 mins to read correct measurements
    uint32_t lastPlantingStartTime = 0; // planting time 10 secs, then stop
    int plantingStatus = LOW;
    uint32_t inDelayUntil = 0;
public:
    Planter(int motorPin, int sensorPin, int sensorPowerPin, char *id, int plantTime = PLANT_TIME, int hydratedLevel = 250, int deHydratedLevel = 320) :
            motorPin(motorPin),
            sensorPin(sensorPin),
            sensorPowerPin(sensorPowerPin),
            plantTime(plantTime),
            hydratedLevel(hydratedLevel),
            deHydratedLevel(deHydratedLevel),
            id(id) {
        if (hydratedLevel < deHydratedLevel) { // set default values
            hydratedLevel = 250;
            deHydratedLevel = 320;
        }
    }

    void loop() {
        uint32_t currentTime = millis();
        if (inDelayUntil > currentTime) {
            return;
        }
        int newPlantingStatus = getSensorStatus();

        if (newPlantingStatus == LOW) {
            log("No need to plan, shut down and wait for the next check.");
            plantOff();
            delayFor(CHECK_INTERVAL); // wait for 2 mins before the next check
        } else { // we need to plant according to sensor
            if (plantingStatus == HIGH) { // check if we've been planting already
                log("Already planting, check for how long and if we need to stop. ");
                if (currentTime - lastPlantingStartTime > plantTime) { // already planting more than 10 secs, shut down
                    log("Shut down, planting for more than secs:", (currentTime - lastPlantingStartTime) / 1000);
                    plantOff();
                    delayFor(CHECK_INTERVAL);
                } else {// to make sure we're not planting for ever
                    log("Delay for a sec, to wait until planting cycle is over. Cur planting secs: ",
                        (currentTime - lastPlantingStartTime) / 1000);
                    // just delay for a sec, to check if it's enough planting yet
                    delayFor(1000);
                }
            } else { // new planting cycle
                log("Sensor says need to plant, check if it's not too often. ");
                if (lastPlantingStartTime == 0
                    || currentTime - lastPlantingStartTime >
                       MIN_PLANT_TIME_INTERVAL) { // check that we planted more than 2 mins ago, or just started
                    // just start
                    log("Starting new planting cycle, as last time planted secs ago:",
                        (currentTime - lastPlantingStartTime) / 1000);
                    cyclesRun++;
                    plantOn();
                    //delayFor(plantTime / 2); // plant for half period, so we check more often and mb sensor would say it's enough
                } else {
                    log("Even though soil is dry, don't start, as just planted secs ago:",
                        (currentTime - lastPlantingStartTime) / 1000);
                    plantOff();
                    delayFor(CHECK_INTERVAL);
                }
            }
        }
    }

    void delayFor(uint64_t delayFor) {
        inDelayUntil = millis() + delayFor;
    }

    void setup() {
        log("CHECK_INTERVAL:", CHECK_INTERVAL);
        log("MIN_PLANT_TIME_INTERVAL:", MIN_PLANT_TIME_INTERVAL);
        log("plantTime:", plantTime);
        log("motorPin:", motorPin);
        log("sensorPin:", sensorPin);
        log("sensorPowerPin:", sensorPowerPin);
        log("hydratedLevel:", hydratedLevel);
        log("deHydratedLevel:", deHydratedLevel);
        pinMode(sensorPin, INPUT);
        pinMode(motorPin, OUTPUT);
        pinMode(sensorPowerPin, OUTPUT);
        plantOff();
    }

    void plantOn() {
        digitalWrite(motorPin, HIGH);
        lastPlantingStartTime = millis();
        plantingStatus = HIGH;
        log("Water ON. ");
    }

    void plantOff() {
        digitalWrite(motorPin, LOW);
        plantingStatus = LOW;
        log("Water OFF. ");
    }

    int getSensorStatus() {
        digitalWrite(sensorPowerPin, HIGH);
        delay(10);
        int sensorValue = analogRead(sensorPin); //take a sample
        digitalWrite(sensorPowerPin, LOW);
        if (sensorValue >= 1000) {
            log(sensorValue, " - Sensor is not in the Soil or DISCONNECTED");
            // don't do anything as most probably curciut has sensor broken or disconnected. we can overplant
        } else if (sensorValue >= 600) {
            log(sensorValue, " - Soil is DRY");
        } else if (sensorValue >= deHydratedLevel) {
            log(sensorValue, " - Soil is DRYISH");
        } else if (sensorValue > hydratedLevel) {
            log(sensorValue, " - Sensor in HUMID/WATER, OK");
        } else {
            log(sensorValue, " - Sensor in WATER, OK");
        }
        int ret;
        if (sensorValue < 1000 && sensorValue > deHydratedLevel) {
            ret = HIGH;
        } else {
            ret = LOW;
        }
        log((uint64_t)((sensorValue / (double)deHydratedLevel) *100), "% dry. ");
        return ret;
    }

    void log(char *txt) {
        printStartOfLine();
        Serial.println(txt);
    }

    void log(char *txt, uint64_t num) {
        printStartOfLine();
        Serial.print(txt);
        Serial.println(uintToStr(num, str));
    }

    void log(uint64_t num, char *txt) {
        printStartOfLine();
        Serial.print(uintToStr(num, str));
        Serial.println(txt);
    }

    void printStartOfLine() {
        Serial.print(id);
        Serial.print(" (");
        Serial.print(cyclesRun);
        Serial.print("): ");
    }

    char *uintToStr(const uint64_t num, char *str) {
        uint8_t i = 0;
        uint64_t n = num;

        do
            i++;
        while (n /= 10);

        str[i] = '\0';
        n = num;

        do
            str[--i] = (n % 10) + '0';
        while (n /= 10);

        return str;
    }

    char str[21];
    //Serial.println( uintToStr( num, str ) );
};
