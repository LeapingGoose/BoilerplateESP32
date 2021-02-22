#ifndef UPDATE_FIRMWARE_HTML_H
#define UPDATE_FIRMWARE_HTML_H

#include <Arduino.h>

const char _updateFirmwareHtml[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta charset="utf-8" /><title>Firmware Update</title></head>
<style>
body{
background-color: #fff;
color: #444444;
font-family:'Open Sans', sans-serif;
font-size: 13px;
text-align: center;
}
</style>
<h1>Firmware Update</h1>
<h2>via Factory Reset</h2>
<p>Select the 'firmware.bin' file from your computer<br>to update your device with.</p>
<br>
<form id="upload_form" enctype="multipart/form-data" method="post">
  <input type="file" name="file1" id="file1" onchange="uploadFile()"><br>
  <progress id="progressBar" value="0" max="100" style="width:300px;"></progress>
  <h3 id="status"></h3>
  <p id="loaded_n_total"></p>
</form>
<script type="text/javascript">
function _(el) {
  return document.getElementById(el);
}
function uploadFile() {
  var file = _("file1").files[0];
  var formdata = new FormData();
  formdata.append("file1", file);
  var ajax = new XMLHttpRequest();
  ajax.upload.addEventListener("progress", progressHandler, false);
  ajax.addEventListener("load", completeHandler, false);
  ajax.addEventListener("error", errorHandler, false);
  ajax.addEventListener("abort", abortHandler, false);
  ajax.open("POST", "/update-firmware");
  ajax.send(formdata);
}
function progressHandler(event) {
  _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes of " + event.total;
  var percent = (event.loaded / event.total) * 100;
  _("progressBar").value = Math.round(percent);
  _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
}
function completeHandler(event) {
  _("status").innerHTML = event.target.responseText;
  _("progressBar").value = 0; //wil clear progress bar after successful upload
}
function errorHandler(event) {
  _("status").innerHTML = "Upload Failed";
}
function abortHandler(event) {
  _("status").innerHTML = "Upload Aborted";
}
</script>
</html>
)rawliteral";

#endif