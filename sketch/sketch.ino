#include <FastLED.h>

#define NUM_LEDS 13
#define LED_DATA_PIN 2
#define SPEAKER_PIN 12
#define PLAYER1_BUTTON_PIN 4
#define PLAYER2_BUTTON_PIN 5
#define PLAYER1_LED_PIN 13
#define PLAYER2_LED_PIN 14

int tones[] = { 261, 277, 294, 311, 330, 349, 370, 392, 415, 440 };
//            mid C  C#   D    D#   E    F    F#   G    G#   A

CRGB leds[NUM_LEDS];

int player1Button = 0;
int player2Button = 0;

enum gameStates {
                  IDLE,
                  PLAYING
                };

gameStates gameState;

unsigned long timer;
//P1 -> P2 1 -> NUM_LEDS
//P2 -> P1 NUM_LEDS -> 1 
int currentLed = 0;
//P1 -> P2 direction = 0
//P2 -> P1 direction = 1
int direction = 0;
unsigned long speed = 5000;

void setup() {
  Serial.begin(9600);

  gameState = IDLE;

  pinMode(PLAYER1_BUTTON_PIN, INPUT);
  pinMode(PLAYER2_BUTTON_PIN, INPUT);
  pinMode(PLAYER1_LED_PIN, OUTPUT);
  pinMode(PLAYER2_LED_PIN, OUTPUT);

  noTone(SPEAKER_PIN);
  FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(leds, NUM_LEDS);

  resetLed();
}

void resetLed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
  
}

void loop() {
  FastLED.show();
  
  if (direction) {
    delay(0);
  }

  player1Button = digitalRead(PLAYER1_BUTTON_PIN);
  player2Button = digitalRead(PLAYER2_BUTTON_PIN);

  if (player1Button == HIGH) {
    Serial.println("P1 pressed");
  }

  if (player2Button == HIGH) {
    Serial.println("P2 pressed");
  }

  switch (gameState) {
    case IDLE:
      if (player1Button == HIGH || player2Button == HIGH) {
        gameState = PLAYING;
        currentLed = 1;
        timer = millis();
        if (player1Button == HIGH) {
          Serial.println("CHANGE DIRECTION 0");
          direction = 0;
        } else {
          Serial.println("CHANGE DIRECTION 1");
          direction = 1;
        }
      }
      
      break;
    case PLAYING:
      long passed = millis() - timer;
      if (passed >= speed) {
        if (currentLed < NUM_LEDS) {
          currentLed ++;
          Serial.println(passed);
        }
        timer = millis();
      }
      if (direction == 0) {
        for (int i = 0; i <= NUM_LEDS; i++) {
          if (i < currentLed) {
            leds[i] = CRGB(50, 0, 0);
          } else {
            leds[i] = CRGB(0, 0, 0);
          }
          //FastLED.show();
        }
      } else {
        for (int i = 0; i <= NUM_LEDS; i++) {
          if (i > NUM_LEDS - currentLed + 1) {
            leds[i] = CRGB(50, 0, 0);
          } else {
            leds[i] = CRGB(0, 0, 0);
          }
          //FastLED.show();
        }
      }

      if (currentLed == NUM_LEDS) {
        if (direction == 0) {
          direction = 1;
          Serial.println("P1 -> P2");
        } else {
          Serial.println("P2 -> P1");
          direction = 0;
        }
        currentLed = 1;
      }
      //FastLED.show();

      break;
  }

}