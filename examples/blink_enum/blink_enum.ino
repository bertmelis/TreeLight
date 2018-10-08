/* Copyright 2018 Bert Melis */


#include <Arduino.h>
#include <Ticker.h>
#include <TreeLight.h>

class Blinker {
 public:
  Blinker() :
    _pin(LED_BUILTIN),
    _on(false),
    _pace(500),
    _timer() {}
  ~Blinker() {
    _timer.detach();
    pinMode(_pin, INPUT);
  }
  void setup(uint8_t pin = LED_BUILTIN) {
    _pin = pin;
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, 1);
  }
  void start() {
    _timer.attach_ms(_pace, _tick, this);
    digitalWrite(_pin, 0);
  }
  void stop() {
    _timer.detach();
    digitalWrite(_pin, 1);
  }
  void setPace(uint32_t pace) {
    _pace = pace;
#if defined ARDUINO_ARCH_ESP8266
    if (_timer.active()) {
      this->start();
    }
#elif defined ARDUINO_ARCH_ESP32
      this->start();
#endif
  }

 private:
  static void _tick(Blinker* instance) {
    digitalWrite(instance->_pin, !digitalRead(instance->_pin));
  }
  uint8_t _pin;
  bool _on;
  uint32_t _pace;
  Ticker _timer;
} blinker;

// timer to update stats periodically
Ticker timer;
volatile bool updateStats = false;

// possible states of the node, only printable characterd for now!
const char* blinkerEnum[] = {
  "noBlink",
  "slowBlink",
  "fastBlink",
  "fasterBlink"
};
// calculate the number of possibilities
const size_t enumQty = sizeof(blinkerEnum) / sizeof(blinkerEnum[0]);

EnumNode blinkNode("blinker", true);

void setup() {
  Serial.begin(74880);

  pinMode(LED_BUILTIN, OUTPUT);

  // update the statistics every 60 seconds
  timer.attach(60, [](){
    updateStats = true;
  });

  blinker.setup();  // defaults to LED_BUILTIN

  // (lambda) function that handles the state of the led
  blinkNode.onMessage([](const char* value) {
    if (strcmp(value, blinkerEnum[0]) == 0) {
      // no blink: stop blinker, light led, confirm to node
      blinker.stop();
      digitalWrite(LED_BUILTIN, HIGH);
      blinkNode.setValue(blinkerEnum[0]);
    } else if (strcmp(value, blinkerEnum[1]) == 0) {
      // slow blink: set pace, start blinker, confirm to node
      blinker.setPace(750);
      blinker.start();
      blinkNode.setValue(blinkerEnum[1]);
    } else if (strcmp(value, blinkerEnum[2]) == 0) {
      // fast blink: set pace, start blinker, confirm to node
      blinker.setPace(100);
      blinker.start();
      blinkNode.setValue(blinkerEnum[2]);
    } else if (strcmp(value, blinkerEnum[3]) == 0) {
      // faster blink: set pace, start blinker, confirm to node
      blinker.setPace(50);
      blinker.start();
      blinkNode.setValue(blinkerEnum[3]);
    } else {
      TreeLight.printf("invalid enum received: %s\n", value);
    }
  });

  // pass possibilities to node along with number of possibilities
  blinkNode.setEnum(blinkerEnum, enumQty);

  // set led to off as starting point
  blinkNode.setValue(blinkerEnum[0]);

  // general setup of TreeLight
  TreeLight.setHostname("blinker");       // hostname, base mqtt topic
  TreeLight.setupWiFi("SSID", "PASS");    // WiFi credentials
  TreeLight.setupServer(80);              // port for webserver
  TreeLight.setupMqtt({192, 168, 1, 2});  // IP of Mqtt broker
  TreeLight.begin();                      // start!
}

void loop() {
  // don't forget to loop()!
  TreeLight.loop();
  if (updateStats) {
    updateStats = false;
    TreeLight.updateStats();
  }
}
