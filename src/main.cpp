#include <Arduino.h>
#include <SPI.h>

// include the installed LVGL- Light and Versatile Graphics Library -
// https://github.com/lvgl/lvgl
#include <lvgl.h>

// include the installed "TFT_eSPI" library by Bodmer to interface with the TFT
// Display - https://github.com/Bodmer/TFT_eSPI
#include <TFT_eSPI.h>

// include the installed the "XPT2046_Touchscreen" library by Paul Stoffregen to
// use the Touchscreen - https://github.com/PaulStoffregen/XPT2046_Touchscreen
#include <XPT2046_Touchscreen.h>

#include "ui/ui.h"
#include "wifi_udp.h"

// Create TFT and touchscreen instances
TFT_eSPI tft = TFT_eSPI();

// XPT2046 touch pins (your existing settings)
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

SPIClass touchscreenSPI = SPIClass(VSPI);
XPT2046_Touchscreen touchscreen(XPT2046_CS, XPT2046_IRQ);

// Screen management for WiFi connection handling
static lv_obj_t *lastActiveScreen =
    NULL; // Store last active screen before No_Connection_Screen
static bool wasConnected = false; // Track previous connection state

// Declare custom omega font
LV_FONT_DECLARE(omega_font_14);

// LVGL draw buffer - 1/10 screen (optimal for ESP32 RAM constraints)
#define DRAW_BUF_PIXELS (320 * 240 / 10)
static lv_color_t draw_buf_1[DRAW_BUF_PIXELS];
static lv_disp_draw_buf_t disp_draw_buf;

// Calibration constants from your run
const int TS_MIN_X = 521;
const int TS_MAX_X = 3540;
const int TS_MIN_Y = 379;
const int TS_MAX_Y = 3624;

// Current TFT rotation (0=portrait, 1=landscape, 2=portrait_inverted,
// 3=landscape_inverted)
static int tft_rotation = 0;

// LVGL v8 display flush: push pixels to TFT_eSPI
static void my_disp_flush(lv_disp_drv_t *drv, const lv_area_t *area,
                          lv_color_t *color_p) {
  int32_t w = (area->x2 - area->x1 + 1);
  int32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors((uint16_t *)color_p, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(drv);
}

// Touch read callback using the calibrated mapping
static void touchscreen_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  (void)drv;
  // Only check touch if IRQ is triggered - avoids constant polling
  if (touchscreen.tirqTouched() && touchscreen.touched()) {
    TS_Point p = touchscreen.getPoint();

    int screen_w = tft.width();
    int screen_h = tft.height();

    int x, y;

    // Map touch coordinates based on display rotation
    if (tft_rotation == 0) {
      // Portrait mode (240x320)
      x = map(p.x, TS_MIN_X, TS_MAX_X, 0, screen_w - 1);
      y = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, screen_h - 1);
    } else if (tft_rotation == 1) {
      // Landscape mode (320x240)
      x = map(p.y, TS_MIN_Y, TS_MAX_Y, 0, screen_w - 1);
      y = map(p.x, TS_MIN_X, TS_MAX_X, 0, screen_h - 1);
      y = (screen_h - 1) - y;
    } else if (tft_rotation == 2) {
      // Portrait inverted mode (240x320)
      x = map(p.x, TS_MIN_X, TS_MAX_X, screen_w - 1, 0);
      y = map(p.y, TS_MIN_Y, TS_MAX_Y, screen_h - 1, 0);
    } else { // tft_rotation == 3
      // Landscape inverted mode (320x240)
      x = map(p.y, TS_MIN_Y, TS_MAX_Y, screen_w - 1, 0);
      y = map(p.x, TS_MIN_X, TS_MAX_X, screen_h - 1, 0);
      y = (screen_h - 1) - y;
    }

    // Clamp safety
    x = constrain(x, 0, screen_w - 1);
    y = constrain(y, 0, screen_h - 1);

    data->state = LV_INDEV_STATE_PRESSED;
    data->point.x = x;
    data->point.y = y;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}
int PisteNr = 1;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize LVGL on Core 1 (default core for Arduino setup/loop)
  lv_init();

  // Initialize TFT and force landscape that matches SquareLine export
  tft.init();
  tft.setRotation(tft_rotation); // forced to 0
  tft.setSwapBytes(true);        // adjust if colors are wrong

  // Init touchscreen
  touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touchscreen.begin(touchscreenSPI);
  // Keep touchscreen.setRotation(0) so we get raw values and map them ourselves
  touchscreen.setRotation(0);

  // Initialize LVGL draw buffer (v8 API)
  lv_disp_draw_buf_init(&disp_draw_buf, draw_buf_1, NULL, DRAW_BUF_PIXELS);

  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = tft.width();
  disp_drv.ver_res = tft.height();
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &disp_draw_buf;
  lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

  // Register input device (v8 API)
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = touchscreen_read;
  lv_indev_drv_register(&indev_drv);

  // Initialize the SquareLine UI (ui_init() from generated files)
  ui_init();

  // Initialize WiFi connection
  initWiFi();

  // Load appropriate screen based on WiFi state
  if (wifiConnected) {
    _ui_screen_change(&ui_Central_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                      ui_Central_Screen_screen_init);
    lastActiveScreen = ui_Central_Screen;
    wasConnected = true;
  } else {
    _ui_screen_change(&ui_No_Connection_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                      ui_No_Connection_Screen_screen_init);
    wasConnected = false;
  }
}

void loop() {

  // Handle screen switching based on WiFi state
  if (wifiConnected && !wasConnected) {
    // WiFi reconnected - restore last active screen (or Central if none)
    if (lastActiveScreen == ui_Central_Screen) {
      _ui_screen_change(&ui_Central_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                        ui_Central_Screen_screen_init);
    } else if (lastActiveScreen == ui_Basic_Settings_Screen) {
      _ui_screen_change(&ui_Basic_Settings_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                        ui_Basic_Settings_Screen_screen_init);
    } else if (lastActiveScreen == ui_Cards_Screen) {
      _ui_screen_change(&ui_Cards_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                        ui_Cards_Screen_screen_init);
    } else {
      // Default to Central Screen if no valid last screen
      _ui_screen_change(&ui_Central_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                        ui_Central_Screen_screen_init);
      lastActiveScreen = ui_Central_Screen;
    }
    wasConnected = true;
  } else if (!wifiConnected && wasConnected) {
    // WiFi disconnected - switch to No Connection screen
    lv_obj_t *currentScreen = lv_scr_act();
    if (currentScreen != ui_No_Connection_Screen) {
      lastActiveScreen = currentScreen;
      _ui_screen_change(&ui_No_Connection_Screen, LV_SCR_LOAD_ANIM_NONE, 0, 0,
                        ui_No_Connection_Screen_screen_init);
    }
    wasConnected = false;
  }

  // Update lastActiveScreen when user navigates while connected
  if (wifiConnected) {
    lv_obj_t *currentScreen = lv_scr_act();
    if (currentScreen != ui_No_Connection_Screen) {
      lastActiveScreen = currentScreen;
    }
  }

  lv_task_handler(); // let the GUI do its work
  lv_tick_inc(5);    // tell LVGL how much time has passed
  delay(5);          // let this time pass
}