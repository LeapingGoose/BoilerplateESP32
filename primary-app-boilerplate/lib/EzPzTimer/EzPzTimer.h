#ifndef EZPZTIMER_H
#define EZPZTIMER_H

/*
 * A timer implementation for the Arduino which is safe from the
 * effects of rollover. See
 * http://arduino.cc/playground/Code/TimingRollover for details. This
 * class conveniently encapsulates that behaviour.
 */

/* This is a modified version by Troy Gaylord */

#include <Arduino.h>

class EzPzTimer {

  public:
    enum TIMING_UNITS {
      MICROS,
      MILLIS
    };

    // Have default delay to be zero so that timer is already expired
    inline EzPzTimer(void) : delay(0), unit(MILLIS) {
      expires = millis() + delay;
    }
    
    inline EzPzTimer(unsigned long d, TIMING_UNITS u)
      : delay(d), unit(u) {
      if (unit == MICROS)
        expires = micros() + delay;
      else
        expires = millis() + delay;
    }
    
    inline bool isExpired(void) const {
      if (unit == MICROS)
        return (long(micros() - expires) >= 0);
      else
        return (long(millis() - expires) >= 0);
    }

    inline bool isExpired(void) const volatile {
      if (unit == MICROS)
        return (long(micros() - expires) >= 0);
      else
        return (long(millis() - expires) >= 0);
    }
    
    inline void restart() {
      expire();
      start(delay, unit);
    }
    inline void restart() volatile {
      expire();
      start(delay, unit);
    }
    inline void start(unsigned long d) {
      start(d, TIMING_UNITS::MILLIS);
    }
    inline void start(unsigned long d) volatile {
      start(d, TIMING_UNITS::MILLIS);
    }
    inline void start(unsigned long d, TIMING_UNITS u) {
      delay = d;
      unit = u;
      if (unit == MICROS)
        expires = micros() + delay;
      else
        expires = millis() + delay;
    }

    inline void start(unsigned long d, TIMING_UNITS u) volatile {
      delay = d;
      unit = u;
      if (unit == MICROS)
        expires = micros() + delay;
      else
        expires = millis() + delay;
    }

    // Restart the delay from when it expired (not now)
    inline void repeat(void) {
      expires += delay;
    }

    inline void repeat(void) volatile {
      expires += delay;
    }

    // Force a delay to be expired
    inline void expire(void) {
      if (unit == MICROS)
        expires = micros();
      else
        expires = millis();   
    }

    inline void expire(void) volatile {
      if (unit == MICROS)
        expires = micros();
      else
        expires = millis();   
    }

    inline unsigned long getDelay() const {
      return delay;
    }

    inline unsigned long elapsed() const {
      if (unit == MICROS)
        return micros() - (expires - delay);
      else
        return millis() - (expires - delay);
    }

    inline uint8_t percentComplete() const {
      unsigned long elapsedTime = elapsed();
      return elapsedTime == 0 ? 0 : elapsedTime * 100 / delay;
    }

    inline unsigned long getDelay() const volatile {
      return delay;
    }
    
    inline unsigned long getExpiry(void) const {
      return expires;
    }

    inline unsigned long getExpiry(void) const volatile {
      return expires;
    }

  private:
    unsigned long delay;
    unsigned long expires;
    TIMING_UNITS unit;

};

#endif