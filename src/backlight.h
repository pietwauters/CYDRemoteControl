#pragma once

#include <lvgl.h>

#define TFT_BL 21
#define BL_CH 0
#define BL_FREQ 2000 // 2 kHz (good for TFT)
#define BL_RES 8     // 0–255

// Default values (used if NVS not initialized)
#define DEFAULT_BRIGHTNESS_DEFAULT 100
#define IDLE_BRIGHTNESS_DEFAULT 10
#define BACKLIGHT_TIMEOUT_MS_DEFAULT 15000

#ifdef __cplusplus
extern "C" {
#endif

extern void setBrightness(__UINT8_TYPE__ value);
extern void initBacklight();
extern void updateBacklightTimer();
extern void resetBacklightTimer();
extern void onTouchEvent(lv_event_t *e);

// Setter functions for brightness and timeout settings
extern void setDefaultBrightness(uint8_t brightness);
extern void setIdleBrightness(uint8_t brightness);
extern void setBacklightTimeout(uint32_t timeoutMs);

// Getter functions
extern uint8_t getDefaultBrightness();
extern uint8_t getIdleBrightness();
extern uint32_t getBacklightTimeout();

#ifdef __cplusplus
}
#endif