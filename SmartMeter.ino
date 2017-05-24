#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#include "Ticker.h"
#include "ThingSpeak.h"

unsigned long myChannelNumber = 161152;
const char * APIKey = "9TL3XSVBRNBS9WQJ";
byte mac[] = { 0x5C, 0xCF, 0x7F, 0x86, 0xD1, 0xB8 };
WiFiClient client;
Ticker ticker;

char toSSID[] = "moyashinet";
char ssidPASSWD[] = "atsusuke54";

boolean timeflag = false;
float yen= 0;

void setup() {
  //デバッグ用にシリアルを開く
  Serial.begin(115200);
  Serial.println("esp8266 connected to network using DHCP");

  //WiFiクライアントモード設定
  WiFi.mode(WIFI_STA);

  //WiFiを繋ぐ前に、WiFi状態をシリアルに出力
  WiFi.printDiag(Serial);

  WiFi.begin(toSSID, ssidPASSWD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //WiFiの状態を表示
  WiFi.printDiag(Serial);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // 割り込み処理の設定
  ticker.attach(20, flag);

  //ThingSpeak
  ThingSpeak.begin(client);
}

void flag() {
  timeflag = true;        
}

void loop() {
  float value = system_adc_read();
  Serial.println(value);
  // アンペアを求める計算
  float io = value * 5 / 1024 * 3000 / (0.9 * 0.97 * 100);
  // 電気料金の算出    
  yen += io * 100 / 3600 / 1000 * 20.23;      
  Serial.print(String(yen) + " yen   ");
  Serial.println(String(io) + "A");
  
  if(timeflag == true){
    ThingSpeak.setField(1, yen);
    ThingSpeak.setField(2, io);
    ThingSpeak.writeFields(myChannelNumber, APIKey);
    Serial.println("push!");
    timeflag = false;
  }
  
  delay(500);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(toSSID, ssidPASSWD);
  }
}
