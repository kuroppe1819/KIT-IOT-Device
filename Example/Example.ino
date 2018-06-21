#include <Arduino.h>
#include <MsTimer2.h>

#define CURRENT_SENSOR 0
#define SCHMITT_TRIGGER 4.0

float current_sum = 0;                      //電流センサの値の合計
float threshold_voltage = SCHMITT_TRIGGER;  //しきい値電圧
unsigned int read_count = 0;                //センサを読み込んだ回数をカウント
volatile boolean interrupt = false;         //割り込み許可フラグ
boolean power_on = false;                   //溶接機の電源

void interrupt_switch() {
    interrupt = true;
}

void setup() {
    Serial.begin(9600);
    PORTC = 0x00;   //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    MsTimer2::set(100, interrupt_switch);
    MsTimer2::start();
}

float calc_average() {
    return current_sum / read_count * 5 / 1024;
}

boolean jadge_of_power(float current_value) {
    if (current_value > threshold_voltage) {
        threshold_voltage = 5 - SCHMITT_TRIGGER;
    } else {
        threshold_voltage = SCHMITT_TRIGGER;
    }
    return current_value > threshold_voltage;
}

void loop() {
    current_sum += analogRead(CURRENT_SENSOR);
    read_count++;

    if (interrupt == true) {
        power_on = jadge_of_power(calc_average());
        Serial.println(power_on);
        current_sum = 0;
        read_count = 0;
        interrupt = false;
    }

    if (power_on == true) {
        //溶接機の電源がONのときに実行する処理
    }
}
