#ifndef TASK_HELPERS
#define TASK_HELPERS

#include <Arduino.h>
#include <stdint.h>
#include <ArduinoLog.h>

// void initTask(Task& task, unsigned long interval, long iterations, TaskCallback callback, TaskOnEnable onEnableCb, TaskOnDisable onDisableCb);
void initTask(Task& task, unsigned long interval, long iterations, TaskCallback callback, TaskOnEnable onEnableCb, TaskOnDisable onDisableCb, StatusRequest* trigger, bool triggerDelay = false);


// Shorthand helpers for initializing tasks
void initTask(Task& task, unsigned long interval, long iterations, TaskCallback callback, TaskOnEnable onEnableCb, TaskOnDisable onDisableCb) {
  task.setInterval(interval);
  task.setIterations(iterations);
  if (callback    != NULL) { task.setCallback(callback); }
  if (onEnableCb  != NULL) { task.setOnEnable(onEnableCb); }
  if (onDisableCb != NULL) { task.setOnDisable(onDisableCb); }
}

void initTask(Task& task, unsigned long interval, long iterations, TaskCallback callback, TaskOnEnable onEnableCb, TaskOnDisable onDisableCb, StatusRequest* trigger, bool triggerDelay) {
  if (callback    != NULL) { task.setCallback(callback); }
  if (onEnableCb  != NULL) { task.setOnEnable(onEnableCb); }
  if (onDisableCb != NULL) { task.setOnDisable(onDisableCb); }

  if (triggerDelay == true) {
    task.waitForDelayed(trigger, interval, iterations);
  } else {
    task.waitFor(trigger, interval, iterations);
  }
}

#endif