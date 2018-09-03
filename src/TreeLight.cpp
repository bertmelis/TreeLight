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

#include "TreeLight.h"
#include "Helpers/Helpers.h"
#include "html.h"

std::vector<TreeLightNode*> TreeLightNode::_nodes;

TreeLightNode::TreeLightNode(const char* name, bool settable, TreeLightNodeType type) :
  name(name),
  value{""},
  settable(settable),
  type(type) {
    _nodes.push_back(this);
}

TreeLightNode::~TreeLightNode() {
  std::pair<bool, int> result = findInVector(&_nodes, this);
  _nodes.erase(_nodes.begin() + result.second);
}

const char* TreeLightNode::getType() const {
  switch (type) {
    case BOOL:
      return "BOOL";
    case NUMBER:
      return "NUMBER";
  }
  return {"\0"};
}

TreeLightClass TreeLight;

TreeLightClass::TreeLightClass() :
  _ssid{"\0"},
  _pass{"\0"},
  _hostname{"\0"},
  _timer(),
  _webserver(nullptr),
  _websocket(nullptr),
  _flagForReboot(false) {
    snprintf(_hostname, sizeof(_hostname), "esp8266-%06x", ESP.getChipId());
  }

void TreeLightClass::setHostname(const char* hostname) {
  strncpy(_hostname, hostname, sizeof(_hostname)-1);
}

void TreeLightClass::setupWiFi(const char* ssid, const char* pass) {
  strncpy(_ssid, ssid, sizeof(_ssid)-1);
  strncpy(_pass, pass, sizeof(_pass)-1);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(_hostname);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.disconnect(true);
  _wiFiConnectedHandler = WiFi.onStationModeConnected(std::bind(&TreeLightClass::_onWiFiConnected, this, std::placeholders::_1));
  _wiFiDisconnectedHandler = WiFi.onStationModeDisconnected(std::bind(&TreeLightClass::_onWiFiDisconnected, this, std::placeholders::_1));
}

