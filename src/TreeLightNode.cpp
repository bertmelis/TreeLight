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

#include <TreeLightNode.h>

std::vector<TreeLightNode*> TreeLightNode::_nodes;

TreeLightNode::TreeLightNode(const char* name, bool settable) :
  _name(name),
  _settable(settable) {
    _nodes.push_back(this);
  }

TreeLightNode::~TreeLightNode() {
  std::pair<bool, int> index = findInVector(&_nodes, this);
  if (index.first) _nodes.erase(_nodes.begin() + index.second);
}

void TreeLightNode::parseJson(char* json, size_t length) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success()) {
    TreeLightClass::get().print("json parsing error\n");
    return;
  }
  const char* type = root["type"];
  const char* name = root["data"]["name"];
  const JsonVariant value = root["data"]["value"];
  if (strcmp(type, "nodes") == 0 && name && value.success()) {
    TreeLightNode* node = _findNode(name);
    if (node) {
      node->runJson(value);
    } else {
      TreeLightClass::get().print("node not found\n");
    }
  } else {
    TreeLightClass::get().print("json format error\n");
  }
}

void TreeLightNode::parseMqtt(char* topic, char* payload, size_t length) {
  char* cpyTopic = strdup(topic);  // TODO(bertmelis): is this needed?
  char* nodeName;
  nodeName = strsep(&cpyTopic, "/");
  nodeName = strsep(&cpyTopic, "/");  // call strsep() twice and get the 2nd string from topic
  TreeLightNode* node = _findNode(nodeName);
  if (node) {
    // convert to Json
    StaticJsonBuffer<100> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["value"]= payload;
    node->runMqtt(payload, length);
  }
}

void TreeLightNode::sendNodes(AsyncWebSocketClient* client) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "nodes";
  JsonArray& array = root.createNestedArray("data");
  for (const auto& node : _nodes) {
    JsonObject& object = array.createNestedObject();
    (*node).getNode(&object);
  }
  size_t len = root.measureLength();
  AsyncWebSocketMessageBuffer * buffer = TreeLightClass::get()._websocket->makeBuffer(len);
  if (buffer) {
    root.printTo(reinterpret_cast<char*>(buffer->get()), len + 1);
    if (client) client->text(buffer);
  }
}

TreeLightNode* TreeLightNode::_findNode(const char* name) {
  for (const auto& node : _nodes) {
    if (strcmp((*node)._name, name) == 0) {
      return node;
    }
  }
  return nullptr;
}

void TreeLightNode::sendNode(const JsonObject& nodeData, const char* value) {
  if (TreeLightClass::get()._websocket) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["type"] = "nodes";
    JsonArray& array = root.createNestedArray("data");
    JsonObject& object = array.createNestedObject();
    object["name"] = nodeData["name"];
    object["value"] = nodeData["value"];
    size_t len = root.measureLength();
    AsyncWebSocketMessageBuffer * buffer = TreeLightClass::get()._websocket->makeBuffer(len);
    if (buffer) {
      root.printTo(reinterpret_cast<char*>(buffer->get()), len + 1);
      TreeLightClass::get()._websocket->textAll(buffer);
    }
  }
  char topic[63] = {"\0"};
  strncpy(topic, TreeLightClass::get()._hostname, sizeof(topic) - 1);
  strncat(topic, "/", sizeof(topic) - strlen(topic) - 1);
  const char* name = nodeData["name"];
  strncat(topic, name, sizeof(topic) - strlen(topic) - 1);
  TreeLightClass::get().publish(topic, 1, true, value);
}

BoolNode::BoolNode(const char* name, bool settable) :
  TreeLightNode(name, settable),
  _value(false) {}

void BoolNode::setValue(bool value) {
  _value = value;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = _name;
  root["value"] = _value;
  sendNode(root, (value) ? "1" : "0");
}

void BoolNode::onMessage(std::function<void(bool)> handler) {
  _handler = handler;
}

void BoolNode::runJson(JsonVariant payload) {
  if (_handler) _handler(payload.as<bool>());
}

void BoolNode::runMqtt(char* payload, size_t length) {
  if (_handler) {
    bool value = false;
    if (strcmp(payload, "1") == 0) {
      value = true;
      _handler(value);
    } else if (strcmp(payload, "0") == 0) {
      value = false;
      _handler(value);
    }
  }
}

