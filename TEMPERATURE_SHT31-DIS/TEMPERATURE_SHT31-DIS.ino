#include "SHT31DIS.h"
#include <Arduino.h>
#include <Wire.h>

SHT31DIS sht = SHT31DIS(SHT31DIS_ADDR_LOW, false, LOOP_ACCURATELY_LEVEL_HIGH);

void setup()
{
    Serial.begin(9600);
    Serial.println("Start!");
}

void loop()
{
    sht.begin();
    Serial.print("temp=");
    Serial.println(sht.getTemperature());
    Serial.print("humidity=");
    Serial.println(sht.getHumidity());
    delay(1000);
}
