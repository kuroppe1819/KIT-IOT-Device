#include "SHT31DIS.h"
#include <Arduino.h>
#include <Wire.h>

SHT31DIS::SHT31DIS(int8_t addr, boolean clock_stretch, int8_t loop_accurately_level)
{
    _addr = addr;
    _clock_stretch = clock_stretch;
    _loop_accurately_level = loop_accurately_level;
    Wire.begin();
}

void SHT31DIS::begin(void)
{
    Wire.beginTransmission(_addr);
    if (_clock_stretch) {
        Wire.write(CLOCK_STRETCH_SETTING_ENABLE); //MSB
        switch (_loop_accurately_level) { //LSB
        case LOOP_ACCURATELY_LEVEL_HIGH:
            Wire.write(0x06);
        case LOOP_ACCURATELY_LEVEL_COMMON:
            Wire.write(0x0D);
        default:
            Wire.write(0x10);
        }
    } else {
        Wire.write(CLOCK_STRETCH_SETTING_DISABLE);
        switch (_loop_accurately_level) { //LSB
        case LOOP_ACCURATELY_LEVEL_HIGH:
            Wire.write(0x00);
        case LOOP_ACCURATELY_LEVEL_COMMON:
            Wire.write(0x0B);
        default:
            Wire.write(0x16);
        }
    }
    Wire.endTransmission();
    Wire.requestFrom(_addr, 6);

    /*  
        [0] MSB for temperature
        [1] LSB for temperature
        [2] CRC
        [3] MSB for humidity
        [4] LSB for humidity
        [5] CRC 
    */
    for (int i = 0; i < 6; i++) {
        _read_buf[i] = Wire.read();
    }
}

float SHT31DIS::getTemperature(void)
{
    uint16_t bind_temp = _read_buf[0] << 8 | _read_buf[1];
    return -45.0 + 175.0 * bind_temp / 65535.0;
}

float SHT31DIS::getHumidity(void)
{
    uint16_t bind_humidity = _read_buf[3] << 8 | _read_buf[4];
    return 100.0 * bind_humidity / 65535.0;
}