#include <Arduino.h>
#include <MsTimer2.h>
#include "SchmittTrigger.h"

#define CURRENT_SENSOR 0            //電流センサのpin番号
#define INTERRUPT_TIME 100          //割り込み間隔[ms]
#define SEND_TIME 5000              //xbeeの送信間隔[ms]                 
#define THRESHOLD_VOLTAGE 4.0       //しきい値電圧[V]

float current_sum = 0;              //電流センサの値の合計
unsigned int read_count = 0;        //センサを読み込んだ回数をカウント

typedef volatile struct {
    boolean enable = false;           //割り込み許可フラグ
    unsigned int count = 0;         //割り込み回数
} Interrupt;

Interrupt interrupt;
SchmittTrigger schmittTrigger = SchmittTrigger(THRESHOLD_VOLTAGE); //しきい値電圧を設定

void interrupt_switch() {
    interrupt.enable = true;
    interrupt.count++;
}

void setup() {
    Serial.begin(9600);
    PORTC = 0x00;   //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    MsTimer2::set(INTERRUPT_TIME, interrupt_switch);
    MsTimer2::start();
}

float calc_average() {
    return current_sum / read_count * 5 / 1024;
}

void loop() {
    boolean power_on = false; //溶接機の電源
    current_sum += analogRead(CURRENT_SENSOR);
    read_count++;

    if (interrupt.enable) {
        power_on = schmittTrigger.isHigh(calc_average());
        current_sum = 0;
        read_count = 0;
        interrupt.enable = false;
    }

    if (interrupt.count * INTERRUPT_TIME >= SEND_TIME) {
        //親機へ送信するための処理
        Serial.println(power_on);
        interrupt.count = 0;
    }
}
