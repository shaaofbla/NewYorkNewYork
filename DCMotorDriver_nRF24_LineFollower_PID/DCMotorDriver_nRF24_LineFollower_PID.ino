#include "RF24.h"
#include "MeOrion.h"
#include "SPI.h"
#include "SimpleKalmanFilter.h"

MeDCMotor motor3(M1);

MeDCMotor motor4(M2);

#define CE_PIN  10
#define CSN_PIN  9

#define LineFollower1_PIN  A2
#define LineFollower2_PIN  A3

#define GameOver_PIN A0



RF24 radio(CE_PIN, CSN_PIN); //CE, CSN

const byte address[6] = "Car10";


int lastX = 0;
int lastY = 0;

int motorSpeed = 255;

//Simple Klaman Filter
float e_mean = 4; //fast: 2,2,1   intermediat: 3,2,1; slow; 4,2,1
float e_est = 2;
float q = 1;
SimpleKalmanFilter kf_x = SimpleKalmanFilter(e_mean, e_est, q);// 4,4,1 init: 2,2,0.01
SimpleKalmanFilter kf_y = SimpleKalmanFilter(e_mean, e_est, q);

struct JoyStickData{
  int x;
  int y;
  int b;
  String msg;
};

JoyStickData joyStickData;

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Serial started.");
  Serial.println();
  //nRF24
  
  if (!radio.begin()){
    Serial.println(F("radio hardware not responding!!!"));
    while(1){}
  } else {
    Serial.print(F("Radio Hardware ok.\n"));
  }
  radio.openReadingPipe(1,address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  
  //Linefollower
  pinMode(LineFollower1_PIN, INPUT);
  pinMode(LineFollower2_PIN, INPUT);

  //GameOver
  pinMode(GameOver_PIN, INPUT_PULLUP);
  //digitalWrite(GamOver_PIN, HIGH);
  //tone(8, 440, 100);
  //delay(100);
  //noTone(8);

}

boolean vehicleInBlackField(){
  int S1 = digitalRead(LineFollower1_PIN);
  int S2  = digitalRead(LineFollower2_PIN);
  //Serial.print("Sensor: ");
  //Serial.print(S1);
  //Serial.print("\t");
  //Serial.print(S2);
  //Serial.print("\n");
  return !((S1 == 1) || (S2 == 1));
}

float mapCoordinate(int coord){
  float x = coord-255.;
  float exponent = 2;
  if (x >= 0){
    return pow( sin(x/ 255. * PI / 2.), exponent) ;
  } else {
    return pow( sin(x/ 255. * PI / 2.), exponent) *-1;
  }
  
}

float rotateX(float x, float y, float angle){
  return x * cos(angle) + y * sin(angle);
}

float rotateY(float x, float y, float angle){
  return -x * sin(angle) + y * cos(angle);
}

void loop(){
  boolean BallonGanz = digitalRead(GameOver_PIN);
  
  if(BallonGanz){
    if(vehicleInBlackField()){
    
      if (radio.available()){
        Serial.println("Black");
        motorSpeed = 255;
        radio.read(&joyStickData, sizeof(joyStickData));
        float x = kf_x.updateEstimate(joyStickData.x);
        float y = kf_y.updateEstimate(joyStickData.y);
  
        float x_est = mapCoordinate(x);
        float y_est = mapCoordinate(y);
        float x_rot = rotateX(x_est, y_est, PI/4);
        float y_rot = rotateY(x_est, y_est, PI/4);
        //float x_rot = rotateX(x, y, PI/4);
        //float y_rot = rotateY(x, y, PI/4);
        //Serial.print(joyStickData.x);
        //Serial.print("\t");
        Serial.print(x_rot);
        Serial.print("\t");
        Serial.print(y_rot);
        Serial.print("\t");
        Serial.println();
        motor3.run(x_rot * motorSpeed);
        motor4.run(y_rot * motorSpeed);
        lastX = x;
        lastY = y;
      } else {
        //Serial.println("No Signal received, check Address.");
      }
    
    } else {
      if (radio.available()){
        Serial.println("White");
        motorSpeed = 255;
        radio.read(&joyStickData, sizeof(joyStickData));
        float x = mapCoordinate(joyStickData.x);
        float y = mapCoordinate(joyStickData.b);
        float x_est = kf_x.updateEstimate(x);
        float y_est = kf_y.updateEstimate(y);
        float x_rot = rotateX(x_est, y_est, PI/4);
        float y_rot = rotateY(x_est, y_est, PI/4);
        motor3.run(x_rot * motorSpeed);
        motor4.run(y_rot * motorSpeed);
        lastX = x;
        lastY = y;
    }
    
  }
  } else {
    Serial.println("Ballon geplatzt, GAME OVER");
    motor3.run(0);
    motor4.run(0);
  }
}
