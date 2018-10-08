# TreeLight

[![Build Status](https://travis-ci.com/bertmelis/TreeLight.svg?branch=master)](https://travis-ci.com/bertmelis/TreeLight)

Don't loose any time connecting your ESP8266 to MQTT. Just include this lib and with minimal extra code, your device is up and running.
It comes with a handy webpage too so you can have a quick look at the status of your device.

# Installing & usage

I seriously recommend using Platformio. Check out [https://platformio.org/](https://platformio.org/).

You'll have to add `-DTEMPLATE_PLACEHOLDER=\'~\'` to the build flags for proper working of this lib.

A full platformio.ini could look like this:

```INI
[env:esp8266]
platform = espressif8266
board = d1_mini
framework = arduino
lib_deps =
  5495
  ESP Async WebServer
  ArduinoJson
  AsyncMqttClient
  https://github.com/bertmelis/TreeLight.git
build_flags =
  -Wl,-Tesp8266.flash.4m1m.ld
  -DTEMPLATE_PLACEHOLDER=\'~\'
monitor_speed = 74880
```

`-DTEMPLATE_PLACEHOLDER=\'~\'` is important as it changes the default '%' in EspAsyncWebserver as this interferes with CSS in the webpage.

Also, see the example for further guidance.
If you still can't figure out how to use, please create an issue.

# Disclaimer

I created this library for my own personal needs. As the MIT license states, it comes with no warranty or fit for purpose. If it doesn't fit your use case, feel free to fork/copy/adapt/change... I do like to have some credit and maybe you can share your improvements?

This library is still in developement. Next on the to do list are:
- ESP32 support --> didn't test, but compiles!
- ~enable settable values from webpage~
- ~enable settable values from MQTT~
- enable changing WiFi and MQTT parameters
