#include <RF24.h>
#include <driver/adc.h>
 
RF24 radio(4, 5); // CE, CSN 
const byte address[6] = "Car40";

int frame=0; 
  
struct JoyStickData{
  int x;
  int y;
  int b;
  String msg;
};

void setup() {
  
  Serial.begin(115200);
  Serial.println("Serial beginn");
  
  if (!radio.begin()){
    Serial.print(F("radio hardware not responding!!!"));
    while(1){}
  }                
  radio.openWritingPipe(address); 
  radio.setPALevel(RF24_PA_MIN);  
  radio.stopListening();       

  adc1_config_width(ADC_WIDTH_BIT_9);
}

JoyStickData  joyStick_data;
 
void loop()
{

  joyStick_data.x = adc1_get_raw(ADC1_CHANNEL_6);
  joyStick_data.y = adc1_get_raw(ADC1_CHANNEL_7);
  joyStick_data.msg = "hello";
  joyStick_data.b = frame;
  
  bool report = radio.write(&joyStick_data, sizeof(joyStick_data));

  if (report){
   Serial.print(F("Transmission sucessful!"));
  // Serial.print((String)address);
   
  } else {
    Serial.print(F("Transmission failed!"));
  }
  Serial.println();
  
  Serial.printf("x: %d y: %d\n",joyStick_data.x,joyStick_data.y);
  
  frame++;
  delay(100);
}
