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

#include "TreeLightNode.h"
#include "Helpers/Helpers.h"

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
    case SWITCH:
      return "SWITCH";
    case NUMBER:
      return "NUMBER";
  }
  return {"\0"};
}

TL_SwitchNode::TL_SwitchNode(const char* name, bool settable) :
  TreeLightNode(name, settable, SWITCH) {}

TL_NumberNode::TL_NumberNode(const char* name, bool settable) :
  TreeLightNode(name, settable, NUMBER),
  min(0),
  step(0),
  max(0) {}

void TL_NumberNode::setProperties(float min, float step, float max) {
  min = min;
  step = step;
  max = max;
}
