#include <Arduino.h>
#include <SoftwareSerial.h>
#include <XBee.h>

/*
#define CURRENT_ID 0x00
#define ENVIRONMENT_ID 0x01
#define DUST_ID 0x02
*/
XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();

SoftwareSerial monitor(8, 9); //(Rx, Tx)

void setup()
{
    // start serial
    Serial.begin(9600);
    xbee.setSerial(Serial);
    monitor.begin(9600);
    monitor.println("Starting up!");
}

uint8_t calc_checksum(uint8_t* send_data, uint8_t frame_length)
{
    uint16_t sum = 0;
    for (int i = 0; i < frame_length; i++) {
        sum += send_data[i + 2];
    }
    return 0xFF - (sum & 0xFF); //チェックサムの計算
}

void serial_write(uint8_t* get_data, uint8_t get_data_length)
{
    uint8_t number_of_sensor = get_data_length - 9; //受信パケットからセンサデータ以外の要素を省く
    uint8_t send_data_length = get_data_length + 4; //受信パケットサイズ + シリアル通信時に使用する符号の数
    uint8_t send_data[send_data_length] = {};

    send_data[0] = 0xFF; //スタートビット
    send_data[1] = get_data_length; //フレーム長(フレーム長とチェックサムに挟まれたバイト数)
    for (int i = 0; i < get_data_length; i++) { //受信パケットを加工せずに追加する
        send_data[i + 2] = get_data[i];
    }
    send_data[send_data_length - 2] = calc_checksum(send_data, get_data_length); //チェックサム: フレーム長 == 受信パケットサイズ
    send_data[send_data_length - 1] = 0x00; //ストップビット
    monitor.write(send_data, send_data_length);

    print_send_data(send_data, send_data_length);
}

void print_send_data(uint8_t* send_data, uint8_t send_data_length)
{
    monitor.println("Display the send data");
    for (int i = 0; i < send_data_length; i++) {
        monitor.print("frame");
        monitor.print("[");
        monitor.print(i, DEC);
        monitor.print("] is ");
        monitor.println(send_data[i], HEX);
    }
}

void loop()
{
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
            xbee.getResponse().getZBRxResponse(rx);
            serial_write(rx.getData(), rx.getDataLength());
        }
    } else if (xbee.getResponse().isError()) {
        monitor.print("error code:");
        monitor.println(xbee.getResponse().getErrorCode());
    }
}
