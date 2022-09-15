/*
  2022.4.14
  Motor Control System with i2c slave = MCS_i2c_slave
  i2cでパラメータを受け取りステッピングモータを回す。
  回路はkicad/i2cSteppingMotorDriver/
*/


#include <Wire.h>

const int pulsePin = 3;
const int dirPin = 2;


void setup() {
  Wire.begin(36);                // join i2c bus with address
  Wire.onReceive(dataReceive); // register event
  Serial.begin(115200);           // start serial for output

  pinMode(pulsePin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  delay(10);
  digitalWrite(pulsePin, LOW);
  digitalWrite(dirPin, LOW);
}


//受信データ
// int 2byte long 4byte

//加速部分
int accInitStepInterval;//初期ステップ間隔(uSec)
int accStepDecrease;//ステップ間隔減少数(usec/step)

//定速部分：
int stepInterval;//ステップ間隔(uSec):
long stepCount;//ステップ数:　これは加速と減速部分を含む。-だと逆回転

//減速部分：
int decStepIncrease;//ステップ間隔増加数(usec/step)
int decFinalStepInterval;//最終ステップ(uSec)

//合計で14byte

void dataReceive(int Number) {
  byte data[13];


  if (Wire.available()) {
    for (int i = 0 ; i < 14 ; i++) {
      data[i] = Wire.read();
    }

    ((uint8_t*)&accInitStepInterval)[0] = data[0];
    ((uint8_t*)&accInitStepInterval)[1] = data[1];

    ((uint8_t*)&accStepDecrease)[0] = data[2];
    ((uint8_t*)&accStepDecrease)[1] = data[3];

    ((uint8_t*)&stepInterval)[0] = data[4];
    ((uint8_t*)&stepInterval)[1] = data[5];

    ((uint8_t*)&stepCount)[0] = data[6];
    ((uint8_t*)&stepCount)[1] = data[7];
    ((uint8_t*)&stepCount)[2] = data[8];
    ((uint8_t*)&stepCount)[3] = data[9];

    ((uint8_t*)&decStepIncrease)[0] = data[10];
    ((uint8_t*)&decStepIncrease)[1] = data[11];

    ((uint8_t*)&decFinalStepInterval)[0] = data[12];
    ((uint8_t*)&decFinalStepInterval)[1] = data[13];



    Serial.print("accInitStepInterval:");
    Serial.println(accInitStepInterval);

    Serial.print("accStepDecrease:");
    Serial.println(accStepDecrease);

    Serial.print("stepInterval:");
    Serial.println(stepInterval);

    Serial.print("stepCount:");
    Serial.println(stepCount);

    Serial.print("decStepIncrease:");
    Serial.println(decStepIncrease);

    Serial.print("decFinalStepInterval:");
    Serial.println(decFinalStepInterval);

    //回転方向
    if (stepCount<0) {
      digitalWrite(dirPin, LOW);//反時計回り
    } else {
      digitalWrite(dirPin, HIGH);//反時計回り
    }

    //モータを回す準備
    long accStepCount = (accInitStepInterval - stepInterval) / accStepDecrease; //加速部分のステップ数
    long decStepCount = (decFinalStepInterval - stepInterval) / decStepIncrease; //減速部分のステップ数
    long constSpeedCount = abs(stepCount)  - (accStepCount + decStepCount); //定速部分のステップ数

    Serial.print("accStepCount:");
    Serial.println(accStepCount);

    Serial.print("decStepCount:");
    Serial.println(decStepCount);

    Serial.print("constSpeedCount:");
    Serial.println(constSpeedCount);


    //モータを回す。受け取ったtargetStepステップ

    int intervalSec = accInitStepInterval;
    for (long i = 0 ; i < accStepCount ; i++) {
      digitalWrite(pulsePin, HIGH);
      delayMicroseconds(1);
      digitalWrite(pulsePin, LOW);
      delayMicroseconds(intervalSec);
      intervalSec = intervalSec - accStepDecrease;
    }
    intervalSec = stepInterval;
    for (long i = 0 ; i < constSpeedCount ; i++) {
      digitalWrite(pulsePin, HIGH);
      delayMicroseconds(1);
      digitalWrite(pulsePin, LOW);
      delayMicroseconds(intervalSec);
    }
    for (long i = 0 ; i < decStepCount ; i++) {
      digitalWrite(pulsePin, HIGH);
      delayMicroseconds(1);
      digitalWrite(pulsePin, LOW);
      delayMicroseconds(intervalSec);
      intervalSec = intervalSec + decStepIncrease;
    }

  }
}


void loop() {
}
