#include "Arduino.h"
//#include "Planter.h"

static const uint32_t MIN_PLANT_TIME_INTERVAL = 600000; // let soil to soak for 10 mins
static const uint32_t PLANT_TIME = 5000; // plant for 30 secs once per 10 mins
//static const uint32_t CHECK_INTERVAL = 300000; // check every 5 mins
static const uint32_t CHECK_INTERVAL = 10000; // check every 5 mins

class Planter {
  private:
    char * id;
    int motorPin;
    int sensorPin;
    int hydrationLevel;
    //int plantTime = 10 * 1000; // planting time 10 secs, then stop
    //int delayBetweenPlanting = 2 * 60 * 1000; // we want water to absorb for 2 mins to read correct measurements
    uint32_t lastPlantingStartTime = 0; // planting time 10 secs, then stop
    int plantingStatus = LOW;
    uint32_t inDelayUntil = 0;
  public:
    Planter(int motorPin, int sensorPin, int hydrationLevel, string id): motorPin(motorPin), sensorPin(sensorPin), hydrationLevel(hydrationLevel), id(id) {

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
          if (currentTime - lastPlantingStartTime > PLANT_TIME) { // already planting more than 10 secs, shut down
            log("Shut down, planting for more than secs:", (currentTime - lastPlantingStartTime) / 1000);
            plantOff();
            delayFor(CHECK_INTERVAL);
          } else {// to make sure we're not planting for ever
            log("Delay for a sec, to wait until planting cycle is over. Cur planting secs: ", (currentTime - lastPlantingStartTime) / 1000);
            // just delay for a sec, to check if it's enough planting yet
            delayFor(1000);
          }
        } else { // new planting cycle
          log("Sensor says need to plant, check if it's not too often. ");
          if (lastPlantingStartTime == 0
              || currentTime - lastPlantingStartTime > MIN_PLANT_TIME_INTERVAL) { // check that we planted more than 2 mins ago, or just started
            // just start
            log("Starting new planting cycle, as last time planted secs ago:", (currentTime - lastPlantingStartTime) / 1000);

            plantOn();
            //delayFor(plantTime / 2); // plant for half period, so we check more often and mb sensor would say it's enough
          } else {
            log("Even though soil is dry, don't start, as just planted secs ago:", (currentTime - lastPlantingStartTime) / 1000);
            plantOff();
            delayFor(CHECK_INTERVAL);
          }
        }
      }
    }

    void delayFor(uint64_t delayFor) {
      log("delay for:", delayFor);
      inDelayUntil = millis() + delayFor;
      log("result:", inDelayUntil);
    }

    void setup() {
      log("CHECK_INTERVAL:", CHECK_INTERVAL);
      log("MIN_PLANT_TIME_INTERVAL:", MIN_PLANT_TIME_INTERVAL);
      log("PLANT_TIME:", PLANT_TIME);
      pinMode(sensorPin, INPUT);
      pinMode(motorPin, OUTPUT);
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
      int ret = LOW;
      int sensorValue = analogRead(sensorPin); //take a sample
      if (sensorValue >= 1000) {
        log(sensorValue, " - Sensor is not in the Soil or DISCONNECTED");
        ret = LOW; // don't do anything as most probably curciut has sensor broken or disconnected. we can overplant
      } else if (sensorValue >= 600) {
        log(sensorValue, " - Soil is DRY");
        ret = HIGH;
      } else if (sensorValue >= 320) {
        log(sensorValue, " - Soil is DRYISH");
        ret = HIGH;
      } else if (sensorValue > hydrationLevel) {
        log(sensorValue, " - Sensor in HUMID/WATER, OK");
        ret = LOW;
      }
      return ret;
    }

    void log(char *txt) {
      Serial.print(id);
      Serial.print(": ");
      Serial.println(txt);
    }

    void log(char *txt, uint64_t num) {
      Serial.print(id);
      Serial.print(": ");
      Serial.print(txt);
      Serial.println(uintToStr(num, str));
    }

    void log(uint64_t num, char *txt) {
      Serial.print(id);
      Serial.print(": ");
      Serial.print(uintToStr(num, str));
      Serial.println(txt);
    }
    char * uintToStr( const uint64_t num, char *str )
    {
      uint8_t i = 0;
      uint64_t n = num;

      do
        i++;
      while ( n /= 10 );

      str[i] = '\0';
      n = num;

      do
        str[--i] = ( n % 10 ) + '0';
      while ( n /= 10 );

      return str;
    }
    char str[21];
    //Serial.println( uintToStr( num, str ) );
};
