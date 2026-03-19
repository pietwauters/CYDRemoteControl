#include "ESP32SleepButton.h"
#include <Arduino.h>

std::map<uint8_t, ESP32SleepButton *> ESP32SleepButton::instances;

// ---------------------------------------------------------------------------
// Singleton factory
// ---------------------------------------------------------------------------

ESP32SleepButton *ESP32SleepButton::getInstance(uint8_t pin, bool activeLow,
                                                uint16_t debounceTimeMs,
                                                uint32_t sleepTimeoutMs) {
  if (instances.find(pin) == instances.end()) {
    instances[pin] =
        new ESP32SleepButton(pin, activeLow, debounceTimeMs, sleepTimeoutMs);
  }
  return instances[pin];
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ESP32SleepButton::ESP32SleepButton(uint8_t pin, bool activeLow,
                                   uint16_t debounceTimeMs,
                                   uint32_t sleepTimeoutMs)
    : ESP32Button(pin, activeLow, debounceTimeMs),
      sleepTimeoutMs(sleepTimeoutMs), lastActivityMs(0), sleepPending(false) {}

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------

void ESP32SleepButton::begin() {
  ESP32Button::begin();
  // Seed the timer so the countdown starts from after hardware init,
  // not from the moment of construction.
  lastActivityMs = millis();
}

// ---------------------------------------------------------------------------
// Timeout management
// ---------------------------------------------------------------------------

void ESP32SleepButton::setSleepTimeout(uint32_t timeoutMs) {
  sleepTimeoutMs = timeoutMs;
}

uint32_t ESP32SleepButton::getSleepTimeout() const { return sleepTimeoutMs; }

void ESP32SleepButton::resetSleepTimer() { lastActivityMs = millis(); }

uint32_t ESP32SleepButton::remainingMs() const {
  uint32_t elapsed = millis() - lastActivityMs;
  if (elapsed >= sleepTimeoutMs) {
    return 0;
  }
  return sleepTimeoutMs - elapsed;
}

// ---------------------------------------------------------------------------
// Update loop — called from main loop
// ---------------------------------------------------------------------------

void ESP32SleepButton::doUpdate() {
  // Snapshot the state before the debounce logic runs.
  bool prevState = currentState();

  // Run the base-class debounce logic.
  ESP32Button::doUpdate();

  // If the debounced state changed, reset the inactivity timer.
  // We compare states directly to avoid consuming the stateChangedFlag,
  // which external callers rely on via stateHasChanged().
  if (currentState() != prevState) {
    resetSleepTimer();

    // If sleep was deferred waiting for release, enter it now.
    if (sleepPending && isReleased()) {
      enterDeepSleep();
      // Does not return.
    }
  }

  // Check for inactivity timeout — defer if button is currently held.
  if (sleepTimeoutMs > 0 && (millis() - lastActivityMs) >= sleepTimeoutMs) {
    if (isPressed()) {
      sleepPending = true;
    } else {
      enterDeepSleep();
    }
    // enterDeepSleep() does not return; execution resumes from boot.
  }
}

// ---------------------------------------------------------------------------
// Force sleep
// ---------------------------------------------------------------------------

void ESP32SleepButton::forceSleep() {
  if (isPressed()) {
    // Defer until the button is released to avoid an immediate wakeup
    // (the wakeup pin would already be in the active state).
    sleepPending = true;
  } else {
    enterDeepSleep();
  }
}

// ---------------------------------------------------------------------------
// Deep sleep
// ---------------------------------------------------------------------------

void ESP32SleepButton::holdDuringSleep(gpio_num_t gpioPin, int level) {
  holdPins.push_back({gpioPin, level});
}

void ESP32SleepButton::holdLowDuringSleep(gpio_num_t gpioPin) {
  holdDuringSleep(gpioPin, 0);
}

void ESP32SleepButton::holdHighDuringSleep(gpio_num_t gpioPin) {
  holdDuringSleep(gpioPin, 1);
}

void ESP32SleepButton::enterDeepSleep() {
  // Drive each registered pin to its requested level and latch it.
  // gpio_hold_en() freezes the output level in the GPIO latch register.
  // gpio_deep_sleep_hold_en() extends that latch hold into deep sleep for
  // non-RTC digital IOs.
  for (const auto &hp : holdPins) {
    gpio_set_direction(hp.pin, GPIO_MODE_OUTPUT);
    gpio_set_level(hp.pin, hp.level);
    gpio_hold_en(hp.pin);
  }
  if (!holdPins.empty()) {
    gpio_deep_sleep_hold_en();
  }

  // Wake up when the button is pressed.
  // activeLow == true  → pin is pulled LOW on press  → wake level = 0
  // activeLow == false → pin is pulled HIGH on press → wake level = 1
  int wakeupLevel = activeLow ? 0 : 1;

  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(pin), wakeupLevel);
  printf("Going into deep sleep .....\n");
  printf("\n");
  esp_deep_sleep_start();
  // Execution resumes from the beginning after wakeup (full reboot).
}
