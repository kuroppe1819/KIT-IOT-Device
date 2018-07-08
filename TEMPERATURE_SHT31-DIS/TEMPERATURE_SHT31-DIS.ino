#include <Arduino.h>
#include <Wire.h>

const char sht31_address = 0x44;
byte buf[6];

void setup()
{
    Wire.begin();
    Serial.begin(9600);
    Serial.println("Start!");
}

void loop()
{
    Wire.beginTransmission(sht31_address);
    Wire.write(0x24);
    Wire.write(0x00);
    Wire.endTransmission();
    Wire.requestFrom(sht31_address, 6);
    buf[0] = Wire.read(); //MSB for temperature
    buf[1] = Wire.read(); //LSB for temperature
    buf[2] = Wire.read(); //CRC
    buf[3] = Wire.read(); //MSB for humidity
    buf[4] = Wire.read(); //LSB for humidity
    buf[5] = Wire.read(); //CRC

    int bind_temp = buf[0] << 8 | buf[1];
    float temp = -45.0 + 175.0 * bind_temp / 65535.0;
    float bind_humidity = buf[3] << 8 | buf[4];
    float humidity = 100.0 * bind_humidity / 65535.0;

    Serial.print("temp=");
    Serial.println(temp);
    Serial.print("humidity=");
    Serial.println(humidity);
    delay(1000);
}
