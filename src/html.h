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

const char index_html[] PROGMEM = R"***(<!doctype html><html lang="en"><head><meta charset="utf-8"><title>~TITLE~ status page</title><meta name="description" content="Node values and system info from ~TITLE~"><style>body,html{margin:0;padding:0;background:#fff;font-family:Arial,Helvetica,sans-serif;font-size:12px}header{padding:10px 20px;background:#13a6db}section{padding:20px;width:80%}h1{color:#fff;font-size:200%}h2{margin:0;padding:0 0 20px;color:#13a6db;font-size:150%}table{border-collapse:collapse;border-spacing:0;border:1px solid #ddd}td,th{text-align:left;padding:5px}tr:nth-child(even){background-color:#f2f2f2}pre{padding:10px;background:#323233;color:#ffc300;overflow:auto;line-height:16px;height:160px}</style></head><body><header><h1>~TITLE~ status page</h1></header><main><section id="mqtt"><h2>MQTT nodes</h2><table><thead><tr><th>Node</th><th>Value</th><th>Set</th></tr></thead><tbody></tbody></table></section><section id="system"><h2>System info</h2><table></table><pre></pre></section><section id="update"><h2>Update</h2><form action="/update" method="post" enctype="multipart/form-data" id="updateForm"><input type="file" name="firmware" id="firmware"> <input type="submit" value="Update" id="submitButton"></form><p id="updateMessage"></p></section></main><script src="script.js"></script></body></html>)***";

