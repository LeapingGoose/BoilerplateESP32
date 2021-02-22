#ifndef task_btn_H
#define task_btn_H

#include <OneButton.h>
#include <stdint.h>
#include <ArduinoLog.h>

namespace task_btn {

  /**
   *  Settings - Adjust as Required.
   */

  #define PIN_BTN           4 // Default button for ESP32 factory reset
  #define LONG_PRESS_DUR_MS 5000

  enum class BUTTON_STATUS:uint8_t {
    READY         = 1,
    UNAVAILABLE   = 2
  };

  /**
   *  Routines
   */
  void          setup(callbackFunction clickFuncPtr, callbackFunction dblClickFuncPtr, callbackFunction longPressFuncPtr);
  OneButton*    getButton();
  void          loop();
  void          setStatus(BUTTON_STATUS status);
  BUTTON_STATUS getStatus();

}

#endif