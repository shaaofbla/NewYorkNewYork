
#include <RF24.h>
#include <driver/adc.h>
#include <Ticker.h>

#include "OSCCommunication.h"

RF24 radio(4, 5); // CE, CSN



int car = 1;//BLAU : 0, GRÜN 1, ROT 2, VIOLETT: 3};
const byte address[][6] = {"Car10", "Car20", "Car30", "Car40"};



struct JoyStickData {
  uint16_t x; // X-axis value of joystick
  uint16_t y; // Y-axis value of joystick
  bool b; // Button state of joystick
  String msg; // Message to be sent
};



JoyStickData joyStick_data; // Joystick data object

OscCommunication Osc;

Ticker timer;

void setup() {

  Serial.begin(115200);
  delay(1000);
  Serial.println("Serial begin!");

  Osc.init();
  Serial.println("Osc ok! ");

  if (!radio.begin()) {
    Serial.print(F("radio hardware not responding!!!"));
    while (1) {
      // Wait indefinitely if radio hardware is not responding
    }
  }
  radio.openWritingPipe(address[car]);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  adc1_config_width(ADC_WIDTH_BIT_9);

  // Task manager
  timer.attach_ms(100, sendJoyStickData);
}

void sendJoyStickData(){
  Serial.print("Sending data.");
  Serial.print("\t");
  Serial.println(joyStick_data.x);
  bool report = radio.write(&joyStick_data, sizeof(joyStick_data)); 
  if (report) {
    Serial.print(F("Transmission successful!"));
  } else {
    Serial.print(F("Transmission failed!"));
  }
  //radio.startListening();
}

void loop() {
  Osc.Update();
  if (Osc.gameState) {
    joyStick_data.x = adc1_get_raw(ADC1_CHANNEL_6);
    joyStick_data.y = adc1_get_raw(ADC1_CHANNEL_7);
    //Serial.println();
    //Serial.printf("x: %d y: %d\n", joyStick_data.x, joyStick_data.y);
  } else {
    joyStick_data.x = (uint16_t) 255;
    joyStick_data.y = (uint16_t) 255;

    //Serial.println();
    //Serial.println("Car off.");
  }
}