const char script_js[] PROGMEM = R"***(var ws,nodes={};function updateValue(a,b){nodes[b]&&(nodes[b].change(a),nodes[b].timer=setTimeout(function(){nodes[b].update(nodes[b].value)},2e3))}function Node(a,b,c){this.name=a,this.value=b,this.settable=c,this.timer=null}Node.prototype.update=function(a){this.timer&&clearTimeout(this.timer),this.value=a,this.draw()};function BoolNode(a,b,c){Node.call(this,a,b,c)}BoolNode.prototype=Object.create(Node.prototype),BoolNode.prototype.draw=function(){if(document.getElementById(this.name))document.getElementById(this.name).textContent=this.value,this.settable&&(document.getElementById(`input_${this.name}`).checked=this.value);else{let b=document.getElementById("mqtt").getElementsByTagName("tbody")[0],c=b.insertRow(-1),d=c.insertCell(0);d.textContent=this.name,d=c.insertCell(1),d.id=this.name,document.getElementById(this.name).textContent=this.value,d=c.insertCell(2);var a;this.settable?(a=`<input type="checkbox" id="input_${this.name}"`,this.value&&(a+=" checked"),a+=` onclick="updateValue(this, '${this.name}')">`):a="<!-- not settable -->",d.innerHTML=a}},BoolNode.prototype.change=function(a){var b={};b.type="nodes",b.data={},b.data.name=this.name,b.data.value=a.checked,ws.send(JSON.stringify(b))};function IntNode(a,b,c,d){Node.call(this,a,b,c),this.set=d}IntNode.prototype=Object.create(Node.prototype),IntNode.prototype.draw=function(){if(document.getElementById(this.name))document.getElementById(this.name).textContent=this.value,this.settable&&(document.getElementById("input_"+this.name).value=this.value,document.getElementById("inputVal_"+this.name).textContent=this.value);else{let b=document.getElementById("mqtt").getElementsByTagName("tbody")[0],c=b.insertRow(-1),d=c.insertCell(0);d.textContent=this.name,d=c.insertCell(1),d.id=this.name,document.getElementById(this.name).textContent=this.value,d=c.insertCell(2);var a;this.settable?(a=`<input type="range" id="input_${this.name}"`,a+=` min="${this.set.min}"`,a+=` step="${this.set.step}"`,a+=` max="${this.set.max}"`,this.value&&(a+=` value="${this.value}"`),a+=` onclick="updateValue(this, '${this.name}')"`,a+=` oninput="document.getElementById('inputVal_${this.name}').textContent = this.value">`,a+=` <span id="inputVal_${this.name}">${this.value}</span>`):a="<!-- not settable -->",d.innerHTML=a}},IntNode.prototype.change=function(a){var b={};b.type="nodes",b.data={},b.data.name=this.name,b.data.value=parseInt(a.value),ws.send(JSON.stringify(b))};function FloatNode(a,b,c,d){Node.call(this,a,b,c),this.set=d}FloatNode.prototype=Object.create(IntNode.prototype),FloatNode.prototype.change=function(a){var b={};b.type="nodes",b.data={},b.data.name=this.name,b.data.value=parseFloat(a.value),ws.send(JSON.stringify(b))};function EnumNode(a,b,c,d){Node.call(this,a,b,c),this.set=d}EnumNode.prototype=Object.create(Node.prototype),EnumNode.prototype.draw=function(){if(!document.getElementById(this.name)){let d=document.getElementById("mqtt").getElementsByTagName("tbody")[0],f=d.insertRow(-1),g=f.insertCell(0);g.textContent=this.name,g=f.insertCell(1),g.id=this.name,document.getElementById(this.name).textContent=this.value,g=f.insertCell(2);var c;if(this.settable){c=`<select id="input_${this.name}" onChange="updateValue(this, '${this.name}')">`;for(var b=0;b<Object.keys(this.set).length;++b)c+=`<option id="inputVal_${this.set[b]}" value="${this.set[b]}"`,this.value==this.set[b]&&(c+=` selected`),c+=`>${this.set[b]}</option>`;c+=`</select>`}else c="<!-- not settable -->";g.innerHTML=c}else if(document.getElementById(this.name).textContent=this.value,this.settable){for(var a=document.getElementById(`input_${this.name}`).getElementsByTagName("OPTION"),b=0;b<a.length;b++)a[b].selected=!1;document.getElementById("inputVal_"+this.value).selected=!0}},EnumNode.prototype.change=function(a){var b={};b.type="nodes",b.data={},b.data.name=this.name,b.data.value=a.value;for(var c=a.getElementsByTagName("OPTION"),d=0;d<c.length;d++)c[d].selected=!1;document.getElementById("inputVal_"+this.value).selected=!0,ws.send(JSON.stringify(b))};function validJSON(a){try{var b=JSON.parse(a);if(b&&"object"==typeof b)return!0}catch(c){return!1}}function onMessage(a){a=a.data;try{if(validJSON(a)){a=JSON.parse(a);var b;if("status"==a.type)b=a.data,Object.keys(b).forEach(function(f){if(document.getElementById(f.replace(/\W/g,"")))document.getElementById(f.replace(/\W/g,"")).textContent=b[f];else{let g=document.getElementById("system").getElementsByTagName("table");g=g[0];let h=g.insertRow(),j=h.insertCell(0);j.textContent=f,j=h.insertCell(1),j.id=f.replace(/\W/g,""),j.textContent=b[f]}});else if("nodes"==a.type)for(var c=0;c<a.data.length;++c)if(b=a.data[c],nodes[b.name])nodes[b.name].update(b.value);else{switch(b.type){case"BOOL":nodes[b.name]=new BoolNode(b.name,b.value,b.settable);break;case"INT":nodes[b.name]=new IntNode(b.name,b.value,b.settable,b.set);break;case"FLOAT":nodes[b.name]=new FloatNode(b.name,b.value,b.settable,b.set);break;case"ENUM":console.log(a),nodes[b.name]=new EnumNode(b.name,b.value,b.settable,b.set);}nodes[b.name].draw()}}else{var d=document.getElementById("system").getElementsByTagName("pre");d[0].textContent+=a,d[0].scrollTop=d[0].scrollHeight}}catch(f){console.log(f)}}function initWebsocket(){"WebSocket"in window?(ws=new WebSocket(("https:"===window.location.protocol?"wss://":"ws://")+window.location.host+"/ws"),ws.onopen=function(){},ws.onmessage=onMessage,ws.onclose=function(){},window.onbeforeunload=function(){ws.close()}):document.getElementById("body").innerHTML="<h1>Your browser does not support WebSocket.</h1>"}function initFirmwareUpload(){var a=document.getElementById("updateForm");a.onsubmit=function(){var b=new FormData(a),c=a.getAttribute("action"),d=document.getElementById("firmware"),f=d.files[0];return b.append("firmware",f),sendXHRequest(b,c),!1}}function sendXHRequest(a,b){var c=new XMLHttpRequest;c.upload.addEventListener("loadstart",function(){document.getElementById("updateMessage").innerHTML="<br />Update started."},!1),c.upload.addEventListener("progress",function(){},!1),c.upload.addEventListener("load",function(){document.getElementById("updateMessage").innerHTML+="<br />Firmware uploaded. Waiting for response."},!1),c.onreadystatechange=function(){if(4==c.readyState&&200==c.status)return document.getElementById("updateMessage").innerHTML+="<br />Server message: "+c.responseText,document.getElementById("updateMessage").innerHTML+="<br />Reloading in 5 seconds...",setTimeout(function(){location.reload(!0)},5e3),c.responseText},c.open("POST",b,!0),c.send(a)}document.addEventListener("DOMContentLoaded",function(){initWebsocket(),initFirmwareUpload()},!1);)***";

