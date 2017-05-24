#include <ESP8266WiFi.h>
extern "C" {
#include "user_interface.h"
}
#include "Ticker.h"
#include "ThingSpeak.h"

unsigned long myChannelNumber = "";
const char * APIKey = "";
byte mac[] = "";
WiFiClient client;
Ticker ticker;

char toSSID[] = "";
char ssidPASSWD[] = "";

volatile float CurrentValue = 0;
volatile float PowerRates = 0;

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
  
  // 20秒に1回pushData関数を呼ぶ
  ticker.attach(20, pushData);

  //ThingSpeak
  ThingSpeak.begin(client);
}

void pushData(float value){
    ThingSpeak.setField(1, PowerRates);
    ThingSpeak.setField(2, CurrentValue);
    ThingSpeak.writeFields(myChannelNumber, APIKey);
    Serial.println("push!");
    Serial.print(String(CurrentValue) + "A");
    Serial.println(String(PowerRates) + " yen   ");
}

void toElectryRates(float value){
  // アンペアを求める計算
  CurrentValue = value * 5 / 1024 * 3000 / (0.9 * 0.97 * 100);
  Serial.print(String(CurrentValue) + "A");
  // 電気料金の算出    
  PowerRates += CurrentValue * 100 / 3600 / 1000 * 20.23;
  Serial.println(String(PowerRates) + " yen   ");
}

void loop() {
  // TOUTピンからディジタル値を取得    
  toElectryRates(system_adc_read());

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(toSSID, ssidPASSWD);
  }
}
