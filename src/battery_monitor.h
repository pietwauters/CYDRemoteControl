#pragma once
#include <lvgl.h>
#include <stdint.h>

// Reads LiPo battery voltage from GPIO34 via a /2 resistor divider.
// Charge percentage is estimated using a lookup table that models the
// non-linear voltage-to-capacity curve of a single-cell LiPo (3.0 V – 4.2 V).
//
// Typical usage:
//   BatteryMonitor battery;          // construct (GPIO34, 16 samples, ÷2)
//   battery.begin(10000);            // init + set 10-second update interval
//   // in loop():
//   const char *sym = battery.update();  // returns LV_SYMBOL_* or nullptr if
//                                        // interval not yet elapsed
//   if (sym) lv_label_set_text(ui_LabelBatLevel, sym);

class BatteryMonitor {
public:
  // pin      – ADC input pin (default GPIO34)
  // samples  – ADC readings averaged per measurement (default 16)
  // divider  – resistor-divider ratio: battV = adcV * divider (default 2.0)
  BatteryMonitor(uint8_t pin = 34, uint8_t samples = 16, float divider = 2.0f);

  // Call once in setup().
  // intervalMs – how often update() will take a new measurement (default 10 s)
  // Takes an initial baseline measurement immediately.
  void begin(uint32_t intervalMs = 10000);

  // Call every loop iteration.
  // Returns the LVGL symbol string (LV_SYMBOL_BATTERY_* or LV_SYMBOL_CHARGE)
  // when a new measurement was taken, or nullptr if the interval hasn't
  // elapsed yet.
  const char *update();

  // Returns the LVGL symbol for the current cached state without measuring.
  const char *getSymbol() const;

  // Returns cached battery voltage in mV from the last measurement.
  uint32_t getMillivolts() const { return _lastMv; }

  // Returns cached state-of-charge [0–100] from the last measurement.
  uint8_t getPercent() const { return _lastPct; }

  // Returns true if the last measurement indicated charging.
  bool isCharging() const { return _charging; }

  // Force an immediate measurement regardless of the interval.
  // Returns the symbol string.
  const char *measure();

  // Direct ADC read helpers (always perform a fresh measurement).
  uint32_t readMillivolts();
  uint8_t readPercent();

  // Converts a millivolt value to percent using the LiPo discharge curve.
  static uint8_t millivoltsToPercent(uint32_t mv);

private:
  uint8_t _pin;
  uint8_t _samples;
  float _divider;
  uint32_t _intervalMs;
  uint32_t _lastTime; // millis() of last measurement
  uint32_t _prevMv;   // measurement before last (for trend detection)
  uint32_t _lastMv;   // most recent measurement
  uint8_t _lastPct;
  bool _charging;
};
