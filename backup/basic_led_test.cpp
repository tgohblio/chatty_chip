#include "Arduino.h"

#define MIC_I2S_SD       32 // serial data out
#define MIC_I2S_WS       34 // left/right word select clock
#define MIC_I2S_SCK      35 // serial bit clock
// #define MIC_I2S_LRC   // left/right channel select is grounded

// put your setup code here, to run once
void setup()
{
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);
}

// put your main code here, to run repeatedly
void loop()
{
    digitalWrite(LED_PIN, HIGH);    // sets the digital pin 13 on
    Serial.println("ED ON");
    delay(500);                    // waits for a second
    digitalWrite(LED_PIN, LOW);     // sets the digital pin 13 off
    Serial.println("LED OFF");
    delay(500);                    // waits for a second
}
