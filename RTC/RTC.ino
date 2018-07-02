#include <Arduino.h>
#include <RTClib.h>

RTC_DS1307 rtc;

void setup()
{
    Serial.begin(9600);
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
    }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //コンパイルした時の時間をRTCに記録する
}

void loop()
{
    DateTime now = rtc.now();
    uint8_t year = now.year() % 100; // 2018 -> 18に変換
    Serial.print(year);
    Serial.print('/');
    Serial.print(now.month());
    Serial.print('/');
    Serial.print(now.day());
    Serial.print(' ');
    Serial.print(now.hour());
    Serial.print(':');
    Serial.print(now.minute());
    Serial.print(':');
    Serial.print(now.second());
    Serial.println();
    delay(1000);
}
