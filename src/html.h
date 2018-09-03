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

const char index_html[] PROGMEM = R"***(<!doctype html><html lang="en"><head><meta charset="utf-8"><title>~TITLE~ status page</title><meta name="description" content="Node values and system info from ~TITLE~"><style>body,html{margin:0;padding:0;background:#fff;font-family:Arial,Helvetica,sans-serif;font-size:12px}header{padding:20px;background:#13a6db}section{padding:20px;width:80%}h1{color:#fff;font-size:200%}h2{margin:0;padding:0 0 20px;color:#13a6db}table{border-collapse:collapse;border-spacing:0;border:1px solid #ddd}td,th{text-align:left;padding:5px}tr:nth-child(even){background-color:#f2f2f2}pre{padding:10px;background:#323233;color:#ffc300;overflow:auto;line-height:16px;height:160px}</style></head><body><header><h1>~TITLE~ status page</h1></header><main><section id="mqtt"><h2>MQTT nodes</h2><table><thead><tr><th>Node</th><th>Value</th><th>Set</th></tr></thead><tbody></tbody></table></section><section id="system"><h2>System info</h2><table></table><pre></pre></section><section id="update"><h2>Update</h2><form action="/update" method="post" enctype="multipart/form-data" id="updateForm"><input type="file" name="firmware" id="firmware"> <input type="button" value="Update" id="updateButton" disabled> <input type="submit" value="Submit" id="submitButton"></form><p id="updateMessage"></p></section></main><script>function validJSON(e){try{var t=JSON.parse(e);if(t&&"object"==typeof t)return!0}catch(e){return!1}}function onMessage(e){if(e=e.data,validJSON(e)){e=JSON.parse(e);var t=e.data;if("status"==e.type)Object.keys(t).forEach(function(e){document.getElementById(e.replace(/\W/g,""))?document.getElementById(e.replace(/\W/g,"")).textContent=t[e]:(table=document.getElementById("system").getElementsByTagName("table"),table=table[0],row=table.insertRow(),cell=row.insertCell(0),cell.textContent=e,cell=row.insertCell(1),cell.id=e.replace(/\W/g,""),cell.textContent=t[e])});else if("nodes"==e.type)for(var n=0;n<t.length;++n)document.getElementById(t[n].name)?document.getElementById(t[n].name).textContent=t[n].value:(table=document.getElementById("mqtt").getElementsByTagName("tbody")[0],row=table.insertRow(-1),cell=row.insertCell(0),cell.textContent=t[n].name,cell=row.insertCell(1),cell.id=t[n].name,document.getElementById(t[n].name).textContent=t[n].value,cell=row.insertCell(2),t[n].set?cell.innerHTML="\x3c!-- add code to set value --\x3e-":cell.innerHTML="-")}else{var o=document.getElementById("system").getElementsByTagName("pre");o[0].textContent+=e,o[0].scrollTop=o[0].scrollHeight}}function startWebsocket(){"WebSocket"in window?(ws=new WebSocket(("https:"===window.location.protocol?"wss://":"ws://")+window.location.host+"/ws"),ws.onmessage=onMessage,window.onbeforeunload=function(e){ws.close()}):document.getElementById("body").innerHTML="<h1>Your browser does not support WebSocket.</h1>"}function TreeLightReady(){startWebsocket(),supportAjaxUploadWithProgress()&&(document.getElementById("updateButton").removeAttribute("disabled"),document.getElementById("submitButton").style.display="none",document.getElementById("updateMessage").innerHTML="ready for update",initFullFormAjaxUpload(),initFileOnlyAjaxUpload())}function supportAjaxUploadWithProgress(){function e(){var e=document.createElement("INPUT");return e.type="file","files"in e}function t(){var e=new XMLHttpRequest;return!!(e&&"upload"in e&&"onprogress"in e.upload)}function n(){return!!window.FormData}return e()&&t()&&n()}function initFullFormAjaxUpload(){var e=document.getElementById("updateForm");e.onsubmit=function(){var t=new FormData(e),n=e.getAttribute("action");return sendXHRequest(t,n),!1}}function initFileOnlyAjaxUpload(){document.getElementById("updateButton").onclick=function(e){var t=new FormData,n="/update",o=document.getElementById("firmware"),a=o.files[0];t.append("firmware",a),sendXHRequest(t,n)}}function sendXHRequest(e,t){var n=new XMLHttpRequest;n.upload.addEventListener("loadstart",function(e){document.getElementById("updateMessage").innerHTML="<br />Update started."},!1),n.upload.addEventListener("progress",function(e){},!1),n.upload.addEventListener("load",function(e){document.getElementById("updateMessage").innerHTML+="<br />Firmware uploaded. Waiting for response."},!1),n.onreadystatechange=function(){if(4==n.readyState&&200==n.status)return document.getElementById("updateMessage").innerHTML+="<br />Server message: "+n.responseText,console.log(n.responseText),document.getElementById("updateMessage").innerHTML+="<br />Reloading in 5 seconds...",setTimeout("location.reload(true);",5e3),n.responseText},n.open("POST",t,!0),n.send(e)}document.addEventListener("DOMContentLoaded",function(){TreeLightReady()},!1)</script></body></html>)***";

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
