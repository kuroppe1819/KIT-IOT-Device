#ifndef XBeeSmartPlug_h
#define XBeeSmartPlug_h

#include <Arduino.h>

class XBeeSmartPlug {
public:
    XBeeSmartPlug();
    XBeeSmartPlug(uint8_t* frameData, uint8_t frameLength);
    void setFrameData(uint8_t* frameData, uint8_t frameLegnth);
    float getTemperature();
    float getLight();
    float getCurrent();

private:
    uint8_t* _frameData;
    uint8_t _frameLength;
};

#endif