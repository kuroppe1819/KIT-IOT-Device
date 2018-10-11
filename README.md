# IoT device for Kobayashi Mfg
小林製作所で使用するIoTデバイスのプログラム

- KOBAYASHI_RX: 親機のプログラム
- KOBAYASHI_TX: 子機のプログラム
- PacketGenerator: シリアル解析ソフトウェアのテストに使用するプログラム

##使用しているライブラリ
- [xbee-arduino](https://github.com/andrewrapp/xbee-arduino): Arduino用のXBeeライブラリ
- [RTClib](https://github.com/adafruit/RTClib): RTC(DS1307)のライブラリ
- [Metro](http://playground.arduino.cc/Code/Metro): Arduinoのタイマーライブラリ
- [TSL2561-Arduino-Library](https://github.com/adafruit/TSL2561-Arduino-Library): 照度センサ(TSL2561)のライブラリ