/*
  2022.4.14
  Motor Control System with i2c master = MCS_i2c_master
  シリアル通信でパラメータを受け取りi2cでSlaveに送信する。
  i2cでパラメータを受け取りステッピングモータを回す。
  回路はkicad/i2cSteppingMotorDriver/

  シリアルモニタから送るデータは
  I2Cアドレス、加速パルスインターバル初期値、加速インターバル減少数、定速インターバル、合計パルス数、減速インターバル増加数、減速パルスインターバル最終値
  の形式
　例えば：
  8, 800, 1, 10, 6400, 5, 800
  9, 1200, 1, 200, 6400, 1, 100
  10, 800, 1, 10, 25600, 10, 800
*/

#include <Wire.h>

const int LED1 = 2;
const int LED2 = 3;
const int LED3 = 4;

//String parameter[7] = {"\0"};

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);


  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);

  Serial.println("start");
}


/*
  int: accInitStepInterval //初期ステップ間隔(uSec)
  int: accStepDecrease; //ステップ間隔減少数(usec/step)

  //定速部分：
  int: stepInterval; //ステップ間隔(uSec):
  long: stepCount; //ステップ数:　これは加速と減速部分を含む。-だと逆回転

  //減速部分：
  int: decStepIncrease; //ステップ間隔増加数(usec/step)
  int: decFinalStepInterval; //最終ステップ(uSec)
*/

void sendStepi2c(int address, int accInitStepInterval, int  accStepDecrease, int stepInterval, long stepCount, int decStepIncrease, int decFinalStepInterval) {

  char values[13];

  values[0] = lowByte(accInitStepInterval);
  values[1] = highByte(accInitStepInterval);
  values[2] = lowByte(accStepDecrease);
  values[3] = highByte(accStepDecrease);
  values[4] = lowByte(stepInterval);
  values[5] = highByte(stepInterval);
  values[6] = stepCount & 0x000000ff;
  values[7] = (stepCount & 0x0000ff00) >> 8;
  values[8] = (stepCount & 0x00ff0000) >> 16;
  values[9] = (stepCount & 0xff000000) >> 24;
  values[10] = lowByte(decStepIncrease);
  values[11] = highByte(decStepIncrease);
  values[12] = lowByte(decFinalStepInterval);
  values[13] = highByte(decFinalStepInterval);

  Wire.beginTransmission(address); // transmit to device #8
  Wire.write(values, 14);
  Wire.endTransmission();    // stop transmitting
}

//https://algorithm.joho.info/arduino/string-split-delimiter/から引用
int split(String data, char delimiter, String *dst) {
  int index = 0;
  int arraySize = (sizeof(data) / sizeof((data)[0]));
  // Serial.print("arraySize=");
  //Serial.println(arraySize);
  int datalength = data.length();
  //Serial.print("datalength=");
  //Serial.println(datalength);
  for (int i = 0; i < datalength; i++) {
    char tmp = data.charAt(i);
    //Serial.println(tmp);
    if ( tmp == delimiter ) {
      index++;

      // if ( index > (arraySize - 1)) return -1;
    }
    else dst[index] += tmp;
    //Serial.println( dst[index]);
  }
  return (index + 1);
}


long stringTolongNumber(String str) {
  long longNumber = 0;
  long m = 1;
  int datalength = str.length();
  //Serial.println("~~~~~~~~");
  for (int i = datalength-1; i > 0; i--) {
    char tmp = str.charAt(i);
    if (tmp != "") {
      //Serial.println(tmp);
      String tmpString = String(tmp);
      if (tmpString == "-") {
        longNumber = -longNumber;
      } else {
        //Serial.println(tmpString);
        longNumber = longNumber +  tmpString.toInt() * m;
        m = m * 10;
      }
      //Serial.println(longNumber);
    }
  }
  //Serial.println("~~~~~~~~");
  return longNumber;
}

void loop() {
  String parameterLine;//受信したコマンドの文字列

//シリアル通信を受信した場合の処理
  while (Serial.available()) {
    String parameter[10] = {"\n"};

    parameterLine = Serial.readStringUntil('\n');//改行までシリアルの読み込み
    Serial.println("parameterLine=" + parameterLine);
    int index = split(parameterLine, ',', parameter);//カンマで区切り配列parameterに入れる。

    if (index == 7) {//パラメータの数が7の場合は正常処理
      int address = parameter[0].toInt();
      int accInitStepInterval = parameter[1].toInt();
      int  accStepDecrease = parameter[2].toInt();
      int stepInterval = parameter[3].toInt();

      //longの範囲(-2147483648〜2147483647)の値を扱えるように文字列からlongの値に変換する
      long stepCount = stringTolongNumber(parameter[4]);
      //Serial.print("stepCount=");
      //Serial.println(stepCount);
      // long stepCount = parameter[4].toDouble();//
      int decStepIncrease = parameter[5].toInt();
      int decFinalStepInterval = parameter[6].toInt();

      //i2cでslaveに送信する
      sendStepi2c(address, accInitStepInterval, accStepDecrease, stepInterval, stepCount, decStepIncrease, decFinalStepInterval);
    } else {
      //パラメータの数が違う
      Serial.println("parameter ERROR");
    }
  }
}


//I2Cアドレス、加速パルスインターバル初期値、加速インターバル減少数、定速インターバル、合計パルス数、減速インターバル増加数、減速パルスインターバル最終値
/*シリアルコンソールから以下の文字列を送信する
  8, 800, 1, 10, 6400, 5, 800
  9, 1200, 1, 200, 6400, 1, 100
  10, 800, 1, 10, 25600, 10, 800
*/
