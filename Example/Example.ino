#include <Arduino.h>
#include <MsTimer2.h>
#include "SchmittTrigger.h"

#define CURRENT_SENSOR 1                //電流センサの数
#define INTERRUPT_TIME 100              //割り込み間隔[ms]
#define SEND_TIME 5000                  //xbeeの送信間隔[ms]                 
#define THRESHOLD_VOLTAGE 3.0           //しきい値電圧[V]

float current_sum[CURRENT_SENSOR] = {}; //電流センサの値の合計
unsigned int read_count = 0;            //センサを読み込んだ回数をカウント

typedef volatile struct {
    boolean enable = false;             //割り込み許可フラグ
    unsigned int count = 0;             //割り込み回数
} Interrupt;

Interrupt interrupt;

void interrupt_switch() {
    interrupt.enable = true;
    interrupt.count++;
}

void clear_variable() {
    for (int i =0; i < CURRENT_SENSOR; i++) {
        current_sum[i] = 0;
    }
    read_count = 0;
}

void setup() {
    Serial.begin(9600);
    PORTC = 0x00;   //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    MsTimer2::set(INTERRUPT_TIME, interrupt_switch);
    MsTimer2::start();
}

float calc_average(float sum, unsigned int count) {
    return sum / count * 5 / 1024;
}

void loop() {
    boolean power_on[CURRENT_SENSOR] = {}; //溶接機の電源

    for (int i = 0; i < CURRENT_SENSOR; i++) {
        current_sum[i] += analogRead(i);
    }
    read_count++;

    if (interrupt.enable) {
        for (int i = 0; i < CURRENT_SENSOR; i++) {
            power_on[i] = calc_average(current_sum[i], read_count) > THRESHOLD_VOLTAGE;
        }
        clear_variable();
        interrupt.enable = false;
    }

    if (interrupt.count * INTERRUPT_TIME >= SEND_TIME) {
        //親機へ送信するための処理
        interrupt.count = 0;
    }
}
