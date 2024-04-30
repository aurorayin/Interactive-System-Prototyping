/**
 * Adapted from the simple pong Flappy Bird game code from Makeability lab
 *  
 * This code requires Shape.hpp from the MakeabilityLab_Arduino_Library
 *  
 * By Jon E. Froehlich
 * @jonfroehlich
 * http://makeabilitylab.io
 *
 */

#include <Wire.h>
#include <Shape.hpp>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Entities.h"

#define SCREEN_WIDTH 128 // OLED _display width, in pixels
#define SCREEN_HEIGHT 64 // OLED _display height, in pixels

// Declaration for an SSD1306 _display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 _display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char STR_LOADSCREEN_CREATOR[] = "CSE 493F SPR 2024";
const char STR_LOADSCREEN_APP_NAME_LINE1[] = "Shout";
const char STR_LOADSCREEN_APP_NAME_LINE2[] = "Jump!";
const char STR_PRESS_TO_PLAY[] = "Press button to play";
const char STR_GAME_OVER[] = "Game Over!";

// Define I/O pins
const int SOUND_ANALOG_INPUT_PIN = A0;
const int SOUND_DIGITAL_INPUT_PIN = 7;
const int BUTTON_INPUT_PIN = 13;
const int TONE_OUTPUT_PIN = 5;
const int VIBROMOTOR_OUTPUT_PIN = 6;
const int LED1_PIN = 4;
const int LED2_PIN = 8;
const int LED3_PIN = 10;

// Vibromotor timing
unsigned long _vibroMotorStartTimeStamp = -1;

// Set constants
const int CEILING_COLLISION_TONE_FREQUENCY = 200;
const int PLAY_TONE_DURATION_MS = 200;
const int VIBROMOTOR_DURATION_MS = 200;

// for tracking fps
unsigned long _frameCount = 0;
float _fps = 0;
unsigned long _fpsStartTimeStamp = 0;

// status bar
const boolean _drawFrameCount = false; // change to show/hide frame count
const int DELAY_LOOP_MS = 5;
const int LOAD_SCREEN_SHOW_MS = 1000;

// variables
int _spriteIndex = 0;
int _xSprite = 5; 
int _ySprite = _display.height() - SPRITE_HEIGHT;

class Spike : public Circle {
  protected:
    bool _hasPassedBird = false;
  
  public:
    Spike(int x, int y, int radius) : Circle(x, y, radius) 
    {
    }

    bool getHasPassedBird() {
      return _hasPassedBird;
    }

    bool setHasPassedBird(bool hasPassedBird) {
      _hasPassedBird = hasPassedBird;
    }
};

const int BIRD_HEIGHT = 21;
const int BIRD_WIDTH = 20;
const int NUM_PIPES = 3;

const int MIN_PIPE_WIDTH = 2;
const int MAX_PIPE_WIDTH = 15; // in pixels
const int MIN_PIPE_X_SPACING_DISTANCE = 0; // in pixels
const int MAX_PIPE_X_SPACING_DISTANCE = 100; // in pixels

int _pipeSpeed = 2;
int _gravity = 2; // can't apply gravity every frame, apply every X time
int _points = 0;
unsigned long _gameOverTimestamp = 0;

int previousMillis = 0;

const int IGNORE_INPUT_AFTER_GAME_OVER_MS = 500; //ignores input for 500ms after game over
          
Spike _spikes[NUM_PIPES] = { Spike(0, 0, 0),
                             Spike(0, 0, 0),
                             Spike(0, 0, 0)
                           };

enum GameState {
  NEW_GAME,
  PLAYING,
  GAME_OVER,
};

GameState _gameState = NEW_GAME;

// This is necessary for the game to work on the ESP32
// See: 
//  - https://github.com/espressif/arduino-esp32/issues/1734
//  - https://github.com/Bodmer/TFT_eSPI/issues/189
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

