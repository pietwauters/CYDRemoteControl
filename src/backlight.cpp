#include "backlight.h"
#include <Arduino.h>

static unsigned long lastTouchTime = 0;
static bool backlightActive = true;

void setBrightness(__UINT8_TYPE__ value) {
  ledcSetup(BL_CH, BL_FREQ, BL_RES);
  ledcAttachPin(TFT_BL, BL_CH);
  ledcWrite(BL_CH, value);
}

void initBacklight() {
  setBrightness(DEFAULT_BRIGHTNESS);
  lastTouchTime = millis();
  backlightActive = true;
}

void updateBacklightTimer() {
  unsigned long currentTime = millis();

  // Check if timeout has elapsed
  if (backlightActive &&
      (currentTime - lastTouchTime >= BACKLIGHT_TIMEOUT_MS)) {
    // Turn off backlight after inactivity
    setBrightness(0);
    backlightActive = false;
    Serial.println("Backlight off due to inactivity");
  }
}

void resetBacklightTimer() {
  lastTouchTime = millis();

  // If backlight was off, turn it back on
  if (!backlightActive) {
    setBrightness(DEFAULT_BRIGHTNESS);
    backlightActive = true;
    Serial.println("Backlight restored on touch");
  }
}

// Global touch event callback - resets backlight on any touch
void onTouchEvent(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);

  if (code == LV_EVENT_PRESSED) {
    resetBacklightTimer();
  }
}