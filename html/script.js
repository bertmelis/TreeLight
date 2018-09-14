
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
 
function onMessage(message) {
  message = message.data;
  if (validJSON(message)) {
    message = JSON.parse(message);
    var data = message.data;
    // stats management
    if (message.type == 'status') {
      Object.keys(data).forEach(function(key) {
        if (document.getElementById(key.replace(/\W/g, ''))) {
          document.getElementById(key.replace(/\W/g, '')).textContent = data[key];
        } else {
          table = document.getElementById('system').getElementsByTagName('table');
          table = table[0];
          row = table.insertRow();
          cell = row.insertCell(0);
          cell.textContent = key;
          cell = row.insertCell(1);
          cell.id = key.replace(/\W/g, '');
          cell.textContent = data[key];
        }
      });
    // nodes management
    } else if (message.type == 'nodes') {
      // check if elementID already exists
      for (var i = 0; i < data.length; ++i) {
        // check if elementId already exists
        if (document.getElementById(data[i].name)) {
          document.getElementById(data[i].name).textContent = data[i].value;
        // else add it to the table
        } else {
          table = document.getElementById('mqtt').getElementsByTagName('tbody')[0];
          row = table.insertRow(-1);
          cell = row.insertCell(0);
          cell.textContent = data[i].name;
          cell = row.insertCell(1);
          cell.id = data[i].name;
          document.getElementById(data[i].name).textContent = data[i].value;
          cell = row.insertCell(2);
          if (data[i].settable) {
            cell.innerHTML = "<!-- add code to set value -->-";  // depends on this.type
          } else {
            cell.innerHTML = "-";
          }
        }
      }
    }
  } else {
    // message is raw printed message
    var el = document.getElementById('system').getElementsByTagName('pre');
    el[0].textContent += message;
    el[0].scrollTop = el[0].scrollHeight;
  }
}
 
function startWebsocket() {
  if ('WebSocket' in window) {
    ws = new WebSocket(((window.location.protocol === 'https:') ? 'wss://' : 'ws://') + window.location.host + '/ws');
    ws.onopen = function() {
      console.log('ws connected');
      /* esp should automatically send nodes when client is connected */
    };
    ws.onmessage = onMessage;
    ws.onclose = function() { 
      console.log('ws disconnected');
    };
    window.onbeforeunload = function(event) {
      ws.close();
    };
  } else {
    document.getElementById('body').innerHTML = '<h1>Your browser does not support WebSocket.</h1>';
  }
}
 
function TreeLightReady() {
  startWebsocket();
  if (supportAjaxUploadWithProgress()) {
    document.getElementById('updateButton').removeAttribute('disabled');
    document.getElementById('submitButton').style.display = 'none';
    document.getElementById('updateMessage').innerHTML = 'ready for update';
    initFullFormAjaxUpload();
    initFileOnlyAjaxUpload();
  }
}
 
function supportAjaxUploadWithProgress() {
  return supportFileAPI() && supportAjaxUploadProgressEvents() && supportFormData();
  function supportFileAPI() {
    var fi = document.createElement('INPUT');
    fi.type = 'file';
    return 'files' in fi;
  }
  function supportAjaxUploadProgressEvents() {
    var xhr = new XMLHttpRequest();
    return !! (xhr && ('upload' in xhr) && ('onprogress' in xhr.upload));
  }
  function supportFormData() {
    return !! window.FormData;
  }
}
 
function initFullFormAjaxUpload() {
  var form = document.getElementById('updateForm');
  form.onsubmit = function() {
    var formData = new FormData(form);
    var action = form.getAttribute('action');
    sendXHRequest(formData, action);
    return false;  // Avoid normal form submission
  };
}
 
function initFileOnlyAjaxUpload() {
  document.getElementById('updateButton').onclick = function (evt) {
    var formData = new FormData();
    var action = '/update';
    var fileInput = document.getElementById('firmware');
    var file = fileInput.files[0];
    formData.append('firmware', file);
    sendXHRequest(formData, action);
  };
}
 
function sendXHRequest(formData, uri) {
  var xhr = new XMLHttpRequest();
  xhr.upload.addEventListener('loadstart', function(evt) {
    document.getElementById('updateMessage').innerHTML = '<br />Update started.';
  }, false);
  xhr.upload.addEventListener('progress', function(evt) {
    // progress bar is not implemented in html
    // var percent = evt.loaded/evt.total*100;
    // document.getElementById('updateProgress').setAttribute('value', percent);
  }, false);
  xhr.upload.addEventListener('load', function(evt) {
    document.getElementById('updateMessage').innerHTML += '<br />Firmware uploaded. Waiting for response.';
  }, false);
  /* The following doesn't seem to work on Edge and Chrome?
  xhr.addEventListener('readystatechange', function(evt) {
    // ...
  }
  }, false);
  */
  xhr.onreadystatechange = function() {
    if (xhr.readyState == 4 && xhr.status == 200) {
      document.getElementById('updateMessage').innerHTML += '<br />Server message: ' + xhr.responseText;
      console.log(xhr.responseText);
      document.getElementById('updateMessage').innerHTML += '<br />Reloading in 5 seconds...';
      setTimeout("location.reload(true);",5000);
      return xhr.responseText;
    }
  };
  xhr.open('POST', uri, true);
  xhr.send(formData);
}

// launch javascript when finished loading and processing
document.addEventListener('DOMContentLoaded', function() {
  TreeLightReady();
  }, false);
