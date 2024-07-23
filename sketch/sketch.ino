#include <FastLED.h>

#define NUM_LEDS 13
#define DATA_PIN 2
#define SPEAKER_PIN 12

int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A


CRGB leds[NUM_LEDS];

void setup() { 
  noTone(SPEAKER_PIN);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS); 
}

void loop() {
  for (int i = 0; i < NUM_LEDS; i++) {
    int red = map(i, 0, NUM_LEDS, 1, 255);
    leds[i] = CRGB(red, 0, 0); //
    FastLED.show();
    delay(100);  
  }

  tone(SPEAKER_PIN, tones[0]);
  delay(300);
  noTone(SPEAKER_PIN);
  
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
    FastLED.show();
    delay(50);  
  }

  tone(SPEAKER_PIN, tones[7]);
  delay(300);
  noTone(SPEAKER_PIN);

}