#include <Arduino.h>
#include <Metro.h>
#include <RTClib.h>
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

RTC_DS1307 rtc;

void setup()
{
    /** Serial Communication **/
    Serial.begin(9600);
    xbee.setSerial(Serial);

    /** RTC **/
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1)
            ;
    }
    if (!rtc.isrunning()) {
        Serial.println("RTC is NOT running!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //コンパイルした時の時間をRTCに記録する
    }

    /** PORT **/
    PORTC = 0x00; //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    delay(10);
}

float calc_average(float sum, unsigned int count)
{
    return sum / count * 5 / 1024;
}

void pack_data_in_array(uint8_t* send_data, uint8_t payload)
{
    DateTime now = rtc.now();
    send_data[0] = now.year() % 100;
    send_data[1] = now.month();
    send_data[2] = now.day();
    send_data[3] = now.hour();
    send_data[4] = now.minute();
    send_data[5] = now.second();
    send_data[6] = payload;
}

void send_to_xbee(uint8_t payload)
{
    uint8_t send_data[7] = {};
    pack_data_in_array(send_data, payload);
    ZBTxRequest zbTx = ZBTxRequest(addr64, send_data, sizeof(send_data));
    xbee.send(zbTx);
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
            send_to_xbee(power_on);
        }
    }
}
