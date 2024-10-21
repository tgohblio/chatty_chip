#include <FastLED.h>
#include <Arduino.h>

#define NUM_LEDS 1
#define DATA_PIN GPIO_NUM_48
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() { 
	FastLED.addLeds<WS2812B, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS); 
	FastLED.setBrightness(10);
}

void loop() {
	leds[0] = CRGB::Yellow; FastLED.show(); delay(500);
	leds[0] = CRGB::Black; FastLED.show(); delay(500);

}