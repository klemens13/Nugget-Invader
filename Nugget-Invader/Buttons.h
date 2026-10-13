#pragma once

#include "Arduino.h"

// Buttons driver for the Hackheld Vega 2 (Spacehuhn) layout:
//
//        [ UP ]
//   [ A ]          [ B ]
//        [ DOWN ]
//
// Press codes returned by getPress():
//   1 = UP   2 = DOWN   3 = A (lt)   4 = B (rt)   -1 = no new press
class Buttons {
  
  public:
    Buttons(int8_t upBtnPin, int8_t dnBtnPin, int8_t ltBtnPin, int8_t rtBtnPin);
    void init();
    int8_t getPress();
    int8_t pressStatus;
    
    bool upPressed();
    bool dnPressed();
    bool ltPressed();
    bool rtPressed();

    // Vega 2 friendly aliases
    bool aPressed() { return pressStatus == 3; } // A button
    bool bPressed() { return pressStatus == 4; } // B button
    
  private:
    int8_t upBtnPin;
    int8_t dnBtnPin;
    int8_t ltBtnPin;
    int8_t rtBtnPin;

    // per-button debounce state
    int8_t btnPins[4];
    int lastRawState[4]   = {HIGH, HIGH, HIGH, HIGH};
    int stableState[4]    = {HIGH, HIGH, HIGH, HIGH};
    unsigned long lastDebounceTime[4] = {0, 0, 0, 0};
    static const unsigned long debounceDelay = 50; // ms
 };
