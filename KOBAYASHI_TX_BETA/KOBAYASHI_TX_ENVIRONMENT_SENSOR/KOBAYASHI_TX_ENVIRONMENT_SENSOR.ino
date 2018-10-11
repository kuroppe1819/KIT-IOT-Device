#include "SHT31DIS.h"
#include "TSL2561.h"
#include <Arduino.h>
#include <Metro.h>
#include <RTClib.h>
#include <SD.h>
#include <XBee.h>

#define SUB_MACHINE_ID 0x01 //子機を識別するためのID
#define INTERRUPT_TIME 60000 //割り込み間隔[ms]
#define SS_PIN 4 //SDカードのハードウェアPIN番号
#define FRAME_SIZE 12 //XBeeに送信するフレームのサイズ

const int CODE_LIST_FILE = "codelist.txt";
uint16_t area_code;

Metro send_time_metro = Metro(INTERRUPT_TIME);

XBee xbee = XBee();
XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4166e492);

RTC_DS1307 rtc;
SHT31DIS sht = SHT31DIS(SHT31DIS_ADDR_FLOAT, false, LOOP_ACCURATELY_LEVEL_HIGH);
TSL2561 tsl(TSL2561_ADDR_FLOAT);

void setup()
{
    /** Serial Communication **/
    Serial.begin(9600);
    xbee.setSerial(Serial);

    /** PORT **/
    pinMode(SS_PIN, OUTPUT);
    delay(10);

    /** TSL2561 **/
    tsl.begin();
    tsl.setGain(TSL2561_GAIN_16X);
    tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);

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

    // エリアコードをSDカードから読み取る
    String read_file = "";
    File sd = SD.open(CODE_LIST_FILE);
    while (sd.available()) {
        char str = (char)sd.read();
        if (str == ':') {
            area_code = read_file.toInt();
            break;
        } else {
            read_file.concat(str);
        }
    }
    sd.close();
}

void pack_data_in_array(uint8_t* send_data, uint8_t* payload, uint8_t payload_size)
{
    send_data[0] = SUB_MACHINE_ID;
    send_data[1] = (uint8_t)((area_code >> 8) & 0xFF); //エリアコードの上位8bit
    send_data[2] = (uint8_t)(area_code & 0xFF); //エリアコードの下位8bit
    DateTime now = rtc.now();
    send_data[3] = now.year() % 100;
    send_data[4] = now.month();
    send_data[5] = now.day();
    send_data[6] = now.hour();
    send_data[7] = now.minute();
    send_data[8] = now.second();

    for (int i = 0; i < payload_size; i++) {
        send_data[i + 9] = payload[i];
    }
}

uint8_t get_lux()
{
    uint32_t lum = tsl.getFullLuminosity();
    uint16_t ir, full;
    ir = lum >> 16;
    full = lum & 0xFFFF;
    return (uint8_t)tsl.calculateLux(full, ir);
}

void send_to_xbee(uint8_t* payload, uint8_t payload_size)
{
    uint8_t send_data[FRAME_SIZE] = {};
    pack_data_in_array(send_data, payload, payload_size);
    ZBTxRequest zbTx = ZBTxRequest(addr64, send_data, sizeof(send_data));
    xbee.send(zbTx);
}

void loop()
{
    if (send_time_metro.check()) {
        uint8_t payload[3] = {};
        uint8_t payload_size = sizeof(payload) / sizeof(uint8_t);
        payload[0] = (uint8_t)sht.getTemperature();
        payload[1] = (uint8_t)sht.getHumidity();
        payload[2] = get_lux();
        send_to_xbee(payload, payload_size);
    }
}
