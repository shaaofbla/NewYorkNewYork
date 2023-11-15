#include "RF24.h"
#include "MeOrion.h"
#include "SPI.h"
#include "SimpleKalmanFilter.h"

MeDCMotor motor3(M1);

MeDCMotor motor4(M2);

MeBuzzer buzzer;

#define CE_PIN  10
#define CSN_PIN  9

#define LineFollower1_PIN  A2
#define LineFollower2_PIN  A3

#define GameOver_PIN A0


//nRF24
RF24 radio(CE_PIN, CSN_PIN); //CE, CSN



int car = 0;//BLAU : 0, GRÜN 1, ROT 2, VIOLETT: 3};
const byte address[][6] = {"Car10", "Car20", "Car30", "Car40"};


int motorSpeed = 255;

float lastX = 0;
float lastY = 0;

//Simple Klaman Filter
float e_mean = 4; //fast: 2,2,1   intermediat: 3,2,1; slow; 4,2,1
float e_est = 2;
float q = 1;

SimpleKalmanFilter kf_x = SimpleKalmanFilter(e_mean, e_est, q);// 4,4,1 init: 2,2,0.01
SimpleKalmanFilter kf_y = SimpleKalmanFilter(e_mean, e_est, q);

struct JoyStickData{
  uint16_t x;
  uint16_t y;
  bool b;
  String msg;
};

JoyStickData joyStickData;

void setup(){

  buzzer.setpin(8);
  
  Serial.begin(115200);
  delay(1000);
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
  radio.openReadingPipe(1,address[car]);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  //Serial.print(addressReading, BIN);
  
  //Linefollower
  pinMode(LineFollower1_PIN, INPUT);
  pinMode(LineFollower2_PIN, INPUT);

  //GameOver
  pinMode(GameOver_PIN, INPUT_PULLUP);

  buzzer.tone(440, 100);

}

boolean vehicleInBlackField(){
  int S1 = digitalRead(LineFollower1_PIN);
  int S2  = digitalRead(LineFollower2_PIN);
  /*
  Serial.print(S1);
  Serial.print("\t");
  Serial.print(S2);
  Serial.print("\t");
  Serial.print(!S1 || !S2);
  Serial.print("\n");
  */
  return !S1 && !S2 ;
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
  vehicleInBlackField();
  if(BallonGanz){
  
    if (radio.available()){
      if (vehicleInBlackField()){
        Serial.println("Black");

        motorSpeed = 255;
        radio.read(&joyStickData, sizeof(joyStickData));
        float x = kf_x.updateEstimate(joyStickData.x);
        float y = kf_y.updateEstimate(joyStickData.y);
  
        float x_est = mapCoordinate(x);
        float y_est = mapCoordinate(y);
        float x_rot = rotateX(x_est, y_est, PI/4);
        float y_rot = rotateY(x_est, y_est, PI/4);
  
  
        float cutoff = 0.2;
        
        if (x_rot >= cutoff || y_rot >= cutoff || x_rot < -cutoff || y_rot < -cutoff){
          float freq = 440*pow(2, (-y_rot*12+24*x_rot)/12);
          Serial.println(freq);
          buzzer.tone(freq, 50);
          motor3.run(x_rot * motorSpeed);
          motor4.run(y_rot * motorSpeed);
          lastX = x_rot;
          lastY = y_rot;
        } else {
          motor3.run(0);
          motor4.run(0);
        }
      } else {
        Serial.println("White\t");
        Serial.print(lastX);
        Serial.print("\t");
        Serial.println(lastY);
        motor3.run(-lastX * motorSpeed);
        motor4.run(-lastY * motorSpeed);
        delay(1000);
        motor3.run(0);
        motor4.run(0);
        delay(500);
        
      }
    }
    
  } else {
        motor3.run(0);
        motor4.run(0);
  }
}
