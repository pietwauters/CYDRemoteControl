#include "battery_monitor.h"
#include <Arduino.h>

// ---------------------------------------------------------------------------
// LiPo single-cell discharge curve (resting voltage, no load)
// Source: typical empirical measurements; adjust to your cell if needed.
//
// Column 0: voltage in mV
// Column 1: state-of-charge in %
//
// The curve is non-linear:
//   - Top region  (4.0–4.2 V): voltage drops steeply for the last ~20 %
//   - Mid region  (3.6–3.9 V): very flat – most of the usable energy is here
//   - Low region  (3.0–3.5 V): voltage drops steeply again near empty
//
// Between table entries the value is linearly interpolated.
// Below 3000 mV → 0 %, above 4200 mV → 100 %.
// ---------------------------------------------------------------------------
struct VoltagePoint {
  uint16_t mv;
  uint8_t pct;
};

static const VoltagePoint kLipoTable[] = {
    {3000, 0},  {3200, 5},  {3300, 10}, {3400, 20}, {3500, 30},  {3600, 40},
    {3700, 50}, {3750, 55}, {3800, 60}, {3850, 68}, {3900, 75},  {3950, 80},
    {4000, 85}, {4050, 90}, {4100, 95}, {4150, 98}, {4200, 100},
};
static const uint8_t kTableSize = sizeof(kLipoTable) / sizeof(kLipoTable[0]);

// ---------------------------------------------------------------------------

BatteryMonitor::BatteryMonitor(uint8_t pin, uint8_t samples, float divider)
    : _pin(pin), _samples(samples), _divider(divider), _intervalMs(10000),
      _lastTime(0), _prevMv(0), _lastMv(0), _lastPct(0), _charging(false) {}

void BatteryMonitor::begin(uint32_t intervalMs) {
  _intervalMs = intervalMs;
  analogSetAttenuation(ADC_11db); // full-scale ~3.3 V input range
  pinMode(_pin, INPUT);
  // Take initial baseline measurement
  measure();
  _prevMv = _lastMv; // baseline: no trend yet
}

uint32_t BatteryMonitor::readMillivolts() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < _samples; i++) {
    sum += analogReadMilliVolts(_pin);
  }
  return (uint32_t)((sum / _samples) * _divider);
}

uint8_t BatteryMonitor::readPercent() {
  return millivoltsToPercent(readMillivolts());
}

const char *BatteryMonitor::measure() {
  uint32_t mv = readMillivolts();

  // Charging: voltage consistently rising by more than 20 mV
  _charging = (_prevMv > 0) && (mv > _prevMv + 20);
  _prevMv = _lastMv; // shift: previous becomes the one before last
  _lastMv = mv;
  _lastPct = millivoltsToPercent(mv);
  _lastTime = millis();

  return getSymbol();
}

const char *BatteryMonitor::update() {
  if (millis() - _lastTime < _intervalMs)
    return nullptr;
  return measure();
}

const char *BatteryMonitor::getSymbol() const {
  if (_charging)
    return LV_SYMBOL_CHARGE;
  if (_lastPct > 75)
    return LV_SYMBOL_BATTERY_FULL;
  if (_lastPct > 50)
    return LV_SYMBOL_BATTERY_3;
  if (_lastPct > 25)
    return LV_SYMBOL_BATTERY_2;
  if (_lastPct > 10)
    return LV_SYMBOL_BATTERY_1;
  return LV_SYMBOL_BATTERY_EMPTY;
}

uint8_t BatteryMonitor::millivoltsToPercent(uint32_t mv) {
  if (mv <= kLipoTable[0].mv)
    return 0;
  if (mv >= kLipoTable[kTableSize - 1].mv)
    return 100;

  // Find surrounding entries and interpolate
  for (uint8_t i = 1; i < kTableSize; i++) {
    if (mv <= kLipoTable[i].mv) {
      const VoltagePoint &lo = kLipoTable[i - 1];
      const VoltagePoint &hi = kLipoTable[i];
      // Linear interpolation
      uint32_t range_mv = hi.mv - lo.mv;
      uint32_t range_pct = hi.pct - lo.pct;
      uint32_t offset = mv - lo.mv;
      return (uint8_t)(lo.pct + (offset * range_pct) / range_mv);
    }
  }
  return 100;
}
