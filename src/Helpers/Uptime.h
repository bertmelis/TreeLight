/* TreeLight

Copyright 2017 Bert Melis

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#if defined ARDUINO_ARCH_ESP32
#include <esp32-hal.h>  // for millis()
#elif defined ARDUINO_ARCH_ESP8266
#include <Arduino.h>
#include <ESP8266WiFi.h>  // for WiFi.RSSI()
#endif

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (SECS_PER_MIN * 60UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
#define numberOfHours(_time_) ((_time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define elapsedDays(_time_) (_time_ / SECS_PER_DAY)

class Uptime {
 public:
  Uptime() :
    _uptimeStr{'\0'},
    _uptime(0),
    _lastMillis(0) {}
  uint64_t getUptime() {  // Call at least every 50 days to avoid overflow ;-)
    _uptime += (millis() - _lastMillis);
    _lastMillis = millis();
    return _uptime;
  }
  char* getUptimeStr() {
    getUptime();
    uint64_t uptime = _uptime / 1000;
    uint8_t days = elapsedDays(uptime);
    uint8_t hours = numberOfHours(uptime);
    uint8_t minutes = numberOfMinutes(uptime);
    uint8_t seconds = numberOfSeconds(uptime);
    snprintf(_uptimeStr, sizeof(_uptimeStr), "%d Days %02d:%02d:%02d", days, hours, minutes, seconds);
    return _uptimeStr;
  }

 private:
  char _uptimeStr[18];
  uint64_t _uptime;
  uint32_t _lastMillis;
};
