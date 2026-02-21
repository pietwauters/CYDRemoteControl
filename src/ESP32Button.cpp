#include "ESP32Button.h"

std::map<uint8_t, ESP32Button *> ESP32Button::instances;

ESP32Button *ESP32Button::getInstance(uint8_t pin, bool activeLow,
                                      uint16_t debounceTimeMs) {
  if (instances.find(pin) == instances.end()) {
    instances[pin] = new ESP32Button(pin, activeLow, debounceTimeMs);
  }
  return instances[pin];
}

ESP32Button::ESP32Button(uint8_t pin, bool activeLow, uint16_t debounceTimeMs)
    : pin(pin), activeLow(activeLow), debounceTime(debounceTimeMs),
      longPressTime(500), state(activeLow ? HIGH : LOW), lastState(state),
      stateChangedFlag(false), longPressFlag(false), lastDebounceTime(0),
      pressStartMs(0) {}

void ESP32Button::begin() {
  // NOTE: GPIO34-39 on ESP32 are input-only and do NOT support internal
  // pull-up/pull-down resistors. INPUT_PULLUP has no effect on those pins.
  // If using such a pin (e.g. GPIO35), add an external 10 kΩ pull-up to 3.3 V.
  pinMode(pin, activeLow ? INPUT_PULLUP : INPUT_PULLDOWN);
  // Seed state from the actual pin level so we don't fire a spurious event
  // if the pin is already at the "pressed" level when begin() is called.
  bool initialReading = digitalRead(pin);
  state = initialReading;
  lastState = initialReading;
  lastDebounceTime = millis(); // prevent immediate timeout on first update
  stateChangedFlag = false;
}

bool ESP32Button::currentState() const { return state; }

bool ESP32Button::stateHasChanged() {
  bool changed = stateChangedFlag;
  stateChangedFlag = false; // Clear the flag after checking
  return changed;
}

bool ESP32Button::isPressed() const { return activeLow ? !state : state; }

bool ESP32Button::isReleased() const { return activeLow ? state : !state; }

void ESP32Button::setDebounceTime(uint16_t timeMs) { debounceTime = timeMs; }

void ESP32Button::setLongPressTime(uint16_t timeMs) { longPressTime = timeMs; }

uint16_t ESP32Button::getLongPressTime() const { return longPressTime; }

bool ESP32Button::isLongPress() {
  bool lp = longPressFlag;
  longPressFlag = false;
  return lp;
}

void ESP32Button::doUpdate() {
  bool reading = digitalRead(pin);
  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceTime) {
    if (reading != state) {
      state = reading;
      stateChangedFlag = true;
      if (isPressed()) {
        // Record when the press started; reset the long-press flag
        // so it can fire again on this new press.
        pressStartMs = millis();
        longPressFlag = false;
      }
    }
  }

  // Fire the long-press flag once when the threshold is exceeded while
  // the button is still held. Cleared on the next press or when consumed.
  if (isPressed() && !longPressFlag &&
      (millis() - pressStartMs) >= longPressTime) {
    longPressFlag = true;
  }

  lastState = reading;
}
