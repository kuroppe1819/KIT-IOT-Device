#include <Arduino.h>
#include <SoftwareSerial.h>
#include <XBee.h>

#define CURRENT_ID 0x00
#define ENVIRONMENT_ID 0x01
#define DUST_ID 0x02
#define USE_NUMBER_OF_AREA 3

const uint8_t send_frame_size = 13;
const uint8_t send_data_size = 16;

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();

SoftwareSerial monitor(8, 9); //(Rx, Tx)

struct AreaData {
    uint8_t current = 0xFE; //0xFEはそのセンサを使用していないことを示す
    uint8_t temperature = 0xFE;
    uint8_t humidity = 0xFE;
    uint8_t illumination = 0xFE;
    uint8_t dust = 0xFE;
};
struct AreaData area_data[USE_NUMBER_OF_AREA];

void setup()
{
    // start serial
    Serial.begin(9600);
    xbee.setSerial(Serial);
    monitor.begin(9600);
    monitor.println("Starting up!");
}

uint8_t calc_checksum(uint8_t* send_data)
{
    uint16_t sum = 0;
    for (int i = 0; i < send_frame_size; i++) {
        sum += send_data[i + 2]; //2~14の範囲の合計
    }
    return 0xFF - (sum & 0xFF); //チェックサムの計算
}

void serial_write(uint8_t* get_data, struct AreaData area_data)
{
    uint8_t send_data[16] = {
        0xFF, //スタートビット
        send_frame_size,
        get_data[1], //エリアコードの上位8bit
        get_data[2], //エリアコードの下位8bit
        get_data[3], //年
        get_data[4], //月
        get_data[5], //日
        get_data[6], //時
        get_data[7], //分
        get_data[8], //秒
        area_data.current,
        area_data.temperature,
        area_data.humidity,
        area_data.illumination,
        area_data.dust,
        0x00 //チェックサム
    };

    send_data[send_data_size - 1] = calc_checksum(send_data);
    monitor.write(send_data, send_data_size);

    // monitor.print("AreaCode=");
    // monitor.println((rx.getData()[1] << 8) + rx.getData()[2]);
    // for (int i = 0; i < send_data_size; i++) {
    //     monitor.print("frame");
    //     monitor.print("[");
    //     monitor.print(i, DEC);
    //     monitor.print("] is ");
    //     monitor.println(send_data[i], HEX);
    // }
}

void loop()
{
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
            xbee.getResponse().getZBRxResponse(rx);
            uint8_t h_digit = rx.getData()[1];
            uint8_t l_digit = rx.getData()[2];
            uint8_t index = (h_digit << 8) + l_digit;

            switch (rx.getData()[0]) {
            case CURRENT_ID:
                area_data[index].current = rx.getData()[9];
                break;
            case ENVIRONMENT_ID:
                area_data[index].temperature = rx.getData()[9];
                area_data[index].humidity = rx.getData()[10];
                area_data[index].illumination = rx.getData()[11];
                break;
            case DUST_ID:
                area_data[index].dust = rx.getData()[9];
                break;
            default:
                break;
            }
            serial_write(rx.getData(), area_data[index]);
        }
    } else if (xbee.getResponse().isError()) {
        monitor.print("error code:");
        monitor.println(xbee.getResponse().getErrorCode());
    }
}