void BoolNode::getNode(JsonObject* object) {
  (*object)["name"] = _name;
  (*object)["type"] = "BOOL";
  (*object)["value"] = _value;
  (*object)["settable"] = _settable;
}

IntNode::IntNode(const char* name, bool settable) :
  TreeLightNode(name, settable),
  _value(0),
  _minimum(0),
  _step(0),
  _maximum(0) {}

void IntNode::setValue(int32_t value) {
  _value = value;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = _name;
  root["value"] = _value;
  char valueStr[12] = {"\0"};
  snprintf(valueStr, sizeof(valueStr), "%i", value);
  sendNode(root, valueStr);
}

void IntNode::onMessage(std::function<void(int32_t)> handler) {
  _handler = handler;
}

void IntNode::runJson(JsonVariant payload) {
  if (_handler) _handler(payload.as<int>());
}

void IntNode::runMqtt(char* payload, size_t length) {
  if (_handler) {
    int value = 0;
    sscanf(payload, "%i", &value);
    _handler(value);
  }
}

void IntNode::getNode(JsonObject* object) {
  (*object)["name"] = _name;
  (*object)["type"] = "INT";
  (*object)["value"] = _value;
  (*object)["settable"] = _settable;
  if (_settable) {
    JsonObject& set = (*object).createNestedObject("set");
    set["min"] = _minimum;
    set["step"] = _step;
    set["max"] = _maximum;
  }
}

void IntNode::setRange(int32_t min, int32_t step, int32_t max) {
  _minimum = min;
  _step = step;
  _maximum = max;
}

FloatNode::FloatNode(const char* name, bool settable, uint8_t decimals) :
  TreeLightNode(name, settable),
  _value(0),
  _decimals(decimals),
  _minimum(0),
  _step(0),
  _maximum(0) {}

void FloatNode::setValue(float value) {
  _value = value;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = _name;
  root["value"] = _value;
  char valueStr[12] = {"\0"};
  snprintf(valueStr, sizeof(valueStr), "%.*f", _decimals, value);
  sendNode(root, valueStr);
}

void FloatNode::onMessage(std::function<void(float)> handler) {
  _handler = handler;
}

void FloatNode::runJson(JsonVariant payload) {
  if (_handler) _handler(payload.as<float>());
}

void FloatNode::runMqtt(char* payload, size_t length) {
  if (_handler) {
    float value = 0;
    sscanf(payload, "%f", &value);
    _handler(value);
  }
}

void FloatNode::getNode(JsonObject* object) {
  (*object)["name"] = _name;
  (*object)["type"] = "FLOAT";
  (*object)["value"] = _value;
  (*object)["settable"] = _settable;
  if (_settable) {
    JsonObject& set = (*object).createNestedObject("set");
    set["min"] = _minimum;
    set["step"] = _step;
    set["max"] = _maximum;
  }
}

void FloatNode::setRange(float min, float step, float max) {
  _minimum = min;
  _step = step;
  _maximum = max;
}

EnumNode::EnumNode(const char* name, bool settable) :
  TreeLightNode(name, settable),
  _value{"\0"} {}

void EnumNode::setValue(const char* value) {
  strncpy(_value, value, sizeof(_value));
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["name"] = _name;
  root["value"] = _value;
  sendNode(root, _value);
}

void EnumNode::onMessage(std::function<void(const char*)> handler) {
  _handler = handler;
}

void EnumNode::runJson(JsonVariant payload) {
  if (_handler) _handler(payload.as<char*>());
}

void EnumNode::runMqtt(char* payload, size_t length) {
  // payload is not null terminated, so create a new c-string
  char* value = new char[length + 1];
  strncpy(value, payload, length);
  value[length] = 0;
  if (_handler) {
    _handler(value);
  }
  // and delete the newly created c-string to avoid a memory leak
  delete[] value;
}

void EnumNode::getNode(JsonObject* object) {
  (*object)["name"] = _name;
  (*object)["type"] = "ENUM";
  (*object)["value"] = _value;
  (*object)["settable"] = _settable;
  if (_settable) {
    JsonObject& set = (*object).createNestedObject("set");
    for (uint8_t i = 0; i < _length; ++i) {
      char index[2] = {"\0"};
      snprintf(index, sizeof(index), "%u", i);
      set[index] = _range[i];
    }
  }
}

void EnumNode::setEnum(const char** range, size_t length) {
  _range = range;
  _length = length;
}