const uint16_t favicon_ico_gz_len = 393;
const uint8_t favicon_ico_gz[] PROGMEM = {
  0x1F, 0x8B, 0x08, 0x08, 0x7F, 0x7C, 0x0E, 0x5B, 0x02, 0x00, 0x66, 0x61, 0x76, 0x69, 0x63, 0x6F,
  0x6E, 0x2E, 0x69, 0x63, 0x6F, 0x00, 0x9D, 0x8F, 0xBB, 0x4A, 0x03, 0x51, 0x10, 0x86, 0xFF, 0x45,
  0xC1, 0x52, 0x0B, 0xB1, 0x30, 0xF1, 0x52, 0x5A, 0x4A, 0x88, 0x62, 0x65, 0x1A, 0x03, 0xF1, 0x8E,
  0xF8, 0x02, 0x26, 0x36, 0x96, 0x36, 0xA6, 0x35, 0xB5, 0xE0, 0x03, 0xF8, 0x36, 0x76, 0xDE, 0x12,
  0x15, 0x6B, 0x8B, 0xC4, 0x18, 0x85, 0x20, 0x44, 0xF0, 0xB6, 0x91, 0xE0, 0xFA, 0xCF, 0x9C, 0x61,
  0x59, 0x0C, 0xAC, 0x9A, 0xB3, 0x7C, 0xEC, 0x30, 0x7B, 0xBE, 0xF9, 0x67, 0x01, 0x8F, 0xCF, 0xD0,
  0x90, 0xBC, 0x27, 0xB1, 0xD3, 0x0F, 0x8C, 0x00, 0x98, 0x22, 0x6C, 0xB1, 0xE3, 0xFA, 0x7A, 0xF8,
  0x6D, 0x17, 0x42, 0xE4, 0x14, 0x71, 0x43, 0x02, 0x6C, 0xA1, 0x8E, 0x0C, 0x3E, 0x90, 0x40, 0x93,
  0x03, 0x3A, 0xE4, 0x30, 0x08, 0x02, 0x08, 0xAC, 0x4B, 0xDA, 0x1B, 0x45, 0x0B, 0xF3, 0xBC, 0x93,
  0x47, 0x55, 0x1C, 0x72, 0x49, 0x36, 0xB5, 0xDE, 0x63, 0x7F, 0x01, 0x3E, 0x52, 0x78, 0x33, 0x5F,
  0xDE, 0x49, 0x32, 0x4C, 0x5A, 0xDA, 0x9B, 0xB6, 0x3B, 0xBC, 0x6B, 0xFE, 0x3A, 0xF1, 0xC8, 0x75,
  0x74, 0x07, 0x5A, 0x0D, 0x9B, 0x71, 0x44, 0x0E, 0xB4, 0x4E, 0xE0, 0x49, 0xBF, 0x15, 0xC2, 0xEC,
  0x8A, 0xED, 0x07, 0xD6, 0x1B, 0xB6, 0x83, 0xAF, 0xF3, 0xD3, 0x78, 0x35, 0xDF, 0x27, 0xAE, 0x4E,
  0x59, 0x76, 0x91, 0x38, 0x7F, 0x2D, 0xE2, 0x7B, 0xE4, 0x4A, 0xFB, 0x79, 0xDC, 0x6B, 0xCE, 0x18,
  0x6A, 0xE2, 0x29, 0x49, 0x34, 0x2D, 0xBB, 0x66, 0x6E, 0xD9, 0xDC, 0x10, 0xF9, 0x17, 0xDB, 0xA1,
  0xAD, 0x39, 0xB3, 0x78, 0x09, 0xFD, 0x34, 0xDD, 0xAC, 0xFC, 0x77, 0x98, 0xBD, 0xAA, 0x5E, 0xF7,
  0x8C, 0x8A, 0xED, 0xD0, 0xD0, 0xBC, 0x71, 0xDC, 0x72, 0x8F, 0x47, 0xCB, 0xBE, 0x33, 0xF7, 0xA2,
  0xCB, 0x35, 0x64, 0x6E, 0xB8, 0x83, 0xE4, 0xCD, 0x71, 0x87, 0x19, 0xBC, 0x4B, 0xCD, 0x7E, 0xDB,
  0xFC, 0x65, 0xB9, 0x1B, 0x33, 0xA3, 0xAC, 0xF7, 0x0A, 0x6E, 0x07, 0xCB, 0xAE, 0x9B, 0x7B, 0x1E,
  0xE7, 0x9A, 0xBF, 0x12, 0xDD, 0xE1, 0x47, 0xF6, 0x52, 0xAC, 0x6F, 0x48, 0x8E, 0xED, 0xF0, 0x80,
  0xED, 0x30, 0xFB, 0xEC, 0x57, 0xD7, 0x90, 0x1C, 0x73, 0x3E, 0x05, 0xAB, 0x17, 0xFF, 0xE4, 0x1B,
  0x92, 0x27, 0x9E, 0x71, 0xFA, 0x1F, 0xD7, 0xFC, 0x5C, 0xC4, 0xCF, 0x49, 0xAF, 0x87, 0x19, 0x27,
  0x82, 0xD4, 0x3D, 0xFA, 0x59, 0x21, 0xEE, 0x8E, 0x57, 0x82, 0x3B, 0x1E, 0x50, 0x12, 0xFA, 0x80,
  0x63, 0x52, 0x1D, 0x70, 0x3C, 0x0F, 0x3A, 0xFC, 0x09, 0x47, 0x27, 0xE3, 0xF8, 0xDA, 0x77, 0x7C,
  0x03, 0x94, 0xC4, 0xE5, 0x40, 0x7E, 0x04, 0x00, 0x00
};
