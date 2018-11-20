#include "XBeeLTHSensor.h"
#include <Arduino.h>

XBeeLTHSensor::XBeeLTHSensor()
{
}

XBeeLTHSensor::XBeeLTHSensor(uint8_t* frameData, uint8_t frameLength)
{
    _frameData = frameData;
    _frameLength = frameLength;
}

void XBeeLTHSensor::setFrameData(uint8_t* frameData, uint8_t frameLength)
{
    _frameData = frameData;
    _frameLength = frameLength;
}

float XBeeLTHSensor::getLight()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc1 = (_frameData[17] << 8) + _frameData[18];
    return (adc1 / 1023.0) * 1200.0;
}

float XBeeLTHSensor::getTemperature()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc2 = (_frameData[19] << 8) + _frameData[20];
    return ((((adc2 / 1023.0) * 1200.0) - 500.0) / 10.0);
}

float XBeeLTHSensor::getHumidity()
{
    if (!_frameData || !_frameLength) {
        return -1;
    }
    int adc3 = (_frameData[21] << 8) + _frameData[22];
    return (((((adc3 / 1023.0) * 1200.0) * 108.2 / 33.2) / 5000.0 - 0.16) / 0.0062);
}
