#include <XbeeS2C.h>
#include <SD.h>                             /*SDメモリ用のライブラリの読み出し*/
#include <Wire.h>                           /*I2Cライブラリ*/
#include "RTClib.h"                         /*RTCライブラリ*/
RTC_DS1307 RTC;
Xbees2c xbee;

//===============================デバイス情報==================================
#define ADJUST_PIN 7
#define DATA_LEN 19
#define PIN_SDCS 4 /*CS信号のポート規定*/
#define LITE       /*XBEE→LITEモード*/

byte ArMa_code[4]={};                 /*エリアコード*/

//センサ別コード
byte CT_code[4]={0x43,0x00,0x00,0x01};               /*電流"C001"*/
byte Light_code[4]={0x4c,0x00,0x00,0x01};            /*照度"L001"*/
byte Temp_code[4]={0x54,0x00,0x00,0x01};             /*温度"T001"*/
byte Humi_code[4]={0x55,0x00,0x00,0x01};             /*湿度"H001"*/
//SDカード用センサchar型コード
char CT_NAME[]="A1M1C001";
char L_NAME[]="A1M1L001";
char T_NAME[]="A1M1T001";
char H_NAME[]="A1M1H001";
//=============================================================================

//===============================ユーザ構造体==================================
typedef struct {
  byte bin_year;
  byte bin_month;
  byte bin_day;
  byte bin_hour;
  byte bin_min;
  byte bin_sec;
} datetime;

typedef union {
  float val;
  byte binary[4];
} uf;

//=============================================================================

//=================================大域変数====================================

int RegTbl[8];// RTCのレジスタテーブル(8byte) 
unsigned int S_NO = 1;//シーケンス番号
byte DEVICE_ADDRESS = 0x68;  //デバイスアドレス(スレーブ)
byte s_data[24]={}; 

datetime Now;//現在時刻

  File file;//SDファイルの定義
//s_no.val=0;
int cnt;
//=============================================================================

//==============================ユーザ定義関数=================================
byte check_f(){
  byte x;
  unsigned int sum;
  sum=0;
  for(int i=2;i<23;i++){
    sum+=s_data[i];
  }
  x=0xFF-(sum & 0xFF);
  return x;
}


void make_f(float data){
  uf b_data;
  b_data.val=data;
  s_data[0]=0xAA;//データヘッダー
  s_data[1]=0xAB;
  s_data[2]=S_NO >> 8 & 0xFF;
  s_data[3]=S_NO & 0xFF;

  for(int i=0;i<4;i++){
    s_data[i+4]=ArMa_code[i];
  }
  
  for(int i=8;i<12;i++){
    s_data[i]=CT_code[i-8];
  }
  s_data[12]=Now.bin_year;
  s_data[13]=Now.bin_month;
  s_data[14]=Now.bin_day;
  s_data[15]=Now.bin_hour;
  s_data[16]=Now.bin_min;
  s_data[17]=Now.bin_sec;
  for(int i=18;i<22;i++){
    s_data[i]=b_data.binary[i-18];
  }
  s_data[22]=0x00;
  s_data[23]=check_f();
 }

void make_f_rx(XBEE_RESULT *result){
    s_data[0]=0xAA;
    s_data[1]=0xAB;
    s_data[2]=(S_NO >> 8) & 0xFF;
    s_data[3]=S_NO & 0xFF;
    for(int i=0;i<8;i++){
      s_data[i+4]=result->DATA[i];
    }
    s_data[12]=Now.bin_year;
    s_data[13]=Now.bin_month;
    s_data[14]=Now.bin_day;
    s_data[15]=Now.bin_hour;
    s_data[16]=Now.bin_min;
    s_data[17]=Now.bin_sec;
    for(int i=0;i<4;i++){
      s_data[i+18]=result->DATA[i+8];
    }
    s_data[22]=0x00;
    s_data[23]=check_f();
}

/****************　SD書き込み　*******************/


