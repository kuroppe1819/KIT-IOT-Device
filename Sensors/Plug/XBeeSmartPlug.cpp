#include "XBeeSmartPlug.h"
#include <Arduino.h>

XBeeSmartPlug::XBeeSmartPlug()
{
}

XBeeSmartPlug::XBeeSmartPlug(uint8_t* frameData, uint8_t frameLength)
{
    _frameData = frameData;
    _frameLength = frameLength;
}

void XBeeSmartPlug::setFrameData(uint8_t* frameData, uint8_t frameLength)
{
    _frameData = frameData;
    _frameLength = frameLength;
}

float XBeeSmartPlug::getLight()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc1 = (_frameData[17] << 8) + _frameData[18];
    return (adc1 / 1023.0) * 1200.0;
}

float XBeeSmartPlug::getTemperature()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc2 = (_frameData[19] << 8) + _frameData[20];
    return ((((adc2 / 1023.0) * 1200.0) - 500.0) / 10.0);
}

float XBeeSmartPlug::getCurrent()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc3 = (_frameData[21] << 8) + _frameData[22];
    return ((((adc3 / 1023) * 1200) * (156 / 47) - 520) / 180 * 0.7071);
}
