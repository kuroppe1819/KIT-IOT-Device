//---------------------------------------------------------------------------
//小林製作所向けセンシングデバイス用プログラム
//照度 : ＴＳＬ２５６１使用　照度センサーモジュール
//温度・照度 :　SHT31使用　高精度温湿度センサ
//各センサのデータを個別に１分間隔で送信。
//
//SCL:analog 5
//SDA:analog 4
//
//---------------------------------------------------------------------------
#include <XbeeS2C.h>              /* xbee用ライブラリ */
#include <Wire.h>
#include <SD.h>
#include "Adafruit_SHT31.h"
#include "TSL2561.h"
#include "RTClib.h"                         /*RTCライブラリ*/
RTC_DS1307 RTC;
Xbees2c xbee;

//=======================================通信用情報===========================================
Adafruit_SHT31 sht31 = Adafruit_SHT31(); //温湿度センサの定義
TSL2561 tsl(TSL2561_ADDR_FLOAT); //照度センサの定義
byte add_C[]={0,0,0,0,0,0,0,0}; //{0x00,0x13,0xA2,0x00,0x40,0xCA,0x9C,0xE7 };//親機の64bitアドレス
byte ping[]={0,0,0};
//============================================デバイス情報==========================================
byte s_data[12]={};//送信フレーム

byte ARMA_CODE[]={0x00,0x01,0x00,0x01};//エリア、機械コード

byte T_CODE[]={0x4c,0x00,0x00,0x01};//温度コード
byte H_CODE[]={0x53,0x00,0x00,0x01};//湿度コード
byte L_CODE[]={0x55,0x00,0x00,0x01};//照度コード
//============================================共用体=================================================
typedef union {
  float val;
  byte bin[4];
}f_bin;

//===========================================構造体==================================================
typedef struct {
  byte year;
  byte month;
  byte day;
  byte hour;
  byte min;
  byte sec;
} datetime;
//============================================大域変数===============================================

int RegTbl[8];// RTCのレジスタテーブル(8byte)
int PIN_SDCS=4;
byte frame_ID=0; 
byte pattern=0x00;
byte buff=0x01;
byte DEVICE_ADDRESS = 0x68;  //デバイスアドレス(スレーブ)

datetime Now;//現在時刻

//===========================================ユーザー定義関数========================================
float tsl2561_read_lux(){//照度計測
  uint32_t lum = tsl.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  float x=tsl.calculateLux(full, ir);
  return x;
}

void make_f(int flag,float data){
  byte code[4]={};
  f_bin b_data;
  b_data.val=data;

  for(int i=0;i<4;i++){
    s_data[i]=ARMA_CODE[i]; //エリア、機械コード
  }
  
  switch(flag){//センサコード識別
    case 1:
    for(int i=4;i<8;i++){
    s_data[i]=T_CODE[i-4];
    }
    break;
    case 2:
    for(int i=4;i<8;i++){
    s_data[i]=H_CODE[i-4];
    }
    break;
    case 3:
    for(int i=4;i<8;i++){
    s_data[i]=L_CODE[i-4];
    }
    break;
  }

  for(int i=8;i<12;i++){
    s_data[i]=b_data.bin[i-8];
  }
}


void RTC_NOW(){
  // レジスタのアドレスを先頭にする
  Wire.beginTransmission(DEVICE_ADDRESS);
    Wire.write(0x00);
  Wire.endTransmission();
 
  // I2Cスレーブに8byteのレジスタデータを要求する
  Wire.requestFrom(DEVICE_ADDRESS, 8);
 
  // 8byteのデータを取得する
  int i;
  for (i = 0; i < 8; i++) {
    while (Wire.available() == 0 ) {}
    RegTbl[i] = Wire.read();
  }
 
  // 現在日時を構造体datetimeへ格納
  Now.year=BCDtoDec(RegTbl[6]);
  Now.month=BCDtoDec(RegTbl[5] & 0x1F);
  Now.day=BCDtoDec(RegTbl[4] & 0x3F);
  Now.hour=BCDtoDec(RegTbl[2] & 0x3F);
  Now.min=BCDtoDec(RegTbl[1] & 0x7F);
  Now.sec=BCDtoDec(RegTbl[0] & 0x7F);
}
// 2進化10進数(BCD)を10進数に変換
byte BCDtoDec(byte value) {
  return ((value >> 4) * 10) + (value & 0x0F) ;
}

