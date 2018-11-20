#include "XBeeSmartPlug.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <XBee.h>

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
XBeeSmartPlug sensor = XBeeSmartPlug();
SoftwareSerial monitor(8, 9);

void setup()
{
    Serial.begin(9600);
    xbee.setSerial(Serial);
    monitor.begin(9600);
    monitor.println("Starting up!");
}

void loop()
{
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        if (xbee.getResponse().getApiId() == ZB_IO_SAMPLE_RESPONSE) { //api id is 0x92
            sensor.setFrameData(xbee.getResponse().getFrameData(), xbee.getResponse().getFrameDataLength());
            monitor.print("Light: ");
            monitor.println(sensor.getLight());
            monitor.print("Temperature: ");
            monitor.println(sensor.getTemperature());
            monitor.print("Current: ");
            monitor.println(sensor.getCurrent());
            monitor.println();
        }
    } else if (xbee.getResponse().isError()) {
        monitor.print("Error code: ");
        monitor.println(xbee.getResponse().getErrorCode());
    } else {
    }
}
