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

#ifndef TL_DEBUG
#define TL_DEBUG 1
#endif

#include <vector>
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
#include "Helpers/Uptime.h"

enum TreeLightNodeType {
  SWITCH,
  NUMBER
};

class TreeLightNode {
  friend class TreeLightClass;
 public:
  TreeLightNode(const char* name, bool settable, TreeLightNodeType type);
  ~TreeLightNode();
  const char* getType() const;
  const char* name;
  char value[16];
  bool settable;
  const TreeLightNodeType type;
 private:
  static std::vector<TreeLightNode*> _nodes;
};

class TreeLightClass : public Print, public AsyncMqttClient {
 public:
  TreeLightClass();
  void setHostname(const char* hostname);
  void setupWiFi(const char* ssid, const char* pass);
  void setupServer(uint16_t port = 80);  // defaults to 80
  void setupMqtt(const IPAddress broker, const uint16_t port = 1883);
  void begin();
  void loop();

 public:
  TreeLightNode* findNode(const char* name);
  void setNode(TreeLightNode& node, const char* value);  // NOLINT
  void updateStats();

 private:
  static void _connectToWiFi(TreeLightClass* instance);
  void _onWiFiConnected(const WiFiEventStationModeConnected& event);
  void _onWiFiDisconnected(const WiFiEventStationModeDisconnected& event);
  static void _connectToMqtt(TreeLightClass* instance);
  void _onMqttConnected();
  void _onMqttDisconnected(AsyncMqttClientDisconnectReason reason);
  WiFiEventHandler _wiFiConnectedHandler;
  WiFiEventHandler _wiFiDisconnectedHandler;
  char _ssid[33];
  char _pass[18];
  char _hostname[15];
  Ticker _timer;
  Uptime _uptime;

 private:
  AsyncWebServer* _webserver;
  AsyncWebSocket* _websocket;
  bool _flagForReboot;
  void _wsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
  void _mqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  void _updateStats(AsyncWebSocketClient* client = nullptr);

 public:
  size_t write(uint8_t character);

 private:
  void _printBuffer();
  uint32_t _lastMessagesSend;
  std::queue<uint8_t> _messageBuffer;
};

extern TreeLightClass TreeLight;
