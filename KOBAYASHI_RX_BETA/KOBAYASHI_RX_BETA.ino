#include <Arduino.h>
#include <SoftwareSerial.h>
#include <XBee.h>

#define DATE_TIME_SIZE 6

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
// create reusable response objects for responses we expect to handle
ZBRxResponse rx = ZBRxResponse();
ModemStatusResponse msr = ModemStatusResponse();

SoftwareSerial monitor(8, 9); //(Rx, Tx)

void setup()
{
    // start serial
    Serial.begin(9600);
    xbee.setSerial(Serial);
    monitor.begin(9600);

    monitor.println("Starting up!");
}

void serial_write(uint8_t* get_data, int data_size)
{
    int send_data_size = data_size + 3;
    uint8_t send_data[send_data_size] = {};
    send_data[0] = 0xFF; //スタートビット
    send_data[1] = data_size; //フレーム長
    for (int i = 0; i < data_size; i++) {
        send_data[i + 2] = get_data[i];
    }
    send_data[send_data_size - 1] = 0xFF; //TODO: チェックサム
    //TODO:WPFアプリケーションにシリアル通信でデータを送る
    for (int i = 0; i < send_data_size; i++) {
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
