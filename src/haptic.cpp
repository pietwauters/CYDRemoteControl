#include "haptic.h"
#include <Arduino.h>
#include <Preferences.h>

static bool hapticEnabled = true;
static Preferences hapticPrefs;
#if HAPTIC_TYPE == MOTOR

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

#elif HAPTIC_TYPE == SPEAKER

const int speakerChannel = 7;
const int speakerPin = 26;
void initHaptic() {
  // Configure GPIO 26 as output
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  ledcSetup(speakerChannel, 2000, 8);
  ledcAttachPin(speakerPin, speakerChannel);

  // Load haptic settings from NVS
  hapticPrefs.begin("haptic", false);
  hapticEnabled = hapticPrefs.getBool("enabled", true);
  hapticPrefs.end();

  Serial.printf("Haptic feedback initialized: %s\n",
                hapticEnabled ? "enabled" : "disabled");
}

void beep(int freq, int duration) {
  ledcWriteTone(speakerChannel, freq);
  delay(duration);
  ledcWriteTone(speakerChannel, 0);
}
void buzz(int durationMs) {
  unsigned long start = millis();

  while (millis() - start < durationMs) {
    int f = random(140, 260);
    ledcWriteTone(speakerChannel, f);
    delay(6);
    ledcWriteTone(speakerChannel, 0);
    delay(3);
  }

  ledcWriteTone(speakerChannel, 0);
}
void uiClick() {
  ledcWriteTone(speakerChannel, 1800);
  delay(2);
  ledcWriteTone(speakerChannel, 0);
}
void keyClick() {
  ledcWriteTone(speakerChannel, 3200);
  delay(2);

  ledcWriteTone(speakerChannel, 2200);
  delay(2);

  ledcWriteTone(speakerChannel, 1200);
  delay(1);

  ledcWriteTone(speakerChannel, 0);
}
void keyClickTok() {
  ledcWriteTone(speakerChannel, 1800);
  delay(2);
  ledcWriteTone(speakerChannel, 900);
  delay(2);
  ledcWriteTone(speakerChannel, 0);
}

void vibrateShort() {
  if (!hapticEnabled) {
    return;
  }
  keyClick();
  // buzz(VIBRATION_SHORT_MS);
  /*beep(800, VIBRATION_SHORT_MS * 100 / 350);
  beep(1200, VIBRATION_SHORT_MS * 50 / 350);
  beep(600, VIBRATION_SHORT_MS * 200 / 350);*/
}

void vibrateLong() {
  if (!hapticEnabled) {
    return;
  }
  keyClickTok();
  // buzz(VIBRATION_LONG_MS);
  /*beep(800, VIBRATION_LONG_MS * 100 / 350);
  beep(1200, VIBRATION_LONG_MS * 50 / 350);
  beep(600, VIBRATION_LONG_MS * 200 / 350);*/
}
#else
#error "Unknown HAPTIC_TYPE"
#endif

void setHapticEnabled(bool enabled) {
  hapticEnabled = enabled;

  // Save to NVS
  hapticPrefs.begin("haptic", false);
  hapticPrefs.putBool("enabled", enabled);
  hapticPrefs.end();

  Serial.printf("Haptic feedback %s\n", enabled ? "enabled" : "disabled");
}

bool isHapticEnabled() { return hapticEnabled; }