void SD_write(float val,int type){
  File logFile = SD.open("datalog.txt", FILE_WRITE);
  // もしファイルが開けたら値を書き込む
  if (logFile) {
    switch(type){
      case 1:
      logFile.print(CT_NAME);
      break;
      case 2:
      logFile.print(T_NAME);
      break;
      case 3:
      logFile.print(H_NAME);
      break;
      case 4:
      logFile.print(L_NAME);
      break;
    }
    logFile.print(",");
    logFile.print(Now.bin_year,DEC);
    logFile.print(",");
    logFile.print(Now.bin_month,DEC);
    logFile.print(",");
    logFile.print(Now.bin_day,DEC);
    logFile.print(",");
    logFile.print(Now.bin_hour,DEC);
    logFile.print(",");
    logFile.print(Now.bin_min,DEC);
    logFile.print(",");
    logFile.print(Now.bin_sec,DEC);
    logFile.print(",");
    logFile.println(val);
    logFile.close();
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
  Now.bin_year=BCDtoDec(RegTbl[6]);
  Now.bin_month=BCDtoDec(RegTbl[5] & 0x1F);
  Now.bin_day=BCDtoDec(RegTbl[4] & 0x3F);
  Now.bin_hour=BCDtoDec(RegTbl[2] & 0x3F);
  Now.bin_min=BCDtoDec(RegTbl[1] & 0x7F);
  Now.bin_sec=BCDtoDec(RegTbl[0] & 0x7F);
}
// 2進化10進数(BCD)を10進数に変換
byte BCDtoDec(byte value) {
  return ((value >> 4) * 10) + (value & 0x0F) ;
}
//=============================================================================

//==================================SETUP======================================
void setup() {
  Serial.begin(9600); // シリアルポートを9600bpsで開く
  Wire.begin();//i2cポートを開く
  RTC.begin();//RTCを開く
  pinMode(INPUT,ADJUST_PIN);//時刻合わせピン

  if (digitalRead(ADJUST_PIN)==HIGH) {//デジタル7ピンがLOWのとき、時刻合わせ
     RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  while(SD.begin(PIN_SDCS)==false){    //SDカードの使用開始命令
   delay(500);
  }
  
  xbee.at("FR",0x00);//xbeeソフトウェアリセット

  //エリア、機械コードの読み込み
  File file=SD.open("swadd.bin",FILE_READ);
  if(file){
    for(int i=0;i<4;i++){
      ArMa_code[i]=file.read();
    }
    for(int i=0;i<4;i++){
      CT_code[i]=file.read();
    }
    file.close();
  }
}
//=============================================================================

//====================================LOOP=====================================
void loop() {
  cnt++;//ループ回数
  float S_CT=0;


//-----------------------------電流センサ--------------------------------------
  //Serial.println("CT");
  S_CT=(float)analogRead(0)*5/1024;//CT内電流 IEEE754_32bit 浮動小数点
  if(S_CT>=0.0){
    float sum;
    for(int i=0;i<10;i++){
        sum+=(float)analogRead(0)*5/1024;//CT内電流 IEEE754_32bit 浮動小数点
    }
    S_CT=sum/10;
    RTC_NOW();
    make_f(S_CT);
    Serial.write(s_data,24);
    S_NO++;

    SD_write(S_CT,1);

    delay(500);
  }
//-----------------------------------------------------------------------------
  XBEE_RESULT xbee_result={0};
  byte type=xbee.rx_call(&xbee_result);
  if(type==0x90){//もし受信ステータスがデータフレームならば
    uf Temp,Humi,Lux;
    RTC_NOW();
    make_f_rx(&xbee_result);
    if(s_data[8]!=0x00){
    Serial.write(s_data,24);//フレーム送信
    S_NO++;
    }
//研究データ収集用にfloat型変数へ変換↓
    
    switch(s_data[8]){
      case 0x54:
      for(int i=0;i<4;i++){
        Temp.binary[i]=s_data[i+18];
      }
      SD_write(Temp.val,2);
      break;
      case 0x48:
      for(int i=0;i<4;i++){
        Humi.binary[i]=s_data[i+18];
      }
      SD_write(Humi.val,3);
      break;
      case 0x4c:
      for(int i=0;i<4;i++){
        Lux.binary[i]=s_data[i+18];
      }
      SD_write(Lux.val,4);
      break;
    }
  }
  if(cnt>100){
    cnt=0;
    Serial.flush();}

delay(10);
}
//=============================================================================

//===================================END=======================================

