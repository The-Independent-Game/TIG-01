#include <Wire.h>
#include <FastLED.h>
#include "Notes.h"

#define PIN_SPEAKER 12
#define LED_PIN     5
#define NUM_LEDS    9
#define MAX_DELAY_VEL 300

#define MAX_LEVEL 15


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
int midBallPosition;
int tones[] = {261, 277, 294, 311, 330, 349, 370, 392, 415, 440};
//            mid C  C#   D    D#   E    F    F#   G    G#   A
int level;

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

  ballSpeed = MAX_DELAY_VEL;

  midBallPosition = NUM_LEDS / 2;

  level = 0;

  Serial.println("setup OK");
}

void rotateAnimation() {
  if (++animationButton > 1) animationButton = 0;
  buttonLedOn(animationButton);
}

void player1MusicWins() {
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
  playerMusicWins(melody, notes);
}

void playerMusicWins(int melody[], int notes) {
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

void player0MusicWins() {
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
  playerMusicWins(melody, notes);
}

void endGame(gameStates gs) {
  stopBall();
  stopButtonLeds();
  gs == KICK_0_1 ? buttonLedOn(0) : buttonLedOn(1);
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = gs == KICK_0_1 ? CRGB::Yellow : CRGB::Blue;
  }
  FastLED.show();
  gs == KICK_0_1 ? player0MusicWins() : player1MusicWins();
  stopBall();
  
  ballPosition = 0;
  ballSpeed = MAX_DELAY_VEL;
  gameState = IDLE;
  buttonPressStates = 0;
  buttonReadyStates = 0;
  level = 1;
}

bool areAllButtonPressed() {
  int count = 0;
  for (int i = 0; i < sizeof(buttons)/sizeof(button); i++) {
    if (digitalRead(buttons[i].pin)) count ++;
  }
  return count == sizeof(buttons)/sizeof(button);
}

void firstKick(gameStates direction) {
  gameState = direction;
  byte button;
  if (direction == KICK_0_1) button = 0;
  if (direction == KICK_1_0) button = 1;

  playerSound(direction);

  buttonLedOn(button);
  ballPosition = 0; //start
  lastTimeBallPosition = timer;

  level = 1;
  
  setBallLedColor(ballPosition, direction);
  FastLED.show();
}

void setBallLedColor(int position, gameStates direction) {
  if (position == midBallPosition) {
    leds[ballLedEncoding(ballPosition, direction)].setRGB(0, map(level, 1, MAX_LEVEL, 20, 255), 0); // =  CRGB::Green; 
  } else {
    leds[ballLedEncoding(ballPosition, direction)].setRGB(map(level, 1, MAX_LEVEL, 20, 255), 0, 0); // = CRGB::Red;
  }
}

int ballLedEncoding(int position, gameStates direction) {
  if (direction == KICK_0_1) return position;
  if (direction == KICK_1_0) return NUM_LEDS - 1 - position;
}

void ballMoveOn(gameStates direction) {
  if (timer - lastTimeBallPosition > ballSpeed) {
    ballPosition ++;
    lastTimeBallPosition = timer;

    if (ballPosition < NUM_LEDS) {
      FastLED.clear();
      setBallLedColor(ballPosition, direction);
      FastLED.show();
    } else {
      endGame(direction);
    }
  }
}

void playerSound(gameStates direction) {
  if (sound) {
    if (direction == KICK_0_1) {
      tone(PIN_SPEAKER, 400, 400);
    } else {
      tone(PIN_SPEAKER, 700, 400);
    }
  }
}

void opponentResponds(gameStates direction) {
  byte button;
  if (direction == KICK_0_1) button = 1;
  if (direction == KICK_1_0) button = 0;
  
  if (isButtonPressed(button)) { //opponent responds
    playerSound(direction);
    
    if (ballPosition <= midBallPosition) { //too early. other wins (4)
      endGame(direction);
    } else {
      level ++;
      if (level > MAX_LEVEL) {
        level = MAX_LEVEL;
      }
      int vel = ballPosition + level;
      int min = midBallPosition + 1 + 1;
      int max = NUM_LEDS - 1 + MAX_LEVEL;

      ballSpeed = map(ballPosition, midBallPosition + 1, NUM_LEDS - 1, 300, 100);
      ballSpeed -= map(level, 1, MAX_LEVEL, 0, 70);

      if (direction == KICK_0_1) gameState = KICK_1_0;
      if (direction == KICK_1_0) gameState = KICK_0_1;

      ballPosition = NUM_LEDS - ballPosition - 1;
    }
  }
}

void loop() {
  readButtons();
  timer = millis();

  if (areAllButtonPressed() && false) { //stop feature
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
          firstKick(KICK_0_1);
        } else if (isButtonPressed(1)) {
          firstKick(KICK_1_0);
        }
      break;

      case KICK_0_1:
        ballMoveOn(KICK_0_1);
        opponentResponds(KICK_0_1);
      break;

      case KICK_1_0:
        ballMoveOn(KICK_1_0);
        opponentResponds(KICK_1_0);
      break;

      default:
      break;
    }
  }
}