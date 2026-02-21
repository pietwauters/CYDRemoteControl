#ifndef ESP32_SLEEP_BUTTON_H
#define ESP32_SLEEP_BUTTON_H

#include "ESP32Button.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include <map>
#include <vector>

/**
 * @class ESP32SleepButton
 * @brief Extends ESP32Button with inactivity-based deep sleep and GPIO wakeup.
 *
 * Tracks how long the button has been inactive. If no state change is detected
 * within the configured timeout, the ESP32 enters deep sleep. The same button
 * pin is configured as an EXT0 wakeup source, respecting the active-low logic
 * level of the base class.
 *
 * The sleep timer can also be reset externally by other subsystems (e.g. touch,
 * network activity) via resetSleepTimer().
 *
 * Example usage:
 * @code
 *   auto* btn = ESP32SleepButton::getInstance(0, true, 20, 30000);
 *   btn->begin();
 *   // In loop:
 *   btn->doUpdate();           // checks debounce and sleep timeout
 *   btn->resetSleepTimer();    // call from other subsystems to defer sleep
 * @endcode
 */
class ESP32SleepButton : public ESP32Button {
public:
  /**
   * @brief Gets (or creates) the singleton instance for a given GPIO pin.
   *
   * @param[in] pin             GPIO pin number.
   * @param[in] activeLow       If true, button is active-low (default: true).
   * @param[in] debounceTimeMs  Debounce time in milliseconds (default: 20 ms).
   * @param[in] sleepTimeoutMs  Inactivity timeout before deep sleep in
   *                            milliseconds (default: 30 000 ms).
   * @return Pointer to the ESP32SleepButton instance for this pin.
   */
  static ESP32SleepButton *getInstance(uint8_t pin, bool activeLow = true,
                                       uint16_t debounceTimeMs = 20,
                                       uint32_t sleepTimeoutMs = 30000);

  /**
   * @brief Initialises the button and seeds the inactivity timer.
   *
   * Always call this instead of (or in addition to) the base begin().
   * Seeding the timer here prevents a false sleep trigger when setup() takes
   * longer than the configured timeout.
   */
  void begin();

  /**
   * @brief Sets the inactivity timeout before the device enters deep sleep.
   * @param[in] timeoutMs Timeout in milliseconds.
   */
  void setSleepTimeout(uint32_t timeoutMs);

  /**
   * @brief Returns the current inactivity timeout in milliseconds.
   * @return Timeout in milliseconds.
   */
  uint32_t getSleepTimeout() const;

  /**
   * @brief Resets the inactivity timer.
   *
   * Call this from any subsystem (touch events, network activity, LVGL
   * callbacks, etc.) to prevent premature sleep.
   */
  void resetSleepTimer();

  /**
   * @brief Returns the number of milliseconds remaining before deep sleep.
   *
   * Useful for driving a "sleep indicator" UI element.
   *
   * @return Remaining time in milliseconds, or 0 if already timed out.
   */
  uint32_t remainingMs() const;

  /**
   * @brief Overrides doUpdate() to add inactivity tracking.
   *
   * Calls the base class debounce logic first. If a state change is detected
   * the sleep timer is reset automatically. When the inactivity timeout
   * expires, #configureSleepWakeup() is called and the device enters deep
   * sleep.
   *
   * Must be called frequently from the main loop.
   */
  void doUpdate() override;

  /**
   * @brief Requests deep sleep as soon as the button is released.
   *
   * If the button is currently pressed, sleep is deferred until the next
   * release so the wakeup pin is not already asserted when the chip sleeps
   * (which would cause an immediate spurious wakeup). If the button is
   * already released, sleep is entered immediately.
   *
   * Identical to the automatic sleep path: all registered hold pins are
   * driven to their configured levels before the chip sleeps.
   */
  void forceSleep();

  /**
   * @brief Returns true if sleep has been requested and is waiting for the
   *        button to be released before entering deep sleep.
   */
  bool isSleepPending() const { return sleepPending; }

  /**
   *        there during deep sleep.
   *
   * Core method — use the convenience wrappers below when the level is known
   * at compile time. Can be called multiple times for different pins.
   * Uses gpio_hold_en() + gpio_deep_sleep_hold_en() internally.
   *
   * @param[in] gpioPin GPIO number to hold.
   * @param[in] level   Output level to hold: 0 = LOW, 1 = HIGH.
   */
  void holdDuringSleep(gpio_num_t gpioPin, int level);

  /**
   * @brief Convenience wrapper — holds @p gpioPin LOW during deep sleep.
   * @param[in] gpioPin GPIO number to hold LOW.
   */
  void holdLowDuringSleep(gpio_num_t gpioPin);

  /**
   * @brief Convenience wrapper — holds @p gpioPin HIGH during deep sleep.
   * @param[in] gpioPin GPIO number to hold HIGH.
   */
  void holdHighDuringSleep(gpio_num_t gpioPin);

private:
  ESP32SleepButton(uint8_t pin, bool activeLow, uint16_t debounceTimeMs,
                   uint32_t sleepTimeoutMs);

  /**
   * @brief Configures the GPIO pin as EXT0 wakeup source and enters deep
   *        sleep.
   *
   * The wakeup trigger level is derived from the active-low setting:
   *   - activeLow == true  → wake on LOW  (button press pulls pin low)
   *   - activeLow == false → wake on HIGH (button press pulls pin high)
   */
  void enterDeepSleep();

  static std::map<uint8_t, ESP32SleepButton *> instances;

  uint32_t sleepTimeoutMs; ///< Inactivity timeout before deep sleep [ms].
  uint32_t lastActivityMs; ///< millis() timestamp of last activity.
  bool sleepPending;       ///< Sleep deferred until button is released.

  /// Pins to drive to a fixed level and hold during deep sleep.
  struct HoldPin {
    gpio_num_t pin;
    int level; ///< 0 = LOW, 1 = HIGH
  };
  std::vector<HoldPin> holdPins;
};

#endif // ESP32_SLEEP_BUTTON_H
