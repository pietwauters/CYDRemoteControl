#pragma once

#include <Arduino.h>

#define MOTOR 1
#define SPEAKER 2

// #define HAPTIC_TYPE MOTOR
#define HAPTIC_TYPE SPEAKER
#define HAPTIC_PIN 18 // GPIO 26 (GPIO 35 is input-only!)
#define VIBRATION_SHORT_MS 150
#define VIBRATION_LONG_MS 400

#ifdef __cplusplus
extern "C" {
#endif

// Initialize the haptic feedback system
extern void initHaptic();

// Trigger a short vibration pulse (for normal button clicks)
extern void vibrateShort();

// Trigger a longer vibration pulse (for long-press events)
extern void vibrateLong();

// Enable or disable haptic feedback
extern void setHapticEnabled(bool enabled);

// Check if haptic feedback is enabled
extern bool isHapticEnabled();

#ifdef __cplusplus
}
#endif