int xbee_join(){
    xbee.at("FR",0x00);
  delay(500);
  XBEE_RESULT xbee_result={1};
  bool flug=false;
  for(int i=0;i<1000;i++){
    xbee.at("CB",0x00);
    delay(3000);
    xbee.bytes(add_C,ping,sizeof(ping));//pin送信
    delay(500);
    for(int j=0;j<10;j++){
     byte type=xbee.rx_call(&xbee_result);
     Serial.println(type,HEX);
     Serial.println(xbee_result.STATUS,HEX);
     if(type==0x8b&&xbee_result.STATUS==0x00){
      //Serial.println("Succes!!");
      return 0;
      flug=true;
      break; 
     }
     else{
      flug=false;
      //Serial.println("Fail");
      }
    }
    
    xbee.at("FR",0x00);
   delay(1000); 
  }  
  return 1;
}

int transmit_check(XBEE_RESULT *result){
    for(int i=0;i<10;i++){
    byte type=xbee.rx_call(result);
    if(type==0x8B){
      sd_write(result);
      if(result->STATUS==0x00){
        return 0;
      }
      else{
        return 2;
      }
    }
    else{continue;}
  }
  return 1;
}

void sd_write(XBEE_RESULT *result){
  File LOGFILE=SD.open("LOG.txt",FILE_WRITE);
  if(LOGFILE){
    LOGFILE.print(frame_ID);
    LOGFILE.print(",");
    LOGFILE.print(Now.year,DEC);
    LOGFILE.print(",");
    LOGFILE.print(Now.month,DEC);
    LOGFILE.print(",");
    LOGFILE.print(Now.day,DEC);
    LOGFILE.print(",");
    LOGFILE.print(Now.hour,DEC);
    LOGFILE.print(",");
    LOGFILE.print(Now.min,DEC);
    LOGFILE.print(",");
    LOGFILE.print(Now.sec,DEC);
    LOGFILE.print(",");
    LOGFILE.print(result->STATUS,HEX);
    LOGFILE.print(",");
    LOGFILE.println(result->RETRY,HEX);
    LOGFILE.close();
  }
}
//---------------------------------------------------------------------------
// arduino setup
//---------------------------------------------------------------------------
void setup()
{
  Serial.begin(9600); 
  Wire.begin();                                   /* シリアル？使うためのセットアップ */
  RTC.begin();
  pinMode(7,INPUT);

  if (digitalRead(7)==HIGH) {//RTCが動作していないとき、時刻合わせ
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  
  sht31.begin(0x45);//温湿度センサの初期化
  
  tsl.begin();
  tsl.setGain(TSL2561_GAIN_16X);      // set 16x gain (for dim situations)
  tsl.setTiming(TSL2561_INTEGRATIONTIME_13MS);  // shortest integration time (bright light)

  SD.begin(PIN_SDCS);

   //エリア、機械コードの読み込み
  File file=SD.open("swadd.bin",FILE_READ);
  if(file){
    for(int i=0;i<4;i++){
      ARMA_CODE[i]=file.read();
    }
    for(int i=0;i<4;i++){
      T_CODE[i]=file.read();
    }
    for(int i=0;i<4;i++){
      H_CODE[i]=file.read();
    }
    for(int i=0;i<4;i++){
      L_CODE[i]=file.read();
    }
    file.close();
  }
}


//---------------------------------------------------------------------------
// arduino loop
//---------------------------------------------------------------------------

void loop(){

  float Temp,Humi,Lux;
  XBEE_RESULT xbee_result={1};                              /* 受信用の構造体関数の定義 */
  switch(pattern){
    
    case 0x00://ネットワーク参加
    xbee_join();
    pattern=buff;
    break;
    
    case 0x01://温度送信
    Temp = sht31.readTemperature();//温度読み取り
    if(! isnan(Temp)){
      RTC_NOW();
      make_f(1,Temp);
      frame_ID=xbee.bytes(add_C,s_data,sizeof(s_data));
    }
    if(transmit_check(&xbee_result)==1){
      pattern=0x00;
    }
    else{
      buff=pattern;
      pattern=0x02;
    }
    break;

    case 0x02:
    Humi = sht31.readHumidity();//湿度読み取り
    if(! isnan(Humi)){
      RTC_NOW();
      make_f(2,Humi);
      frame_ID=xbee.bytes(add_C,s_data,sizeof(s_data));
    }
    if(transmit_check(&xbee_result)==1){
      pattern=0x00;
    }
    else{
      buff=pattern;
      pattern=0x03;
    }
    break;

    case 0x03:
    Lux = tsl2561_read_lux();//照度読み取り
    if(! isnan(Lux)){
      RTC_NOW();
      make_f(3,Lux);
      frame_ID=xbee.bytes(add_C,s_data,sizeof(s_data));
    }
    if(transmit_check(&xbee_result)==1){
      pattern=0x00;
    }
    else{
      buff=pattern;
      pattern=0x01;
    }
    break;
    
  }
  
  delay(3000);//テスト用
  Serial.flush();
 
}

