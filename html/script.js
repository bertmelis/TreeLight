/* jshint esversion: 6 */

var ws;  // global
var nodes = {};

/* jshint ignore:start */
function updateValue(element, nodeName) {
  if (nodes[nodeName]) {
    nodes[nodeName].change(element);
    // reset after 2s timeout
    nodes[nodeName].timer = setTimeout(function() {
      nodes[nodeName].update(nodes[nodeName].value);
    }, 2000);
  }
}
/* jshint ignore:end */

function Node(name, value, settable) {
  this.name = name;
  this.value = value;
  this.settable = settable;
  this.timer = null;
}
Node.prototype.update = function(value) {
  if (this.timer) clearTimeout(this.timer);
  this.value = value;
  this.draw();
};

function BoolNode(name, value, settable) {
  Node.call(this, name, value, settable);
}
BoolNode.prototype = Object.create(Node.prototype);
BoolNode.prototype.draw = function() {
  // check if elementId already exists
  if (document.getElementById(this.name)) {
    document.getElementById(this.name).textContent = this.value;
    if (this.settable) document.getElementById(`input_${this.name}`).checked = this.value;
  // else add it to the table
  } else {
    let table = document.getElementById("mqtt").getElementsByTagName("tbody")[0];
    let row = table.insertRow(-1);
    let cell = row.insertCell(0);
    cell.textContent = this.name;
    cell = row.insertCell(1);
    cell.id = this.name;
    document.getElementById(this.name).textContent = this.value;
    cell = row.insertCell(2);
    var html;
    if (this.settable) {
      html = `<input type="checkbox" id="input_${this.name}"`;
      if (this.value) html += " checked";
      html += ` onclick="updateValue(this, '${this.name}')">`;
    } else {
      html = "<!-- not settable -->";
    }
    cell.innerHTML = html;
  }
};
BoolNode.prototype.change = function(element) {
  var message = {};
  message.type = "nodes";
  message.data = {};
  message.data.name = this.name;
  message.data.value = element.checked;
  ws.send(JSON.stringify(message));
};

function IntNode(name, value, settable, set) {
  Node.call(this, name, value, settable);
  this.set = set;
}
IntNode.prototype = Object.create(Node.prototype);
IntNode.prototype.draw = function() {
  // check if elementId already exists
  if (document.getElementById(this.name)) {
    document.getElementById(this.name).textContent = this.value;
    if (this.settable) {
      document.getElementById("input_" + this.name).value = this.value;
      document.getElementById("inputVal_" + this.name).textContent = this.value;
    }
  // else add it to the table
  } else {
    let table = document.getElementById("mqtt").getElementsByTagName("tbody")[0];
    let row = table.insertRow(-1);
    let cell = row.insertCell(0);
    cell.textContent = this.name;
    cell = row.insertCell(1);
    cell.id = this.name;
    document.getElementById(this.name).textContent = this.value;
    cell = row.insertCell(2);
    var html;
    if (this.settable) {
      html = `<input type="range" id="input_${this.name}"`;
      html += ` min="${this.set.min}"`;
      html += ` step="${this.set.step}"`;
      html += ` max="${this.set.max}"`;
      if (this.value) html += ` value="${this.value}"`;
      html += ` onclick="updateValue(this, '${this.name}')"`;
      html += ` oninput="document.getElementById('inputVal_${this.name}').textContent = this.value">`;
      html += ` <span id="inputVal_${this.name}">${this.value}</span>`;
    } else {
      html = "<!-- not settable -->";
    }
    cell.innerHTML = html;
  }
};
IntNode.prototype.change = function(element) {
  var message = {};
  message.type = "nodes";
  message.data = {};
  message.data.name = this.name;
  message.data.value = parseInt(element.value);
  ws.send(JSON.stringify(message));
};
function FloatNode(name, value, settable, set) {
  Node.call(this, name, value, settable);
  this.set = set;
}
FloatNode.prototype = Object.create(IntNode.prototype);
FloatNode.prototype.change = function(element) {
  var message = {};
  message.type = "nodes";
  message.data = {};
  message.data.name = this.name;
  message.data.value = parseFloat(element.value);
  ws.send(JSON.stringify(message));
};
function EnumNode(name, value, settable, set) {
  Node.call(this, name, value, settable);
  this.set = set;
}
EnumNode.prototype = Object.create(Node.prototype);
EnumNode.prototype.draw = function() {
  // check if elementId already exists
  if (document.getElementById(this.name)) {
    document.getElementById(this.name).textContent = this.value;
    if (this.settable) {
      var options = document.getElementById(`input_${this.name}`).getElementsByTagName('OPTION');
      for (var i=0; i < options.length; i++){
        options[i].selected = false;
      }
      document.getElementById("inputVal_" + this.value).selected = true;
    }
  // else add it to the table
  } else {
    let table = document.getElementById("mqtt").getElementsByTagName("tbody")[0];
    let row = table.insertRow(-1);
    let cell = row.insertCell(0);
    cell.textContent = this.name;
    cell = row.insertCell(1);
    cell.id = this.name;
    document.getElementById(this.name).textContent = this.value;
    cell = row.insertCell(2);
    var html;
    if (this.settable) {
      html = `<select id="input_${this.name}" onChange="updateValue(this, '${this.name}')">`;
      for (var i = 0; i < Object.keys(this.set).length; ++i) {
        html += `<option id="inputVal_${this.set[i]}" value="${this.set[i]}"`;
        if (this.value == this.set[i]) html += ` selected`;
        html += `>${this.set[i]}</option>`;
      }
      html += `</select>`;
    } else {
      html = "<!-- not settable -->";
    }
    cell.innerHTML = html;
  }
};
EnumNode.prototype.change = function(element) {
  var message = {};
  message.type = "nodes";
  message.data = {};
  message.data.name = this.name;
  message.data.value = element.value;
  var options =element.getElementsByTagName('OPTION');
  for (var i=0; i < options.length; i++){
    options[i].selected = false;
  }
  document.getElementById("inputVal_" + this.value).selected = true;
  ws.send(JSON.stringify(message));
};

