/* TreeLight

Copyright 2018 Bert Melis

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

#ifndef USE_STATS
#define USE_STATS 1
#endif

#include <queue>

// Arduino framework
#if TL_DEBUG
#include <Arduino.h>
#endif
#if defined ARDUINO_ARCH_ESP32
#include <WiFi.h>
#include <Update.h>
#elif defined ARDUINO_ARCH_ESP8266
#include <ESP8266WiFi.h>
#include <Updater.h>
#else
#error Platform not supported
#endif
#include <Ticker.h>

// External
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

// Internal
#include "TreeLightNode.h"
#if USE_STATS
#include "Helpers/Uptime.h"
#endif

class TreeLightClass : public Print, public AsyncMqttClient {
  friend class TreeLightNode;

 private:
  TreeLightClass();
  TreeLightClass(const TreeLightClass &rhs);
  TreeLightClass& operator=(const TreeLightClass &rhs);

 public:
  inline static TreeLightClass& get(void) {
    static TreeLightClass instance;
    return instance;
  }

 public:
  void setHostname(const char* hostname);
  void setupWiFi(const char* ssid, const char* pass);
  void setupServer(uint16_t port = 80);
  void setupMqtt(const IPAddress broker, const uint16_t port = 1883);
  void begin();
  void loop();

 public:
#if USE_STATS
  void updateStats();
#endif

 private:
  static void _connectToWiFi(TreeLightClass* instance);
#if defined ARDUINO_ARCH_ESP32
  static void _onWiFiEvent(TreeLightClass* instance, WiFiEvent_t event);
#elif ARDUINO_ARCH_ESP8266
  void _onWiFiConnected(const WiFiEventStationModeConnected& event);
  void _onWiFiDisconnected(const WiFiEventStationModeDisconnected& event);
  WiFiEventHandler _wiFiConnectedHandler;
  WiFiEventHandler _wiFiDisconnectedHandler;
#endif
  static void _connectToMqtt(TreeLightClass* instance);
  void _onMqttConnected();
  void _onMqttDisconnected(AsyncMqttClientDisconnectReason reason);
  void _onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void _onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
  char _ssid[33];
  char _pass[65];
  char _hostname[33];
  Ticker _timer;
  AsyncWebServer* _webserver;
  AsyncWebSocket* _websocket;
  bool _flagForReboot;
#if USE_STATS
  void _updateStats(AsyncWebSocketClient* client = nullptr);
  Uptime _uptime;
#endif

 public:
  size_t write(uint8_t character);

 private:
  void _printBuffer();
  uint32_t _lastMessagesSend;
  std::queue<uint8_t> _messageBuffer;
};

extern TreeLightClass& TreeLight;