void TreeLightClass::setupServer(uint16_t port) {
  _webserver = new AsyncWebServer(port);
  _websocket = new AsyncWebSocket("/ws");
  _webserver->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html, [this](const String& var) -> String {
      if (var == "TITLE") {
        return String(this->_hostname);
      }
      return String();
    });
    request->send(response);
  });
  /*
  _webserver->on("/script.js", HTTP_GET, [this](AsyncWebServerRequest *request) {
    request->send_P(200, "text/javascript", script_js);
  });
  */
  _webserver->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  _webserver->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
      _flagForReboot = !Update.hasError();
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", _flagForReboot ? "OK" : "FAIL");
      response->addHeader("Connection", "close");
      request->send(response);
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      if (!index) {
#if defined ARDUINO_ARCH_ESP8266
        Update.runAsync(true);
        size_t size = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#elif defined ARDUINO_ARCH_ESP32
        size_t size = UPDATE_SIZE_UNKNOWN;
#endif
        if (!Update.begin(size)) {
#ifdef ARDUINO_ARCH_ESP8266
          Update.printError(*this);
#endif
        }
      }
      if (!Update.hasError()) {
        if (Update.write(data, len) != len) {
#ifdef ARDUINO_ARCH_ESP8266
          Update.printError(*this);
#endif
        }
      }
      if (final) {
        if (Update.end(true)) {
          this->printf("Update Success: %uB\n", index+len);
        } else {
#ifdef ARDUINO_ARCH_ESP8266
          Update.printError(*this);
#endif
        }
      }
    });
  _websocket->onEvent(std::bind(&TreeLightClass::_wsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                                                 std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  _webserver->addHandler(_websocket);
  _webserver->onNotFound([](AsyncWebServerRequest *request){
    request->send(404, "text/plain", "Not found");
  });
}

void TreeLightClass::setupMqtt(const IPAddress broker, const uint16_t port) {
  AsyncMqttClient::onConnect(std::bind(&TreeLightClass::_onMqttConnected, this));
  AsyncMqttClient::onDisconnect(std::bind(&TreeLightClass::_onMqttDisconnected, this, std::placeholders::_1));
  AsyncMqttClient::onMessage(std::bind(&TreeLightClass::_mqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                                                             std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  AsyncMqttClient::setServer(broker, port);
  AsyncMqttClient::setKeepAlive(5);
  AsyncMqttClient::setCleanSession(true);
  // set Will only on connect
}

void TreeLightClass::begin() {
  _timer.attach(10, &_connectToWiFi, this);
  _connectToWiFi(this);
}

void TreeLightClass::loop() {
  static uint32_t lastMessages = 0;
  if (millis() - lastMessages > 200) {  // print every 200msec if needed
    lastMessages = millis();
    if (_messageBuffer.size() > 0) {
      _printBuffer();
    }
  }
  if (_flagForReboot) {
    AsyncMqttClient::disconnect();
    this->println("Preparing for reboot");
    uint8_t timer = 2;
    while (--timer) delay(1000);
    ESP.restart();
  }
}

void TreeLightClass::setNode(TreeLightNode& node, const char* value) {  // NOLINT
  strncpy(node.value, value, sizeof(node.value)-1);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "nodes";
  JsonArray& array = root.createNestedArray("data");
  JsonObject& object = array.createNestedObject();
  object["name"] = node.name;
  object["value"] = node.value;
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer* buffer = _websocket->makeBuffer(len);
  if (buffer) {
    root.printTo(reinterpret_cast<char*>(buffer->get()), len + 1);
    _websocket->textAll(buffer);
  }
  char topic[63] = {"\0"};
  strncpy(topic, _hostname, sizeof(topic) - 1);
  strncat(topic, "/", sizeof(topic) - strlen(topic) - 1);
  strncat(topic, node.name, sizeof(topic) - strlen(topic) - 1);
  AsyncMqttClient::publish(topic, 1, true, node.value);
}

void TreeLightClass::updateStats() {
  _updateStats();
}

void TreeLightClass::_connectToWiFi(TreeLightClass* instance) {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(instance->_ssid, instance->_pass);
  } else {
    WiFi.disconnect();
  }
}

void TreeLightClass::_onWiFiConnected(const WiFiEventStationModeConnected& event) {
  _timer.detach();  // stop connecting to WiFi
  _timer.attach(10, &_connectToMqtt, this);
  _webserver->begin();
}

void TreeLightClass::_onWiFiDisconnected(const WiFiEventStationModeDisconnected& event) {
  _timer.detach();  // stop connecting to Mqtt
  _timer.attach(10, &_connectToWiFi, this);
  _connectToWiFi(this);
}

void TreeLightClass::_connectToMqtt(TreeLightClass* instance) {
  static char topic[63] = {"\0"};  // setWill doesn't copy. Make static to keep memory available
  strncpy(topic, instance->_hostname, sizeof(topic) - 1);
  strncat(topic, "/$status/online", sizeof(topic) - strlen(topic) - 1);
  instance->AsyncMqttClient::setWill(topic, 1, true, "false");
  instance->AsyncMqttClient::connect();
}

void TreeLightClass::_onMqttConnected() {
  _timer.detach();  // stop connecting to Mqtt
  char topic[63] = {"\0"};
  strncpy(topic, _hostname, sizeof(topic) - 1);
  strncat(topic, "/$status/online", sizeof(topic) - strlen(topic) - 1);
  AsyncMqttClient::publish(topic, 1, true, "true");
}

void TreeLightClass::_onMqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  if (!_flagForReboot) _timer.attach(10, &_connectToMqtt, this);
  // _connectToMqtt(this);
  Serial.println(static_cast<std::underlying_type<AsyncMqttClientDisconnectReason>::type>(reason));
}

void TreeLightClass::_wsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    // client connected
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["type"] = "nodes";
    JsonArray& array = root.createNestedArray("data");
    for (TreeLightNode* n : TreeLightNode::_nodes) {
      JsonObject& object = array.createNestedObject();
      object["name"] = n->name;
      object["value"] = n->value;
      object["type"] = n->getType();
      object["set"] = n->settable;
    }
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer* buffer = _websocket->makeBuffer(len);
    if (buffer) {
      root.printTo(reinterpret_cast<char*>(buffer->get()), len + 1);
      client->text(buffer);
    }
    _updateStats(client);
  } else if (type == WS_EVT_DISCONNECT) {
    // client disconnected
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo * info = reinterpret_cast<AwsFrameInfo*>(arg);
    if (info->final && info->index == 0 && info->len == len) {
      // the whole message is in a single frame and we got all of it's data
      // info->opcode == WS_TEXT is assumed
      DynamicJsonBuffer jsonBuffer;
      data[len] = 0;
      JsonObject& root = jsonBuffer.parseObject(data);
      if (root.success()) {
        /* parse json and launch action
        if (strcmp(root["action"], "updateNode") == 0) {
          ITreeLightNode* ptr = ITreeLightNode::getNode(root["id"]);
          if (ptr) ptr->callback(root["value"]);
        } else if (strcmp(root["action"], "getNodes") == 0) {
          _sendNodes(client);
        }
        */
      }
    } else {
      // this should not happen. aka not implemented
    }
  }
}

