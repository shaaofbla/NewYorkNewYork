
#include <RF24.h>
#include <driver/adc.h>
#include <WiFi.h>

#include "OSCCommunication.h"

RF24 radio(4, 5); // CE, CSN
const byte address[6] = "Car10";

OscCommunication Osc;

struct JoyStickData {
  uint16_t x;
  uint16_t y;
  bool b;
  String msg;
};

void setup() {
  Serial.begin(115200);
  Serial.println("Serial begin!");

  Osc.init();
  Serial.println("Osc ok! ");

  if (!radio.begin()) {
    Serial.print(F("radio hardware not responding!!!"));
    while (1) {
      // Wait indefinitely if radio hardware is not responding
    }
  }
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  adc1_config_width(ADC_WIDTH_BIT_9);
}

JoyStickData joyStick_data;

void loop() {
  Osc.Update();
  if (Osc.gameState) {
    joyStick_data.x = adc1_get_raw(ADC1_CHANNEL_6);
    joyStick_data.y = adc1_get_raw(ADC1_CHANNEL_7);
    joyStick_data.msg = "hello";

    bool report = radio.write(&joyStick_data, sizeof(joyStick_data));

    if (report) {
      Serial.print(F("Transmission successful!"));

    } else {
      Serial.print(F("Transmission failed!"));
    }

    Serial.println();
    Serial.printf("x: %d y: %d\n", joyStick_data.x, joyStick_data.y);

    delay(100);
  }
}
