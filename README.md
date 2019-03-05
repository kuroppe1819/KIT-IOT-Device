# IoT device for Kobayashi Mfg
小林製作所で使用するIoTデバイスのプログラム

- KOBAYASHI_RX: 親機のプログラム
- KOBAYASHI_TX: 子機のプログラム
- library
    - SchmittTrigger: シュミットトリガーのサンプル
    - SHT31DIS: 温湿度センサ用のライブラリ
- PacketGenerator: シリアル解析ソフトウェアのテストに使用するプログラム
- Sensors
    - LTH: Digi XBee LTH Sensorsの動作確認用プログラム
    - Plug: Digi XBee Smart Plugの動作確認用プログラム
- codelist.txt: デバイスのSDカードに保存されているファイル。エリアコードを記述する。

## 使用しているライブラリ
- [xbee-arduino](https://github.com/andrewrapp/xbee-arduino): Arduino用のXBeeライブラリ
- [RTClib](https://github.com/adafruit/RTClib): RTC(DS1307)のライブラリ
- [Metro](http://playground.arduino.cc/Code/Metro): Arduinoのタイマーライブラリ
- [TSL2561-Arduino-Library](https://github.com/adafruit/TSL2561-Arduino-Library): 照度センサ(TSL2561)のライブラリ

## XBeeの設定
1. XCTUの[ダウンロード](https://www.digi.com/products/iot-platform/xctu)
1. XBeeS24CとPCをUSBで接続。ファームウェアバージョンを`4043`に設定する。
1. XBeeの設定を下記のように変更して書き込む

親機

|設定項目|値|
|:--|:--|
|ID: PAN ID|任意|
|CE: Coordinator Enable|有効|
|JV: Channel Verification|有効|
|JN: Join Notification|有効|
|AP: API Enable|2|

子機

|設定項目|値|
|:--|:--|
|ID: PAN ID|親機に設定したID|
|CE: Coordinator Enable|有効|
|JV: Channel Verification|有効|
|JN: Join Notification|有効|
|AP: API Enable|2|
|DH: Destination Address High|親機のMACアドレス上位8桁|
|DL: Destination Address Low|親機のMACアドレス下位8桁|
