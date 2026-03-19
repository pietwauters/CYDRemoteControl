#include "haptic.h"
#include <Arduino.h>
#include <Preferences.h>

static bool hapticEnabled = true;
static Preferences hapticPrefs;

void initHaptic() {
  // Configure GPIO 35 as output
  pinMode(HAPTIC_PIN, OUTPUT);
  digitalWrite(HAPTIC_PIN, LOW);

  // Load haptic settings from NVS
  hapticPrefs.begin("haptic", false);
  hapticEnabled = hapticPrefs.getBool("enabled", true);
  hapticPrefs.end();

  Serial.printf("Haptic feedback initialized: %s\n",
                hapticEnabled ? "enabled" : "disabled");
}

void vibrateShort() {
  if (!hapticEnabled) {
    return;
  }

  digitalWrite(HAPTIC_PIN, HIGH);
  delay(VIBRATION_SHORT_MS);
  digitalWrite(HAPTIC_PIN, LOW);
}

void vibrateLong() {
  if (!hapticEnabled) {
    return;
  }

  digitalWrite(HAPTIC_PIN, HIGH);
  delay(VIBRATION_LONG_MS);
  digitalWrite(HAPTIC_PIN, LOW);
}

void setHapticEnabled(bool enabled) {
  hapticEnabled = enabled;

  // Save to NVS
  hapticPrefs.begin("haptic", false);
  hapticPrefs.putBool("enabled", enabled);
  hapticPrefs.end();

  Serial.printf("Haptic feedback %s\n", enabled ? "enabled" : "disabled");
}

bool isHapticEnabled() { return hapticEnabled; }
