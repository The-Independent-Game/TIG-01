#include <Wire.h>
#include <FastLED.h>
#include "Notes.h"

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
    {2, (byte)0b11111110}, //P1 - Y
    {3, (byte)0b11111101}  //P2 - G
};

byte buttonPressStates;
byte buttonReadyStates;
gameStates gameState;
short ballPosition;
unsigned long timer;
unsigned long lastTimeBallPosition;
unsigned long animationTime;
int ballSpeed;
byte animationButton;
bool sound = true;
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A

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

void allOn() {
  Wire.beginTransmission(0x20);
  Wire.write((byte)0b11111100);
  Wire.endTransmission();
}

void stopBall() {
  FastLED.clear(); 
  FastLED.show();
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

  stopBall();

  ballSpeed = INITIAL_BALL_SPEED;

  Serial.println("setup OK");
}

void rotateAnimation() {
  if (++animationButton > 1) animationButton = 0;
  buttonLedOn(animationButton);
}

void player1Wins() {
  int melody[] = {
  // Asa branca - Luiz Gonzaga
  // Score available at https://musescore.com/user/190926/scores/181370

    NOTE_G4,8, NOTE_A4,8, NOTE_B4,4, NOTE_D5,4, NOTE_D5,4, NOTE_B4,4, 
    NOTE_C5,4, NOTE_C5,2, NOTE_G4,8, NOTE_A4,8,
    NOTE_B4,4, NOTE_D5,4, NOTE_D5,4, NOTE_C5,4
  };
    /*,

    NOTE_B4,2, REST,8, NOTE_G4,8, NOTE_G4,8, NOTE_A4,8,
    NOTE_B4,4, NOTE_D5,4, REST,8, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8,
    NOTE_G4,4, NOTE_C5,4, REST,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,

    NOTE_A4,4, NOTE_B4,4, REST,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
    NOTE_G4,2, REST,8, NOTE_G4,8, NOTE_G4,8, NOTE_A4,8,
    NOTE_B4,4, NOTE_D5,4, REST,8, NOTE_D5,8, NOTE_C5,8, NOTE_B4,8,

    NOTE_G4,4, NOTE_C5,4, REST,8, NOTE_C5,8, NOTE_B4,8, NOTE_A4,8,
    NOTE_A4,4, NOTE_B4,4, REST,8, NOTE_B4,8, NOTE_A4,8, NOTE_G4,8,
    NOTE_G4,4, NOTE_F5,8, NOTE_D5,8, NOTE_E5,8, NOTE_C5,8, NOTE_D5,8, NOTE_B4,8,

    NOTE_C5,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_G4,8, NOTE_E4,8, NOTE_G4,8,
    NOTE_G4,4, NOTE_F5,8, NOTE_D5,8, NOTE_E5,8, NOTE_C5,8, NOTE_D5,8, NOTE_B4,8,
    NOTE_C5,8, NOTE_A4,8, NOTE_B4,8, NOTE_G4,8, NOTE_A4,8, NOTE_G4,8, NOTE_E4,8, NOTE_G4,8,
    NOTE_G4,-2, REST,4
  
  };
  */
  int notes = sizeof(melody) / sizeof(melody[0]) / 2;
  playerWins(melody, notes);
}

void playerWins(int melody[], int notes) {
  int tempo = 120;

  int wholenote = (60000 * 4) / tempo;

  int divider = 0, noteDuration = 0;

  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    rotateAnimation();

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    if (sound) tone(PIN_SPEAKER, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(PIN_SPEAKER);
  }

  stopButtonLeds();
}

