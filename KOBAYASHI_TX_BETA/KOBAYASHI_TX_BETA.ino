#include <Arduino.h>
#include <Metro.h>
#include <XBee.h>

#define CURRENT_SENSOR_PORT 0 //電流センサが繋がっているポート番号
#define INTERRUPT_TIME 500 //割り込み間隔[ms]
#define THRESHOLD_VOLTAGE 0.05 //しきい値電圧[V]

float current_sum = 0; //電流センサの値の合計
unsigned int read_count = 0; //センサを読み込んだ回数をカウント
boolean power_state = false; //溶接機の電源の状態

Metro average_metro = Metro(INTERRUPT_TIME);

XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4166e492);

void setup()
{
    Serial.begin(9600);
    // xbee.setSerial(Serial);
    PORTC = 0x00; //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    delay(10);
}

float calc_average(float sum, unsigned int count)
{
    return sum / count * 5 / 1024;
}

void loop()
{
    current_sum += analogRead(CURRENT_SENSOR_PORT);
    read_count++;

    if (average_metro.check()) {
        boolean power_on = calc_average(current_sum, read_count) > THRESHOLD_VOLTAGE;
        current_sum = 0;
        read_count = 0;

        if (power_on != power_state) {
            power_state = power_on;
            ZBTxRequest zbTx = ZBTxRequest(addr64, (uint8_t*)&power_on, sizeof(uint8_t));
            xbee.send(zbTx);
        }
    }
}
