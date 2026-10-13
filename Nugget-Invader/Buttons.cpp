#include "Buttons.h"

const unsigned long Buttons::debounceDelay; // definition of static const

// press codes for the Vega 2 layout
#define PRESS_NONE   0
#define PRESS_UP     1
#define PRESS_DOWN   2
#define PRESS_A      3 // lt slot -> A button
#define PRESS_B      4 // rt slot -> B button

Buttons::Buttons(int8_t upBtnPin, int8_t dnBtnPin, int8_t ltBtnPin, int8_t rtBtnPin) {
  this->upBtnPin = upBtnPin;
  this->dnBtnPin = dnBtnPin;
  this->ltBtnPin = ltBtnPin;
  this->rtBtnPin = rtBtnPin;

  btnPins[0] = upBtnPin;
  btnPins[1] = dnBtnPin;
  btnPins[2] = ltBtnPin;
  btnPins[3] = rtBtnPin;
  
  init();
}

// Read all four buttons with proper per-button debouncing.
// Returns a one-shot press code (1..4) when a button transitions
// from released to pressed; returns -1 otherwise so callers can
// ignore spurious repeats while a button is held down.
int8_t Buttons::getPress() {
  int8_t pressCode = PRESS_NONE;
  int releasedCount = 0;

  for (int i = 0; i < 4; i++) {
    int rawState = digitalRead(btnPins[i]);
    if (rawState == HIGH) releasedCount++;

    // if the raw reading changed, restart the debounce timer
    if (rawState != lastRawState[i]) {
      lastDebounceTime[i] = millis();
      lastRawState[i] = rawState;
    }

    // once the reading has been stable for debounceDelay ms, accept it
    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (rawState != stableState[i]) {
        stableState[i] = rawState;
        // buttons wired to GND with INPUT_PULLUP: LOW == pressed
        if (stableState[i] == LOW) {
          pressCode = i + 1; // 1=UP, 2=DOWN, 3=A, 4=B
        }
      }
    }
  }

  if (pressCode != PRESS_NONE) {
    pressStatus = pressCode;
  } else if (releasedCount >= 2) {
    // all buttons have settled back to released -> clear latched press so
    // the same button can fire again after a new press+release cycle
    pressStatus = -1;
  }

  return pressStatus;
}

// wrapper functions to check if individual buttons pressed
bool Buttons::upPressed() { return(pressStatus==PRESS_UP); }
bool Buttons::dnPressed() { return(pressStatus==PRESS_DOWN); }
bool Buttons::ltPressed() { return(pressStatus==PRESS_A); } // Vega 2: A
bool Buttons::rtPressed() { return(pressStatus==PRESS_B); } // Vega 2: B

void Buttons::init() {
  pinMode(upBtnPin, INPUT_PULLUP);
  pinMode(dnBtnPin, INPUT_PULLUP);
  pinMode(ltBtnPin, INPUT_PULLUP);
  pinMode(rtBtnPin, INPUT_PULLUP);
}