void player0Wins() {
  int melody[] = {

    // Baby Elephant Walk
    // Score available at https://musescore.com/user/7965776/scores/1862611

    
    NOTE_C4,-8, NOTE_E4,16, NOTE_G4,8, NOTE_C5,8, NOTE_E5,8, NOTE_D5,8, NOTE_C5,8, NOTE_A4,8,
    NOTE_FS4,8, NOTE_G4,8, REST,4, REST,2,
    NOTE_C4,-8, NOTE_E4,16, NOTE_G4,8, NOTE_C5,8, NOTE_E5,8, NOTE_D5,8, NOTE_C5,8, NOTE_A4,8,
    NOTE_G4,-2, NOTE_A4,8, NOTE_DS4,1
    }; /*,
    
    NOTE_A4,8,
    NOTE_E4,8, NOTE_C4,8, REST,4, REST,2,
    NOTE_C4,-8, NOTE_E4,16, NOTE_G4,8, NOTE_C5,8, NOTE_E5,8, NOTE_D5,8, NOTE_C5,8, NOTE_A4,8,
    NOTE_FS4,8, NOTE_G4,8, REST,4, REST,4, REST,8, NOTE_G4,8,
    NOTE_D5,4, NOTE_D5,4, NOTE_B4,8, NOTE_G4,8, REST,8, NOTE_G4,8,
    
    NOTE_C5,4, NOTE_C5,4, NOTE_AS4,16, NOTE_C5,16, NOTE_AS4,16, NOTE_G4,16, NOTE_F4,8, NOTE_DS4,8,
    NOTE_FS4,4, NOTE_FS4,4, NOTE_F4,16, NOTE_G4,16, NOTE_F4,16, NOTE_DS4,16, NOTE_C4,8, NOTE_G4,8,
    NOTE_AS4,8, NOTE_C5,8, REST,4, REST,2,
  };*/

  int notes = sizeof(melody) / sizeof(melody[0]) / 2;
  playerWins(melody, notes);
}

void setBallSpeed(short position) {
  int delta = position * ((INITIAL_BALL_SPEED - 100) / position);
  ballSpeed = INITIAL_BALL_SPEED - delta;
}

void endGame(bool zeroOrOne) {
  stopBall();
  stopButtonLeds();
  if (zeroOrOne)  {
    player1Wins();
  } else {
    player0Wins();
  }
  ballSpeed = INITIAL_BALL_SPEED;
  gameState = IDLE;
}

bool areAllButtonPressed() {
  int count = 0;
  for (int i = 0; i < sizeof(buttons)/sizeof(button); i++) {
    if (digitalRead(buttons[i].pin)) count ++;
  }
  if (count > 0) Serial.println(count);
  return count == sizeof(buttons)/sizeof(button);
}

void ballMoveOn(gameStates direction) {
  ballPosition ++;
  lastTimeBallPosition = timer;

  if (ballPosition < NUM_LEDS) {
    FastLED.clear();
    if (direction == KICK_0_1) leds[ballPosition] = CRGB::Red;
    if (direction == KICK_1_0) leds[NUM_LEDS - 1 - ballPosition] = CRGB::Red;
    FastLED.show();
  } else {
    endGame(true);
  }
}

void loop() {
  readButtons();
  timer = millis();

  if (areAllButtonPressed()) {
    gameState = IDLE;
    stopBall();
    sound = !sound;
    tone(PIN_SPEAKER, 400, 500);
    allOn();
    delay(1000);
    stopButtonLeds();
    delay(1000);
    allOn();
    delay(1000);
    stopButtonLeds();
    buttonPressStates = 0; //reset buttons
    buttonReadyStates = 0;
  } else {
    switch (gameState) {
      case IDLE:
        if (random(0,400000) == 0) {
          allOn();
          if (sound) tone(PIN_SPEAKER, tones[random(0,sizeof(tones)/sizeof(int))]);
          delay(500);
          noTone(PIN_SPEAKER);
          stopButtonLeds();
        } else {  
          if (timer - animationTime >= 500) {
            rotateAnimation();
            animationTime = millis();
          }
        }
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
          ballMoveOn(KICK_0_1);
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
          ballMoveOn(KICK_1_0);
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
}