void TreeLightClass::_mqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  // to be implemented
}

void TreeLightClass::_updateStats(AsyncWebSocketClient* client) {
    StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "status";
  JsonObject& data = root.createNestedObject("data");
  // uptime
  data["uptime"] = _uptime.getUptimeStr();
  char topic[63] = {"\0"};
  strncpy(topic, _hostname, sizeof(topic) - 1);
  strncat(topic, "/$status/uptime", sizeof(topic) - strlen(topic) - 1);
  AsyncMqttClient::publish(topic, 1, true, data["uptime"]);
  // WiFi signal
  char signal[4] = {"\0"};
  int32_t value = WiFi.RSSI();
  if (value <= -100) {
    value = 0;
  } else if (value >= -50) {
    value = 100;
  } else {
    value = 2 * (value + 100);
  }
  snprintf(signal, sizeof(signal), "%u%%", value);
  data["signal"] = signal;
  strncpy(topic, _hostname, sizeof(topic) - 1);
  strncat(topic, "/$status/signal", sizeof(topic) - strlen(topic) - 1);
  AsyncMqttClient::publish(topic, 1, true, data["signal"]);
  // free heap
  char freeHeap[7] = {"\0"};
  snprintf(freeHeap, sizeof(freeHeap), "%uB", ESP.getFreeHeap());
  data["free heap"] = freeHeap;
  strncpy(topic, _hostname, sizeof(topic) - 1);
  strncat(topic, "/$status/freeheap", sizeof(topic) - strlen(topic) - 1);
  AsyncMqttClient::publish(topic, 1, true, data["free heap"]);
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer* buffer = _websocket->makeBuffer(len);
  if (buffer) {
    root.printTo(reinterpret_cast<char*>(buffer->get()), len + 1);
    if (client) {
      client->text(buffer);
    } else {
      _websocket->textAll(buffer);
    }
  }
}

size_t TreeLightClass::write(uint8_t str) {
  _messageBuffer.push(str);
  if (_messageBuffer.size() > 199) {
    _printBuffer();
  }
  return 1;
}

void TreeLightClass::_printBuffer() {
    if (_websocket->count() > 0) {
      AsyncWebSocketMessageBuffer* wsBuffer = _websocket->makeBuffer(_messageBuffer.size());
      uint8_t* buff = wsBuffer->get();
      do {
        memcpy(buff, &_messageBuffer.front(), 1);
        _messageBuffer.pop();
        buff++;
      } while (_messageBuffer.size() > 0);
      _websocket->textAll(wsBuffer);
    } else {
      clearQueue(_messageBuffer);
    }
}
