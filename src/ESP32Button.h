#ifndef ESP32_BUTTON_H
#define ESP32_BUTTON_H

#include "SubjectObserverTemplate.h"
#include <Arduino.h>
#include <map>

/**
 * @class ESP32Button
 * @brief A debounced button handler for ESP32 with singleton pattern.
 *
 * This class provides functionality to debounce a button and track its state
 * using a singleton pattern to ensure only one instance per GPIO pin.
 */
class ESP32Button : public Subject<ESP32Button> {
public:
  /**
   * @brief Gets an instance of the button for a given GPIO pin.
   * @param[in] pin GPIO pin number.
   * @param[in] activeLow If true, the button is active-low (default: true).
   * @param[in] debounceTimeMs Debounce time in milliseconds (default: 20ms).
   * @return Pointer to the ESP32Button instance.
   */
  static ESP32Button *getInstance(uint8_t pin, bool activeLow = true,
                                  uint16_t debounceTimeMs = 20);

  /**
   * @brief Initializes the button with appropriate pull-up or pull-down
   * configuration.
   */
  virtual void begin();

  /**
   * @brief Gets the current state of the button.
   * @return True if the button is HIGH, false otherwise.
   */
  bool currentState() const;

  /**
   * @brief Checks if the button state has changed since the last update and
   * resets the flag.
   * @return True if the state has changed, false otherwise.
   */
  bool stateHasChanged();

  /**
   * @brief Checks if the button is currently pressed.
   * @return True if the button is pressed, false otherwise.
   */
  bool isPressed() const;

  /**
   * @brief Checks if the button is currently released.
   * @return True if the button is released, false otherwise.
   */
  bool isReleased() const;

  /**
   * @brief Updates the button state and applies debounce logic.
   *
   * This function should be called frequently in the loop to ensure accurate
   * debouncing. It sets the stateChangedFlag when a state change is detected
   * and retains it until stateChanged() is called to check and reset the flag.
   */
  virtual void doUpdate();

  /**
   * @brief Sets a new debounce time for the button.
   * @param[in] timeMs New debounce time in milliseconds.
   */
  void setDebounceTime(uint16_t timeMs);

  /**
   * @brief Sets the duration a button must be held to trigger a long press.
   * @param[in] timeMs Duration in milliseconds (default: 500 ms).
   */
  void setLongPressTime(uint16_t timeMs);

  /**
   * @brief Returns the current long-press threshold in milliseconds.
   * @return Long-press duration in milliseconds.
   */
  uint16_t getLongPressTime() const;

  /**
   * @brief Checks if a long press has been detected since the last call and
   *        resets the flag.
   *
   * The flag is set once when the button has been continuously held for at
   * least the configured long-press duration. It is not re-fired until the
   * button is released and pressed again.
   *
   * @return True if a long press was detected, false otherwise.
   */
  bool isLongPress();

  void StateChanged(uint32_t eventtype) { notify(eventtype); }

protected:
  ESP32Button(uint8_t pin, bool activeLow, uint16_t debounceTimeMs);

  uint8_t pin;    ///< GPIO pin number.
  bool activeLow; ///< Indicates if the button is active-low.

private:
  static std::map<uint8_t, ESP32Button *> instances;
  uint16_t debounceTime;     ///< Debounce time in milliseconds.
  uint16_t longPressTime;    ///< Duration threshold for a long press [ms].
  bool state;                ///< Current button state.
  bool lastState;            ///< Previous button state.
  bool stateChangedFlag;     ///< Flag to indicate state change, retained until
                             ///< checked.
  bool longPressFlag;        ///< Set once when long-press threshold is reached;
                             ///< consumed by isLongPress().
  uint32_t lastDebounceTime; ///< Last debounce timestamp.
  uint32_t pressStartMs;     ///< millis() when the button was last pressed.
};

#endif // ESP32_BUTTON_H
