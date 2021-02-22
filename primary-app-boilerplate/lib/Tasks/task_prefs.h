#ifndef TASK_PREFS_H
#define TASK_PREFS_H

#include <stdint.h>
#include <Preferences.h>
#include <ArduinoLog.h>
#include "common-network.h"

namespace task_prefs {

  /**
   *  Support
   */
  typedef struct prefs_t {
    bool     serverHide = false;
    uint8_t  pairedMac[6] = { 0, 0, 0, 0, 0, 0 };
    uint16_t lightsOnLvl = 100;                       // Level of the LDR required to turn lights on.

    /** ADD ENTRY LOCATION 1 **/

  } prefs_t;

  /**
   *  Routines
   */
  prefs_t*  save(prefs_t* prefs = nullptr);
  prefs_t*  read();
  prefs_t*  get();
  void      serialOut();
  bool      SaveImmediate();
  void      SaveImmediate(bool immediate);

  /** Preference Values **/

  bool      getServerHide();                         // Server Hide
  void      setServerHide(bool hide);
  void      setPairedMac(const uint8_t macAddr[]);   // Paired Mac
  uint8_t*  getPairedMac();
  bool      hasPairedMac();
  char*     getPairedMacAsChar();
  uint16_t  getLightsOnLvl();                        // Lights On Level
  void      setLightsOnLvl(uint16_t lvl);

  /** ADD ENTRY LOCATION 2 **/

}

#endif