#include <EEPROM.h>
#include <Wire.h>
#include <FastLED.h>

#define PIN_SPEAKER 12
#define LED_PIN     5
#define NUM_LEDS    9
#define INITIAL_BALL_SPEED 200

CRGB leds[NUM_LEDS];

enum gameStates {
  IDLE,
  KICK_0_1,
  KICK_1_0
};

typedef struct { 
  int pin;
  byte ledSignal;
} button;

const button buttons[] {
    {2, (byte)0b11111110}, //P1
    {3, (byte)0b11111101}, //P2
};

byte buttonPressStates;
byte buttonReadyStates;
gameStates gameState;
short ballPosition;
unsigned long timer;
unsigned long lastTimeBallPosition;
int ballSpeed;

bool isButtonPressed(byte button) {
  return bitRead(buttonPressStates, button) == 1;
}

void stopButtonLeds() {
  Wire.beginTransmission(0x20);
  Wire.write((byte)0b11111111);
  Wire.endTransmission();
  noTone(PIN_SPEAKER);
}

void readButtons() {
  for (int i = 0; i < sizeof(buttons)/sizeof(button); i++) {
    if (digitalRead(buttons[i].pin)) {
      if (bitRead(buttonReadyStates, i)) {
        bitClear(buttonReadyStates, i);
        buttonPressStates = bitSet(buttonPressStates, i);
      } else {
        buttonPressStates = bitClear(buttonPressStates, i);
      }
    } else {
      bitSet(buttonReadyStates, i);
      buttonPressStates = bitClear(buttonPressStates, i);
    }
  }
}

void buttonLedOn(int ledIndex){
  button b = buttons[ledIndex];
  Wire.beginTransmission(0x20);
  Wire.write(b.ledSignal);
  Wire.endTransmission();
}

void setup() {
  Wire.begin();

  Serial.begin(9600);

  for(int i = 0; i < sizeof(buttons)/sizeof(button); ++i) {
    pinMode(buttons[i].pin, INPUT);
  }

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS); 

  delay(100);

  stopButtonLeds();

  FastLED.clear(); 
  FastLED.show();

  ballSpeed = INITIAL_BALL_SPEED;

  Serial.println("setup OK");
}

void player1Wins() {
}

void increaseBallSpeed(short position) {
  int delta = position * 30;
  ballSpeed = INITIAL_BALL_SPEED - delta;
}

void loop() {
  readButtons();
  timer = millis();

  switch (gameState) {
    case IDLE:
      if (isButtonPressed(0)) {
        gameState = KICK_0_1;

        buttonLedOn(0);
        ballPosition = 0;
        lastTimeBallPosition = timer;
        tone(PIN_SPEAKER, 100, 100);

        leds[0] = CRGB::Red;
        FastLED.show();

      } else if (isButtonPressed(1)) {
        gameState = KICK_1_0;

        buttonLedOn(1);
        ballPosition = 0;
        lastTimeBallPosition = timer;
        tone(PIN_SPEAKER, 200, 100);

        leds[NUM_LEDS - 1] = CRGB::Red;
        FastLED.show();
        
      }
    break;

    case KICK_0_1:
      if (timer - lastTimeBallPosition > ballSpeed) {
        
        ballPosition ++;
        lastTimeBallPosition = timer;

        if (ballPosition < NUM_LEDS) {
          FastLED.clear(); 
          leds[ballPosition] = CRGB::Red;
          FastLED.show();
        } else {
          FastLED.clear(); 
          FastLED.show();
          stopButtonLeds();
          gameState = IDLE;
        }
      }

      if (isButtonPressed(1)) { //opponent responds
        if (ballPosition <= 4) { //too early. 0 wins
          player1Wins();
        } else {
          ballPosition = NUM_LEDS - ballPosition;
          increaseBallSpeed(ballPosition);
          gameState = KICK_1_0;
        }
      }

    break;

    case KICK_1_0:
      if (timer - lastTimeBallPosition > ballSpeed) {
        
        ballPosition ++;
        lastTimeBallPosition = timer;

        if (ballPosition < NUM_LEDS) {
          FastLED.clear(); 
          leds[NUM_LEDS - 1 - ballPosition] = CRGB::Red;
          FastLED.show();
        } else {
          FastLED.clear(); 
          FastLED.show();
          stopButtonLeds();
          gameState = IDLE;
        }
      }

    break;

    default:
    
    break;
  }
 
}