void setup() {
  Serial.begin(9600);

  pinMode(BUTTON_INPUT_PIN, INPUT_PULLUP);
  pinMode(TONE_OUTPUT_PIN, OUTPUT);
  pinMode(VIBROMOTOR_OUTPUT_PIN, OUTPUT);
  pinMode(SOUND_DIGITAL_INPUT_PIN, INPUT);
  pinMode(SOUND_ANALOG_INPUT_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  // SSD1306_SWITCHCAPVCC = generate _display voltage from 3.3V internally
  if (!_display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // if analog input pin 5 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(A5));

  // Show load screen
  showLoadScreen();

  // Setup pipes
  initializeGameEntities();

  _fpsStartTimeStamp = millis();
}

void loop() {

  _display.clearDisplay();

  drawStatusBar();
  if (_gameState == NEW_GAME || _gameState == GAME_OVER) {
    nonGamePlayLoop();
  } else if (_gameState == PLAYING) {
    gamePlayLoop();
  }
  calcFrameRate();

  // Draw the display buffer to the screen
  _display.display();

  if (DELAY_LOOP_MS > 0) {
    delay(DELAY_LOOP_MS);
  }
}

void nonGamePlayLoop() {
  for (int i = 0; i < NUM_PIPES; i++) {
    _spikes[i].draw(_display);
  }

  int16_t x1, y1;
  uint16_t w, h;
  int buttonVal = digitalRead(BUTTON_INPUT_PIN);
  if (_gameState == NEW_GAME) {

    _display.getTextBounds(STR_PRESS_TO_PLAY, 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(_display.width() / 2 - w / 2, 15);
    _display.print(STR_PRESS_TO_PLAY);

    if (buttonVal == LOW) {
      _gameState = PLAYING;
    }
  } else if (_gameState == GAME_OVER) {
    _display.setTextSize(2);
    _display.getTextBounds(STR_GAME_OVER, 0, 0, &x1, &y1, &w, &h);
    int yText = 15;
    _display.setCursor(_display.width() / 2 - w / 2, yText);
    _display.print(STR_GAME_OVER);

    yText = yText + h + 2;
    _display.setTextSize(1);
    _display.getTextBounds(STR_PRESS_TO_PLAY, 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(_display.width() / 2 - w / 2, yText);
    _display.print(STR_PRESS_TO_PLAY);

    // We ignore input a bit after game over so that user can see end game screen
    // and not accidentally start a new game
    if (buttonVal == LOW && millis() - _gameOverTimestamp >= IGNORE_INPUT_AFTER_GAME_OVER_MS) {
      // if the current state is game over, need to reset
      initializeGameEntities();
      _gameState = PLAYING;
    }
  }

  _display.drawBitmap(_xSprite, _ySprite, epd_bitmap_dino_array[_spriteIndex], SPRITE_WIDTH, SPRITE_HEIGHT, WHITE);
}

void initializeGameEntities() {
  _points = 0;

  const int minStartXPipeLocation = _display.width() / 2;
  int lastPipeX = minStartXPipeLocation;
  for (int i = 0; i < NUM_PIPES; i++) {

    int pipeX = lastPipeX + random(MIN_PIPE_X_SPACING_DISTANCE, MAX_PIPE_X_SPACING_DISTANCE);
    int pipeWidth = random(MIN_PIPE_WIDTH, MAX_PIPE_WIDTH);

    _spikes[i].setLocation(pipeX, SCREEN_HEIGHT - pipeWidth/2);
    _spikes[i].setDimensions(pipeWidth, pipeWidth);

    lastPipeX = _spikes[i].getRight();
  }
}

void gamePlayLoop() {
  int buttonVal = digitalRead(BUTTON_INPUT_PIN);
  _ySprite += _gravity;

  int soundVal = analogRead(SOUND_ANALOG_INPUT_PIN);
  Serial.println(soundVal);
  
  // light up LEDs based on sound sensor value
  if (soundVal > 518) {
    _ySprite -= 8;
    digitalWrite(LED1_PIN, HIGH);
    previousMillis = millis();
  } 
  
  if (soundVal > 520) {
    digitalWrite(LED2_PIN, HIGH);
    previousMillis = millis();
  } 
  
  if (soundVal > 525) {
    digitalWrite(LED3_PIN, HIGH);
    previousMillis = millis();
  }
  
  if (millis() - previousMillis > 200) {
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
  }

  if (buttonVal == LOW) {
    _ySprite -= 4;
    tone(TONE_OUTPUT_PIN, CEILING_COLLISION_TONE_FREQUENCY, PLAY_TONE_DURATION_MS);
    
    // Vibrate motor when button hit
    digitalWrite(VIBROMOTOR_OUTPUT_PIN, HIGH);
    _vibroMotorStartTimeStamp = millis();
  }
  
  if (_ySprite < 0) {
    _ySprite = 0;
  } else if (_ySprite > _display.height() - SPRITE_HEIGHT) {
    _ySprite = _display.height() - SPRITE_HEIGHT;
  }

  // Check for vibromotor output
  if(_vibroMotorStartTimeStamp != -1){
    if(millis() - _vibroMotorStartTimeStamp > VIBROMOTOR_DURATION_MS){
      _vibroMotorStartTimeStamp = -1;
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
    }
  }

  // xMaxRight tracks the furthest right pixel of the furthest right pipe
  // which we will use to reposition pipes that go off the left part of screen
  int xMaxRight = 0;

  // Iterate through pipes and check for collisions and scoring
  for (int i = 0; i < NUM_PIPES; i++) {
    _spikes[i].setX(_spikes[i].getX() - _pipeSpeed);

    _spikes[i].draw(_display);

    Serial.println(_spikes[i].toString());

    // Check if the bird passed by the pipe
    if (_spikes[i].getRight() < _xSprite + SPRITE_WIDTH) {

      // If we're here, the bird has passed the pipe. Check to see
      // if we've marked it as passed yet. If not, then increment the score!
      if (_spikes[i].getHasPassedBird() == false) {
        _points++;
        _spikes[i].setHasPassedBird(true);
      }
    }

    // xMaxRight is used to track future placements of pipes once
    // they go off the left part of the screen
    if (xMaxRight < _spikes[i].getRight()) {
      xMaxRight = _spikes[i].getRight();
    }

    // Check for collisions and end of game
    if (_ySprite + SPRITE_HEIGHT > _spikes[i].getY() && 
        _ySprite < _spikes[i].getY() + _spikes[i].getRadius() && 
        _xSprite + SPRITE_WIDTH/2 > _spikes[i].getX() && 
        _xSprite < _spikes[i].getX() + _spikes[i].getRadius()) {
      _spikes[i].setDrawFill(true);
      _gameState = GAME_OVER;
      _gameOverTimestamp = millis();
      _vibroMotorStartTimeStamp = -1;
      digitalWrite(VIBROMOTOR_OUTPUT_PIN, LOW);
    } else {
      _spikes[i].setDrawFill(false);
    }
  }

  // Check for spikes that have gone off the screen to the left
  // and reset them to off the screen on the right
  xMaxRight = max(xMaxRight, _display.width());
  for (int i = 0; i < NUM_PIPES; i++) {
    if (_spikes[i].getRight() < 0) {
      int pipeX = xMaxRight + random(MIN_PIPE_X_SPACING_DISTANCE, MAX_PIPE_X_SPACING_DISTANCE);
      int pipeWidth = random(MIN_PIPE_WIDTH, MAX_PIPE_WIDTH); 

      _spikes[i].setLocation(pipeX, SCREEN_HEIGHT - pipeWidth/2);
      _spikes[i].setDimensions(pipeWidth, pipeWidth);
      _spikes[i].setHasPassedBird(false);

      xMaxRight = _spikes[i].getRight();
    }
  }

  _display.drawBitmap(_xSprite, _ySprite, epd_bitmap_dino_array[_spriteIndex], SPRITE_WIDTH, SPRITE_HEIGHT, WHITE);
  _spriteIndex++;
  if (_spriteIndex >= NUM_SPRITES) {
    _spriteIndex = 0;
  }
}


void showLoadScreen() {
  // Clear the buffer
  _display.clearDisplay();

  // Show load screen
  _display.setTextSize(1);
  _display.setTextColor(WHITE, BLACK);

  int16_t x1, y1;
  uint16_t w, h;
  _display.setTextSize(1);

  int yText = 10;
  _display.getTextBounds(STR_LOADSCREEN_CREATOR, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, yText);
  _display.print(STR_LOADSCREEN_CREATOR);

  _display.setTextSize(2);
  yText = yText + h + 1;
  _display.getTextBounds(STR_LOADSCREEN_APP_NAME_LINE1, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, yText);
  _display.print(STR_LOADSCREEN_APP_NAME_LINE1);

  yText = yText + h + 1;
  _display.getTextBounds(STR_LOADSCREEN_APP_NAME_LINE2, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(_display.width() / 2 - w / 2, yText);
  _display.print(STR_LOADSCREEN_APP_NAME_LINE2);

  _display.display();
  delay(LOAD_SCREEN_SHOW_MS);
  _display.clearDisplay();
  _display.setTextSize(1);

}

/**
 * Call this every frame to calculate frame rate
 */
void calcFrameRate() {
  unsigned long elapsedTime = millis() - _fpsStartTimeStamp;
  _frameCount++;
  if (elapsedTime > 1000) {
    _fps = _frameCount / (elapsedTime / 1000.0);
    _fpsStartTimeStamp = millis();
    _frameCount = 0;
  }
}

/**
 * Draws the status bar at top of screen with points and fps
 */
void drawStatusBar() {
  // Draw accumulated points
  int16_t x1, y1;
  uint16_t w, h;
  const char score[] = "SCORE: ";
  _display.setTextSize(1);

  _display.getTextBounds(score, 0, 0, &x1, &y1, &w, &h);
  _display.setCursor(0, 0);
  _display.print(score);
  _display.setCursor(w, y1);
  _display.print(_points);

  // Draw frame count
  if (_drawFrameCount) {
    int16_t x1, y1;
    uint16_t w, h;
    _display.getTextBounds("XX.XX fps", 0, 0, &x1, &y1, &w, &h);
    _display.setCursor(_display.width() - w, 0);
    _display.print(_fps);
    _display.print(" fps");
  }
}