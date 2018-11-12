#ifndef XBeeLTHSensor_h
#define XBeeLTHSensor_h

#include <Arduino.h>

class XBeeLTHSensor {
public:
    XBeeLTHSensor();
    XBeeLTHSensor(uint8_t* frameData, uint8_t frameLength);
    void setFrameData(uint8_t* frameData, uint8_t frameLegnth);
    float getTemperature();
    float getLight();
    float getHumidity();

private:
    uint8_t* _frameData;
    uint8_t _frameLength;
};

#endif