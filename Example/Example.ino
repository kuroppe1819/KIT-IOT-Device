#include <Arduino.h>
#include <Metro.h>

#define CURRENT_SENSOR 1 //電流センサの数
#define MAX_CURRENT_SENSOR 6 //電流センサを使用できる最大の数
#define INTERRUPT_TIME 500 //割り込み間隔[ms]
#define SEND_TIME 5000 //xbeeの送信間隔[ms]
#define THRESHOLD_VOLTAGE 0.05 //しきい値電圧[V]

float current_sum[CURRENT_SENSOR] = {}; //電流センサの値の合計
unsigned int read_count = 0; //センサを読み込んだ回数をカウント
Metro average_metro = Metro(INTERRUPT_TIME);
Metro send_metro = Metro(SEND_TIME);

void clear_variable()
{
    for (int i = 0; i < CURRENT_SENSOR; i++) {
        current_sum[i] = 0;
    }
    read_count = 0;
}

void setup()
{
    Serial.begin(9600);
    PORTC = 0x00; //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
}

float calc_average(float sum, unsigned int count)
{
    return sum / count * 5 / 1024;
}

boolean is_power_on(boolean* power_on)
{
    for (int i = 0; i < CURRENT_SENSOR; i++) {
        if (power_on[i] == true) {
            return true;
        }
    }
    return false;
}

void loop()
{
    boolean power_on[MAX_CURRENT_SENSOR] = {}; //溶接機の電源

    for (int i = 0; i < CURRENT_SENSOR; i++) {
        current_sum[i] += analogRead(i);
    }
    read_count++;

    if (average_metro.check()) {
        for (int i = 0; i < CURRENT_SENSOR; i++) {
            power_on[i] = calc_average(current_sum[i], read_count) > THRESHOLD_VOLTAGE;
        }

        if (is_power_on(power_on)) {
            // xbeeの送信処理
        }
        clear_variable();
    }
}
