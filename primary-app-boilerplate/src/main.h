#ifndef _MAIN_H
#define _MAIN_H

#include <Arduino.h>
// _TASK_STATUS_REQUEST should be defined but it's causing core panics.
// I was able to get the desired functionality with no issues
// by manually commenting out the #ifndef _TASK_STATUS_REQUEST in
// the TaskScheduler library (not good) and it seems to work. The issue would
// appear to be something to do with the compiler getting mixed
// messages about what _TASK_STATUS_REQUEST is.
#define _TASK_STATUS_REQUEST
#include <TaskScheduler.h>
#include <stdint.h>
#include <analogWrite.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <ArduinoLog.h>
#include <common-network.h>
// #include <TaskScheduler.h>
#include "taskHelpers.h"
#include "task_button.h"
#include "task_prefs.h"
#include "task_network.h"
#include "esp_ota_ops.h"

/**
 *  Prototypes
 */

/** General */
void initTasks();
void restartInFactory();

/** Button */
void tBtnInit();
void tBtnCb();
void onBtnClick();
void onBtnDblClick();
void onBtnHold();

/** Preferences (in place of EEPROM) */
void tPrefsInit();

/** Network */
void tNetInit();
void setWiFiTasksToWait();
void onStationStatusUpdate(NET_JOB_STATUS jobStatus);
void onAccessPointStatusUpdate(NET_JOB_STATUS jobStatus);
void onOtaStatusUpdate(NET_JOB_STATUS jobStatus);
void onNetStatusUpdateCb(NET_JOB jobType, NET_JOB_STATUS netStatus);

/** Boilerplate Tasks */
void tTask2Init();
void tTask1Cb();
void tTask2Cb();

/**
 *  Variables / Constants
 */
bool _reboot = false;

/**
 * Log Levels for 'lg.<...>' calls throughout the system.
 * @note For production, turn off logging completely by
 * uncommenting line 31 (definition of DISABLE_LOGGING) in
 * ArduinoLog.h
*/
// #define LOG_LVL LOG_LEVEL_SILENT
// #define LOG_LVL LOG_LEVEL_FATAL
// #define LOG_LVL LOG_LEVEL_ERROR
// #define LOG_LVL LOG_LEVEL_WARNING
// #define LOG_LVL LOG_LEVEL_NOTICE
// #define LOG_LVL LOG_LEVEL_TRACE
#define LOG_LVL LOG_LEVEL_VERBOSE

/** Task Schedulers **/
Scheduler     _sched;
StatusRequest _srWiFi;
StatusRequest _srPrefsReady;
StatusRequest _srWiFiStation;
StatusRequest _srWiFiAccessPoint;

/** Tasks **/
Task _tPrefs   (NULL, &_sched);
Task _tBtn     (NULL, &_sched);
Task _tNet     (NULL, &_sched);
Task _tTask1   (NULL, &_sched);
Task _tTask2   (NULL, &_sched);

// #if(EZ_DEBUG_ON)
//   Task _tDbgLog   (NULL, &_sched);
// #endif

#endif