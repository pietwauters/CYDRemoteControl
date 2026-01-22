#include "backlight.h"
#include <Arduino.h>
#include <Preferences.h>

static unsigned long lastTouchTime = 0;
static bool backlightActive = true;

// Runtime values (loaded from NVS or defaults)
static uint8_t defaultBrightness = DEFAULT_BRIGHTNESS_DEFAULT;
static uint8_t idleBrightness = IDLE_BRIGHTNESS_DEFAULT;
static uint32_t backlightTimeoutMs = BACKLIGHT_TIMEOUT_MS_DEFAULT;

static Preferences backlightPrefs;

void setBrightness(__UINT8_TYPE__ value) {
  ledcSetup(BL_CH, BL_FREQ, BL_RES);
  ledcAttachPin(TFT_BL, BL_CH);
  ledcWrite(BL_CH, value);
}

void initBacklight() {
  // Load settings from NVS
  backlightPrefs.begin("backlight", false);

  // Read values from NVS, use defaults if not found
  defaultBrightness =
      backlightPrefs.getUChar("defBright", DEFAULT_BRIGHTNESS_DEFAULT);
  idleBrightness =
      backlightPrefs.getUChar("idleBright", IDLE_BRIGHTNESS_DEFAULT);
  backlightTimeoutMs =
      backlightPrefs.getUInt("timeout", BACKLIGHT_TIMEOUT_MS_DEFAULT);

  backlightPrefs.end();

  Serial.printf(
      "Backlight settings loaded: Default=%d, Idle=%d, Timeout=%dms\n",
      defaultBrightness, idleBrightness, backlightTimeoutMs);

  setBrightness(defaultBrightness);
  lastTouchTime = millis();
  backlightActive = true;
}

void updateBacklightTimer() {
  unsigned long currentTime = millis();

  // Check if timeout has elapsed
  if (backlightActive && (currentTime - lastTouchTime >= backlightTimeoutMs)) {
    // Turn off backlight after inactivity
    setBrightness(idleBrightness);
    backlightActive = false;
    Serial.println("Backlight off due to inactivity");
  }
}

void resetBacklightTimer() {
  lastTouchTime = millis();

  // If backlight was off, turn it back on
  if (!backlightActive) {
    setBrightness(defaultBrightness);
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

// Setter functions - save to NVS and apply immediately
void setDefaultBrightness(uint8_t brightness) {
  defaultBrightness = brightness;

  // Save to NVS
  backlightPrefs.begin("backlight", false);
  backlightPrefs.putUChar("defBright", brightness);
  backlightPrefs.end();

  // Apply immediately if backlight is active
  if (backlightActive) {
    setBrightness(defaultBrightness);
  }

  Serial.printf("Default brightness set to %d\n", brightness);
}

void setIdleBrightness(uint8_t brightness) {
  idleBrightness = brightness;

  // Save to NVS
  backlightPrefs.begin("backlight", false);
  backlightPrefs.putUChar("idleBright", brightness);
  backlightPrefs.end();

  // Apply immediately if backlight is idle
  if (!backlightActive) {
    setBrightness(idleBrightness);
  }

  Serial.printf("Idle brightness set to %d\n", brightness);
}

void setBacklightTimeout(uint32_t timeoutMs) {
  backlightTimeoutMs = timeoutMs;

  // Save to NVS
  backlightPrefs.begin("backlight", false);
  backlightPrefs.putUInt("timeout", timeoutMs);
  backlightPrefs.end();

  Serial.printf("Backlight timeout set to %dms\n", timeoutMs);
}

// Getter functions
uint8_t getDefaultBrightness() { return defaultBrightness; }

uint8_t getIdleBrightness() { return idleBrightness; }

uint32_t getBacklightTimeout() { return backlightTimeoutMs; }