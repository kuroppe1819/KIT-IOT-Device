#ifndef SHT31DIS_H
#define SHT31DIS_H

#include <Arduino.h>
#include <Wire.h>

#define SHT31DIS_ADDR_LOW 0x44
#define SHT31DIS_ADDR_FLOAT 0x45

#define CLOCK_STRETCH_SETTING_ENABLE 0x2C
#define CLOCK_STRETCH_SETTING_DISABLE 0x24

#define LOOP_ACCURATELY_LEVEL_HIGH 0x00
#define LOOP_ACCURATELY_LEVEL_COMMON 0x01
#define LOOP_ACCURATELY_LEVEL_LOW 0x02

class SHT31DIS {
public:
    SHT31DIS(int8_t addr, boolean clock_stretch, int8_t loop_accurately_level);
    void begin(void);
    float getTemperature(void);
    float getHumidity(void);

private:
    int8_t _addr;
    boolean _clock_stretch;
    int8_t _loop_accurately_level;
    byte _read_buf[6];
};

#endif