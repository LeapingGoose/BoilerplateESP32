#include "task_prefs.h"

namespace task_prefs {

  /**
   *  Private
   */

  namespace {

    /**
     *  Variables
     */

    prefs_t     _prefs;
    Preferences _prefMan;
    char        _tmpMacAddr[18]; // Used to pass back to caller as temporary storage.
    bool        _saveImmediate = true;
    bool        _loadedPrefs = false;

    /**
     *  Macros
     * 
     *  Some shortcut macros to make common read/save
     *  tasks easier for entries.
     */

    // Macro: Save preference entry immediately (if allowed)
    #define SAVE_ENTRY(SAVE_FN, ENTRY_NAME, VAL) \
      _prefs.ENTRY_NAME = VAL; \
      if (_saveImmediate == true) { \
        _prefMan.begin("grow-prefs", false); \
        _prefMan.SAVE_FN(#ENTRY_NAME, (uint8_t)_prefs.ENTRY_NAME); \
        _prefMan.end(); \
      }
    // End Macro: Save preference entry immediately (if allowed)

    // Macro: READ Preference Entry
    #define READ_AND_RETURN_ENTRY(ENTRY_NAME) \
      if (_loadedPrefs == false) { read(); } \
      return _prefs.ENTRY_NAME; \
    // END Macro: READ Preference Entry
  }

  /**
   *  Public
   */

  prefs_t* save(prefs_t* prefs) {
    lg.verbose(F("task_prefs::save(...)" CR));
    prefs_t* p = (prefs == nullptr ? &_prefs : prefs);
    _prefMan.begin("grow-prefs", false);

    _prefMan.putBytes("pairedMac",   &(p->pairedMac[0]), sizeof(p->pairedMac));
    _prefMan.putBool ("serverHide",  p->serverHide);
    _prefMan.putUInt ("lightsOnLvl", p->lightsOnLvl);

    /** ADD ENTRY LOCATION 4 **/

    _prefMan.end();
    return read(); // Return a read of our write.
  }

  prefs_t* read() {
    lg.verbose(F("task_prefs::read()" CR));
    _prefMan.begin("grow-prefs", true);

    _prefs.serverHide  = _prefMan.getBool("serverHide",  _prefs.serverHide);
    _prefs.lightsOnLvl = _prefMan.getUInt("lightsOnLvl", _prefs.lightsOnLvl);
    // Paired MAC
    size_t len = _prefMan.getBytesLength("pairedMac");
    if (len != 0) {
      _prefMan.getBytes("pairedMac", &(_prefs.pairedMac[0]), len);
    }

    /** ADD ENTRY LOCATION 4 **/

    _prefMan.end();
    _loadedPrefs = true;  
    return &_prefs;
  }

  void serialOut() {
    if (_loadedPrefs == false) { read(); }
    
    lg.notice(F(
      "Preferences\n" \
      "   --------------------------------\n" \
      "   serverHide... |%s| \n" \
      "   lightsOnLvl.. |%d| \n" \
      "   pairedMac.... |%x:%x:%x:%x:%x:%x|\n" \
      "   --------------------------------\n"),
      _prefs.serverHide ? "true" : "false",
      _prefs.lightsOnLvl,
      _prefs.pairedMac[0], _prefs.pairedMac[1], _prefs.pairedMac[2], _prefs.pairedMac[3], _prefs.pairedMac[4], _prefs.pairedMac[5]);

      /** ADD ENTRY LOCATION 5 **/
  }

  prefs_t* get() {
    return &_prefs;
  }

  void saveImmediate(bool immediate) {
    _saveImmediate = immediate;
  }

  bool saveImmediate() {
    return _saveImmediate;
  }

  /**
   *  Custom Preferences
   ***********************************************************************************************/

  /**
   * Server Hide
   */

  bool getServerHide() {
    READ_AND_RETURN_ENTRY(serverHide);
  }

  void setServerHide(bool hide) {
    SAVE_ENTRY(putBool, serverHide, hide);
  }

  /**
   * Paired Mac
   */

  void setPairedMac (const uint8_t macAddr[]) {
    memcpy(&(_prefs.pairedMac[0]), macAddr, 6);
    if (_saveImmediate == true) {
      _prefMan.begin("grow-prefs", false);
      _prefMan.putBytes("pairedMac", _prefs.pairedMac, 6);
      _prefMan.end();
    }
  }

  uint8_t* getPairedMac() {
    if (_loadedPrefs == false) { read(); }
    return &(_prefs.pairedMac[0]);
  }

  char* getPairedMacAsChar() {
    return macAddrToChar(_tmpMacAddr, _prefs.pairedMac);
  }

  bool hasPairedMac() {
    return compareMacAddr(getPairedMac(), emptyMac) == false;
  }

   /**
   * Lights On Level
   */

  uint16_t getLightsOnLvl() {
    READ_AND_RETURN_ENTRY(lightsOnLvl);
  }

  void setLightsOnLvl(uint16_t lvl) {
    SAVE_ENTRY(putUInt, lightsOnLvl, lvl);
  }

  /** ADD ENTRY LOCATION 6 **/

}