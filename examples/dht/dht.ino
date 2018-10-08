#include <Arduino.h>

#include <DHT.h>
#include <TreeLight.h>

// DHT
DHT11 sensor;
volatile int result = 0;
float temperature = 0.0;
float humidity = 0.0;
Ticker timer1;

// TreeLight
FloatNode tempNode("temperature", false);
FloatNode humidNode("humidity", false);
Ticker timer2;
volatile bool updateStats = false;

void ICACHE_RAM_ATTR handleCallback(int8_t res) {
  result = res;
}

void setup() {
  Serial.begin(74880);
  
  // DHT
  sensor.setPin(D4);
  sensor.setCallback(handleCallback);
  timer1.attach(60, []() {  // update DHT values every 60s
    sensor.read();
  });
  
  // TreeLight
  timer2.attach(1, []() {  // update stats every 1s
    updateStats = true;
  });
  TreeLight.setHostname("AIRQ");
  TreeLight.setupWiFi("xxxx", "xxxx");
  TreeLight.setupServer(80);
  TreeLight.setupMqtt({192, 168, 1, 2});
  TreeLight.begin();
}

void loop() {
  TreeLight.loop();
  if (result > 0) {
    result = 0;
    tempNode.setValue(sensor.getTemperature());
    humidNode.setValue(sensor.getHumidity());
  } else if (result < 0) {
    result = 0;
    TreeLight.printf("Error: %s\n", sensor.getError());
  }
  if (updateStats) {
    updateStats = false;
    TreeLight.updateStats();
  }
}
