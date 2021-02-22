#include "main.h"

void setup() {

#ifndef DISABLE_LOGGING
  delay(1000);
  Serial.begin(115200);
  while (!Serial);
  delay(1000);
#endif

  lg.begin(LOG_LVL, &Serial);
  lg.notice(F("Starting Sensor Unit 2...\n"));
  initTasks();
}

void initTasks() {
  _srWiFi            .setWaiting();
  _srWiFiStation     .setWaiting();
  _srWiFiAccessPoint .setWaiting();
  /**      Task       Interval (MS)    Iterations    Callback       onEnable onDisable Trigger         Trigger Delay */
  initTask(_tPrefs,   TASK_IMMEDIATE,  TASK_ONCE,    &tPrefsInit,   NULL,    NULL);
  initTask(_tNet,     TASK_IMMEDIATE,  TASK_ONCE,    &tNetInit,     NULL,    NULL,     &_srPrefsReady);
  initTask(_tBtn,     TASK_IMMEDIATE,  TASK_FOREVER, &tBtnInit,     NULL,    NULL,     &_srPrefsReady);
  initTask(_tTask1,   TASK_SECOND,     TASK_FOREVER, &tTask1Cb,     NULL,    NULL,     &_srPrefsReady);
  initTask(_tTask2,   TASK_SECOND,     TASK_FOREVER, &tTask2Init,   NULL,    NULL,     &_srPrefsReady);
  _tPrefs.enable();
}

void loop() {
  if (_reboot == true) {
    lg.warning(F("\n\n\n! Rebooting...\n\n\n"));
    delay(3000);
    ESP.restart();
    return;
  } 
  _sched.execute();
}

/**
 *  Task: Preferences (like EEPROM)`
 */

void tPrefsInit() {
  lg.verbose(F("tPrefsInit()\n"));
  task_prefs::serialOut();
  _srPrefsReady.signalComplete();
}

/**
 *  Task: Network
 */

void tNetInit() {
  lg.verbose(F("tNetInit()\n"));
  task_net::setup(onNetStatusUpdateCb, false);
}

void onNetStatusUpdateCb(NET_JOB jobType, NET_JOB_STATUS jobStatus) {
  using namespace task_net;
  lg.verbose(F("main::onNetStatusUpdateCb(...)\n"));

  switch (jobType)
  {
  case NET_JOB::STATION:      onStationStatusUpdate(jobStatus);     break;
  case NET_JOB::ACCESS_POINT: onAccessPointStatusUpdate(jobStatus); break;
  case NET_JOB::OTA:          onOtaStatusUpdate(jobStatus);         break;
  case NET_JOB::REBOOT:       _reboot = true;                       break;
  case NET_JOB::UNKNOWN:
  default:                                                          break;
  }
}

void onStationStatusUpdate(NET_JOB_STATUS jobStatus) {
  lg.verbose(F("main::onStationStatusUpdate(...)\n"));
  /** @todo Not sure if all these wifi completions are accurate */
  if (jobStatus == NET_JOB_STATUS::START) {
    _srWiFi.signalComplete();

  } else if (jobStatus == NET_JOB_STATUS::CONNECTED) {
    lg.notice(F("STA CONNECTED\n"));
    _srWiFiStation.signalComplete();
    _srWiFi.signalComplete();

  } else {
    lg.notice(F("STA DISCONNECTED\n"));
    _srWiFiStation.signalComplete(-1);
    _srWiFi.signalComplete(-1);
  }
}

void onAccessPointStatusUpdate(NET_JOB_STATUS jobStatus) {
  lg.verbose(F("main::onAccessPointStatusUpdate(...)\n"));

  if (jobStatus == NET_JOB_STATUS::CONNECTED) {
    lg.notice(F("AP CONNECTED\n"));
    _srWiFiAccessPoint.signalComplete();
    _srWiFi.signalComplete();

  } else {
    lg.notice(F("AP DISCONNECTED\n"));
    _srWiFiAccessPoint.signalComplete(-1);
    _srWiFi.signalComplete(-1);
  }
}

void onOtaStatusUpdate(NET_JOB_STATUS jobStatus) {
  lg.verbose(F("main::onOtaStatusUpdate(...)\n"));
}

/**
 *  Task: Button
 */

void tBtnInit() {
  lg.verbose(F("tBtnInit()\n"));
  task_btn::setup(onBtnClick, onBtnDblClick, onBtnHold);
  _tBtn.setCallback(tBtnCb);
}

void tBtnCb() {
  task_btn::loop();
}

void onBtnClick() {
  lg.verbose(F("onBtnClick() <NO ACTION>\n"));
}

void onBtnDblClick() {
  lg.verbose(F("onBtnDblClick()\n"));  
}

void onBtnHold() {
  lg.verbose(F("onBtnHold()\n"));
  restartInFactory();
}

/** Restarts the ESP32 using the Factory parition **/
void restartInFactory() {
   esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

  if (pi != NULL) {
    const esp_partition_t* factory = esp_partition_get(pi);
    esp_partition_iterator_release(pi);
    if (esp_ota_set_boot_partition(factory) == ESP_OK) {
      lg.trace("Restarting with Factory app...");
      delay(2000);
      esp_restart();
    }
  }
  lg.warning("Factory partition not found."); 
}

/**
 *  Task: 1
 */

void tTask1Cb() {
}

/**
 *  Task: 2
 */

void tTask2Init() {
  _tTask2.setCallback(tTask2Cb);
  _tTask2.forceNextIteration();
}

void tTask2Cb() {
}
