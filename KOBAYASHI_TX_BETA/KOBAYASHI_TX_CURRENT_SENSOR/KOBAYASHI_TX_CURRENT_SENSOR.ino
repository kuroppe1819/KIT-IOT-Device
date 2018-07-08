#include <Arduino.h>
#include <Metro.h>
#include <RTClib.h>
#include <SD.h>
#include <XBee.h>

#define SUB_MACHINE_ID 0x01 //子機を識別するためのID
#define CURRENT_SENSOR_PIN 0 //電流センサが繋がっているPIN番号
#define INTERRUPT_TIME 500 //割り込み間隔[ms]
#define THRESHOLD_VOLTAGE 0.05 //しきい値電圧[V]
#define SS_PIN 4 //SDカードのハードウェアPIN番号

const int CODE_LIST_FILE = "codelist.txt";
uint8_t id_list[4] = {};

float current_sum = 0; //電流センサの値の合計
unsigned int read_count = 0; //センサを読み込んだ回数をカウント
boolean power_state = false; //溶接機の電源の状態

Metro average_metro = Metro(INTERRUPT_TIME);

XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4166e492);
RTC_DS1307 rtc;
File code_list;

void setup()
{
    /** Serial Communication **/
    Serial.begin(9600);
    xbee.setSerial(Serial);

    /** PORT **/
    pinMode(SS_PIN, OUTPUT);
    PORTC = 0x00; //A0~A5を入力pinに設定する
    ADCSRA &= 0xFC; //分周比を16に設定する
    delay(10);

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

    /* SD */
    if (!SD.begin()) {
        Serial.println("Initialization of SD failed!");
        return;
    }
    if (!SD.exists(CODE_LIST_FILE)) {
        Serial.println("Couldn't find a text file");
        return;
    }
    // エリアコードと機械コードをSDカードから読み取る
    String code = "";
    uint8_t _index = 0;
    code_list = SD.open(CODE_LIST_FILE);
    while (code_list.available()) {
        code.concat((char)code_list.read());
        if (code.charAt(code.length() - 1) == ':') {
            id_list[_index] = code.toInt();
            Serial.println(id_list[_index]);
            code = "";
            _index++;
        }
    }
    code_list.close();
}

float calc_average(float sum, unsigned int count)
{
    return sum / count * 5 / 1024;
}

void pack_data_in_array(uint8_t* send_data, uint8_t payload)
{
    send_data[0] = SUB_MACHINE_ID;
    for (int i = 0; i < sizeof(id_list) / sizeof(uint8_t); i++) {
        send_data[i + 1] = id_list[i];
    }
    DateTime now = rtc.now();
    send_data[5] = now.year() % 100;
    send_data[6] = now.month();
    send_data[7] = now.day();
    send_data[8] = now.hour();
    send_data[9] = now.minute();
    send_data[10] = now.second();
    send_data[11] = payload;
}

void send_to_xbee(uint8_t payload)
{
    uint8_t send_data[12] = {};
    pack_data_in_array(send_data, payload);
    ZBTxRequest zbTx = ZBTxRequest(addr64, send_data, sizeof(send_data));
    xbee.send(zbTx);
}

void loop()
{
    current_sum += analogRead(CURRENT_SENSOR_PIN);
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