function validJSON(jsonString) {
  try {
    var o = JSON.parse(jsonString);
    if (o && typeof o === "object") {
      return true;
    }
  }
  catch (e) {
    return false;
  }
}

function onMessage(message){
  message = message.data;
  try {
    if (validJSON(message)) {
      message = JSON.parse(message);
      var data;
      // stats management
      if (message.type == "status") {
        data = message.data;
        Object.keys(data).forEach(function(key) {
          if (document.getElementById(key.replace(/\W/g, ""))) {
            document.getElementById(key.replace(/\W/g, "")).textContent = data[key];
          } else {
            let table = document.getElementById("system").getElementsByTagName("table");
            table = table[0];
            let row = table.insertRow();
            let cell = row.insertCell(0);
            cell.textContent = key;
            cell = row.insertCell(1);
            cell.id = key.replace(/\W/g, "");
            cell.textContent = data[key];
          }
        });
      // nodes management
      } else if (message.type == "nodes") {
        for (var i = 0; i < message.data.length; ++i) {
          data = message.data[i];
          if (nodes[data.name]) {
            nodes[data.name].update(data.value);
          } else {
            switch (data.type) {
              case "BOOL":
                nodes[data.name] = new BoolNode(data.name, data.value, data.settable);
                break;
              case "INT":
                nodes[data.name] = new IntNode(data.name, data.value, data.settable, data.set);
                break;
              case "FLOAT":
                nodes[data.name] = new FloatNode(data.name, data.value, data.settable, data.set);
                break;
              case "ENUM":
              console.log(message);
                nodes[data.name] = new EnumNode(data.name, data.value, data.settable, data.set);
                break;
            }
            nodes[data.name].draw();
          }
        }
      }
    } else {
      // message is raw printed message
      var el = document.getElementById("system").getElementsByTagName("pre");
      el[0].textContent += message;
      el[0].scrollTop = el[0].scrollHeight;
    }
  }
  catch(e) {
    console.log(e);
  }
}

function initWebsocket() {
  if ("WebSocket" in window) {
    ws = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.host + "/ws");
    ws.onopen = function() {
      // console.log("ws connected");
      // esp should automatically send nodes when client is connected
    };
    ws.onmessage = onMessage;
    ws.onclose = function() { 
      // console.log("ws disconnected");
    };
    window.onbeforeunload = function(event) {
      ws.close();
    };
  } else {
    document.getElementById("body").innerHTML = "<h1>Your browser does not support WebSocket.</h1>";
  }
}

function initFirmwareUpload() {
  var form = document.getElementById("updateForm");
  form.onsubmit = function() {
    var formData = new FormData(form);
    var action = form.getAttribute("action");
    var fileInput = document.getElementById("firmware");
    var file = fileInput.files[0];
    formData.append("firmware", file);
    sendXHRequest(formData, action);
    return false;  // Avoid normal form submission
  };
}

function sendXHRequest(formData, uri) {
  var xhr = new XMLHttpRequest();
  xhr.upload.addEventListener("loadstart", function(evt) {
    document.getElementById("updateMessage").innerHTML = "<br />Update started.";
  }, false);
  xhr.upload.addEventListener("progress", function(evt) {
    // progress bar is not implemented in html
    // var percent = evt.loaded/evt.total*100;
    // document.getElementById("updateProgress").setAttribute("value", percent);
  }, false);
  xhr.upload.addEventListener("load", function(evt) {
    document.getElementById("updateMessage").innerHTML += "<br />Firmware uploaded. Waiting for response.";
  }, false);
  /* The following doesn't seem to work on Edge and Chrome?
  xhr.addEventListener("readystatechange", function(evt) {
    // ...
  }
  }, false);
  */
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      document.getElementById("updateMessage").innerHTML += "<br />Server message: " + xhr.responseText;
      document.getElementById("updateMessage").innerHTML += "<br />Reloading in 5 seconds...";
      setTimeout(function() {
        location.reload(true);
      }, 5000);
      return xhr.responseText;
    }
  };
  xhr.open("POST", uri, true);
  xhr.send(formData);
}

document.addEventListener("DOMContentLoaded", function() {
  initWebsocket();
  initFirmwareUpload();
}, false);
