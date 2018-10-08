/* Copyright 2018 Bert Melis */


#include <Arduino.h>
#include <Ticker.h>
#include <TreeLight.h>

class Blinker {
 public:
  explicit Blinker() :
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

BoolNode toggle("led", true);  // node to handle switching of blinker, settable
IntNode pace("blinkPace", true);  // node to hande blinking pace, settable

void setup() {
  Serial.begin(74880);

  // update the statistics every 60 seconds
  timer.attach(60, [](){
    updateStats = true;
  });

  blinker.setup();  // defaults to LED_BUILTIN

  // (lambda) function that handles the state of the led
  toggle.onMessage([](bool value) {
    if (value) {
      blinker.start();
      TreeLight.print("blinker started\n");
      toggle.setValue(true);  // confirm new state
    } else {
      blinker.stop();
      TreeLight.print("blinker stopped\n");
      toggle.setValue(false);  // confirm new state
    }
  });
  // set led to off as starting point
  toggle.setValue(false);

  // set limits to the led blinking pace (min, step, max)
  pace.setRange(100, 100, 1000);

  // (lambda) function that handles the blinking pace of the led
  pace.onMessage([](uint32_t value) {
    if (value < 100) value = 100;  // TreeLight doesn't check the range or step itself!
    if (value > 1000) value = 1000;
    blinker.setPace(value);
    pace.setValue(value);  // confirm new pace
    TreeLight.printf("set pace to %u\n", value);
  });

  // set led blinking pace to 500ms
  pace.setValue(500);

  // general setup of TreeLight
  TreeLight.setHostname("blinker");       // hostname, base mqtt topic
  TreeLight.setupWiFi("ssid", "pass");    // WiFi credentials
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
