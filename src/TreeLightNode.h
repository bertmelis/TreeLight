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

#include <Arduino.h>

#include <vector>
#include <functional>

#include <ArduinoJson.h>

#include "TreeLight.h"
#include "Helpers/Helpers.h"

class TreeLightNode {
 public:
  TreeLightNode(const char* name, bool settable);
  ~TreeLightNode();
  static void parseJson(char* json, size_t length);
  static void parseMqtt(char* topic, char* payload, size_t len);
  virtual void runJson(JsonVariant payload) = 0;
  virtual void runMqtt(char* payload, size_t length) = 0;
  static void sendNodes(AsyncWebSocketClient* client);

 protected:
  static TreeLightNode* _findNode(const char* name);
  virtual void getNode(JsonObject* object) = 0;
  void sendNode(const JsonObject& nodeData, const char* value);

 protected:
  const char* _name;
  bool _settable;

 private:
  static std::vector<TreeLightNode*> _nodes;
};

class BoolNode : public TreeLightNode {
 public:
  BoolNode(const char* name, bool settable);
  void setValue(bool value);
  void onMessage(std::function<void(bool)> handler);
  void runJson(JsonVariant payload);
  void runMqtt(char* payload, size_t length);

 protected:
  void getNode(JsonObject* object);

 private:
  bool _value;
  std::function<void(bool)> _handler;
};

class IntNode : public TreeLightNode {
 public:
  IntNode(const char* name, bool settable);
  void setValue(int32_t value);
  void onMessage(std::function<void(int32_t)> handler);
  void runJson(JsonVariant payload);
  void runMqtt(char* payload, size_t length);
  void setRange(int32_t min, int32_t step, int32_t max);

 protected:
  void getNode(JsonObject* object);

 protected:
  int32_t _value;
  int32_t _minimum;
  int32_t _step;
  int32_t _maximum;
  std::function<void(int32_t)> _handler;
};

class FloatNode : public TreeLightNode {
 public:
  FloatNode(const char* name, bool settable, uint8_t decimals = 2);
  void setValue(float value);
  void onMessage(std::function<void(float)> handler);
  void runJson(JsonVariant payload);
  void runMqtt(char* payload, size_t length);
  void setRange(float min, float step, float max);

 protected:
  void getNode(JsonObject* object);

 protected:
  float _value;
  const uint8_t _decimals;
  float _minimum;
  float _step;
  float _maximum;
  std::function<void(float)> _handler;
};

#ifndef MAX_ENUM_LENGTH
#define MAX_ENUM_LENGTH 33
#endif

class EnumNode : public TreeLightNode {
 public:
  EnumNode(const char* name, bool settable);
  void setValue(const char* value);
  void onMessage(std::function<void(const char*)> handler);
  void runJson(JsonVariant payload);
  void runMqtt(char* payload, size_t length);
  void setEnum(const char** range, size_t length);

 protected:
  void getNode(JsonObject* object);

 private:
  char _value[MAX_ENUM_LENGTH];
  const char** _range;
  size_t _length;
  std::function<void(const char*)> _handler;
};
