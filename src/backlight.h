#pragma once

#include <lvgl.h>

#define TFT_BL 21
#define BL_CH 0
#define BL_FREQ 2000 // 2 kHz (good for TFT)
#define BL_RES 8     // 0–255

// Default brightness when active
#define DEFAULT_BRIGHTNESS 100

// Inactivity timeout in milliseconds (e.g., 30 seconds)
#define BACKLIGHT_TIMEOUT_MS 10000

extern void setBrightness(__UINT8_TYPE__ value);
extern void initBacklight();
extern void updateBacklightTimer();
extern void resetBacklightTimer();
extern void onTouchEvent(lv_event_t *e);