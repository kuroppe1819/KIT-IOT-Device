#include <Arduino.h>
#include <Metro.h>

#define INTERVAL 1000

const uint8_t currentArrayLength = 14;
const uint8_t environmentArrayLength = 16;
const uint8_t dustArrayLength = 14;

Metro interrupt = Metro(INTERVAL);

uint8_t currentArrayData[currentArrayLength] = {
    0xFF, //スタートビット
    0x0A, //フレーム長
    0x00, //子機ID
    0x00, //エリアコードの上位8bit
    0x00, //エリアコードの下位8bit
    0x12, //年
    0x0A, //月
    0x0B, //日
    0x0E, //時
    0x13, //分¨
    0x24, //秒
    0x01, //電流
    0x00, //チェックサム
    0x00, //ストップビット
};

uint8_t environmentArrayData[environmentArrayLength] = {
    0xFF, //スタートビット
    0x0C, //フレーム長
    0x01, //子機ID
    0x00, //エリアコードの上位8bit
    0x01, //エリアコードの下位8bit
    0x12, //年
    0x0A, //月
    0x0B, //日
    0x0E, //時
    0x13, //分
    0x24, //秒
    0x18, //温度
    0x41, //湿度
    0xEA, //照度
    0x00, //チェックサム
    0x00, //ストップビット
};

uint8_t dustArrayData[dustArrayLength] = {
    0xFF, //スタートビット
    0x0A, //フレーム長
    0x02, //子機ID
    0x00, //エリアコードの上位8bit
    0x02, //エリアコードの下位8bit
    0x12, //年
    0x0A, //月
    0x0B, //日
    0x0E, //時
    0x13, //分¨
    0x24, //秒
    0x56, //粉塵
    0x00, //チェックサム
    0x00, //ストップビット
};

uint8_t calcChecksum(uint8_t* arrayData, uint8_t frameLength)
{
    uint16_t sum = 0;
    for (int i = 0; i < frameLength; i++) {
        sum += arrayData[i + 2];
    }
    return 0xFF - (sum & 0xFF); //チェックサムの計算
}

void setup()
{
    Serial.begin(9600);
    currentArrayData[currentArrayLength - 2] = calcChecksum(currentArrayData, currentArrayData[1]);
    environmentArrayData[environmentArrayLength - 2] = calcChecksum(environmentArrayData, environmentArrayData[1]);
    dustArrayData[dustArrayLength - 2] = calcChecksum(dustArrayData, dustArrayData[1]);
}

void loop()
{
    if (interrupt.check()) {
        Serial.write(currentArrayData, currentArrayLength);
        Serial.write(environmentArrayData, environmentArrayLength);
        Serial.write(dustArrayData, dustArrayLength);
    }
}