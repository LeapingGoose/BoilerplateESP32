#include "task_button.h"

namespace task_btn {

  /**
   *  Private
   */

  namespace {

    OneButton _btn(PIN_BTN, true);
    BUTTON_STATUS _status = BUTTON_STATUS::UNAVAILABLE;

  }

  /**
   *  Public
   */

  void setup(callbackFunction clickFuncPtr, callbackFunction dblClickFuncPtr, callbackFunction longPressFuncPtr) {
    lg.verbose(F("task_button::setup(...)" CR));
    _btn.attachClick(clickFuncPtr);
    _btn.attachDoubleClick(dblClickFuncPtr);
    _btn.attachLongPressStart(longPressFuncPtr);
    _btn.setDebounceTicks(40);
    _btn.setClickTicks(300);
    _btn.setPressTicks(LONG_PRESS_DUR_MS);
    _status = BUTTON_STATUS::READY;
  }

  OneButton* getButton() {
    return &_btn;
  }

  void loop() {
    _btn.tick();
  }

  void setStatus(BUTTON_STATUS status) {
    _status = status;
  }

  BUTTON_STATUS getStatus() {
    return _status;
  }

}