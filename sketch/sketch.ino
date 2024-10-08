#include <EEPROM.h>
#include <Wire.h>
#include <FastLED.h>

#define PIN_SPEAKER 12
#define LED_PIN     5
#define NUM_LEDS    9
#define INITIAL_BALL_SPEED 300

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
byte animationButton;
bool sound = false;

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

void rotateAnimation() {
  int transTable[2] = {0,1};
  if (++animationButton > 3) animationButton = 0;
  buttonLedOn(transTable[animationButton]);
}

void player1Wins() {
  byte melody[] = { 250, 196, 196, 220, 196,0, 247, 250};

  byte noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4 };

  int noteDuration = 0;
  int pauseBetweenNotes = 0;

 for (byte thisNote = 0; thisNote < sizeof(melody)/sizeof(byte); thisNote++) {
    noteDuration = 1000/noteDurations[thisNote];
    if (sound) tone(PIN_SPEAKER, melody[thisNote],noteDuration);
    if (melody[thisNote] > 0) {
      rotateAnimation();
    }
    pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PIN_SPEAKER);
    if (melody[thisNote] > 0) {
      stopButtonLeds();
    }
  }
}

void player0Wins() {
  byte melody[] = { 250, 196, 196, 220, 196,0, 247, 250};

  byte noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4 };

  int noteDuration = 0;
  int pauseBetweenNotes = 0;

 for (byte thisNote = 0; thisNote < sizeof(melody)/sizeof(byte); thisNote++) {
    noteDuration = 1000/noteDurations[thisNote];
    if (sound) tone(PIN_SPEAKER, melody[thisNote],noteDuration);
    if (melody[thisNote] > 0) {
      rotateAnimation();
    }
    pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    noTone(PIN_SPEAKER);
    if (melody[thisNote] > 0) {
      stopButtonLeds();
    }
  }
}

void setBallSpeed(short position) {
  int delta = position * ((INITIAL_BALL_SPEED - 100) / position);
  ballSpeed = INITIAL_BALL_SPEED - delta;
}

void endGame(bool zeroOrOne) {
  FastLED.clear(); 
  FastLED.show();
  stopButtonLeds();
  if (zeroOrOne)  {
    player1Wins();
  } else {
    player0Wins();
  }
  ballSpeed = INITIAL_BALL_SPEED;
  gameState = IDLE;
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
        if (sound) tone(PIN_SPEAKER, 100, 100);

        leds[0] = CRGB::Red;
        FastLED.show();

      } else if (isButtonPressed(1)) {
        gameState = KICK_1_0;

        buttonLedOn(1);
        ballPosition = 0;
        lastTimeBallPosition = timer;
        if (sound) tone(PIN_SPEAKER, 200, 100);

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
          endGame(true);
        }
      }

      if (isButtonPressed(1)) { //opponent responds
        if (ballPosition <= 4) { //too early. 0 wins
          endGame(false);
        } else {
          ballPosition = NUM_LEDS - ballPosition;
          setBallSpeed(ballPosition);
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
          endGame(false);
        }
      }

      if (isButtonPressed(0)) { //opponent responds
        if (ballPosition <= 4) { //too early. 1 wins
          endGame(true);
        } else {
          ballPosition = NUM_LEDS - ballPosition;
          setBallSpeed(ballPosition);
          gameState = KICK_0_1;
        }
      }

    break;

    default:
    
    break;
  }
 
}