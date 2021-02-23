#ifndef ADMIN_HTML_H
#define ADMIN_HTML_H
const char _admin_html[] PROGMEM = R"rawliteral(

<!DOCTYPE HTML>
<html lang="en">

<head>
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no"/>
  <meta charset="UTF-8">
</head>

<style>
  body {
    font-family:verdana; /* hello */
    background-color: rgb(89 122 142);
    font-size: 1em;
  }
  .c {
    text-align: center;
  }
  td, th {
    padding: 0 15px;
    font-weight: 400;
  }
  th {
    font-size: 1.2em;
  }
  input {
    width:95%;
  }
  .container {
    display: block;
    float: left;
    min-width:260px;
    max-width: 1024px;
    background-color: #fff;
    padding: 50px;
    margin: 40px;
  }
  .content {
    float: left;
  }
  button {
    border-width: 0;
    margin: 0 0 15px 0;
    border-radius: 0.3rem;
    background-color:#1fa3ec;
    color:#fff;
    line-height:2.4rem;
    font-size:1rem;
    width:100%;
  }
  .sel {
    background-color:#0b3044;
  }
  #details {
    float: left;
    text-align: left;
    min-width: 250px;
    font-size: .8em;
    padding: 0 15px;
    margin-top: 0;
  }
  #status {
    color: #0c22ca;
    font-size: .8em;
  }
  #progressBar{
    margin-top: 10px;
  }
  h1, h2 {
    padding: 0 0 5px 0;
    margin-top: 0;
    border-bottom: 1px solid #ccc;
    font-size: 1.4em;
    font-weight: 400;
    margin-bottom: 40px;
  }
  h2 {
    font-size: 1.1em;
    margin-bottom: 0px;
  }
  .q {
    float: right;
    width: 64px;
    text-align: right;
  }
  .nav {
    float: left;
    width: 250px;
    padding: 5px 15px 5px 5px;
  }

  @media only screen and (max-width: 750px) {
    .container {
      padding: 25px;
      margin: 0px;
    }
    .content {
      width: 100%;
      padding-top: 20px;
    }
    body {
      font-family:verdana;
      background-color: #fff;
      font-size: 1em;
    }
    h1, h2 {
      margin-bottom: 10px;
    }
    button {
      margin: 0 0 5px 0;
    }
  }
</style>

<body>
  <div class="container">
    <span style="float:right; font-size: .8em">v%FIRMWARE%</span>
    <h1>Administration</h1>
    <div class="nav">
      <button id="btnGeneral" onclick="showGeneralInfoForm()">General Info</button>
      <button id="btnUlFw" onclick="showUploadFirmwareForm()">Upload Firmware</button>
      <button id="btnUlData" onclick="showUploadDataForm('Upload Files')">Upload Data File</button>
      <button id="btnList" onclick="showListForm()">List Data Files</button>
      <!-- <button onclick="logoutButton()">Logout</button> -->
      <button id="btnReboot" onclick="rebootButton()">Reboot</button>
    </div>
    <div class="content">
      <div id="status"></div>
      <div id="detailsheader"></div>
      <div id="details"></div>
    </div>
  </div>

<script>

var _currBtn = "btnGeneral";
var _uploadFirmwareForm =
  '<p>Select the "firmware.bin" file from your computer to update your device with.</p>' +
  '<br>' +
  '<form id="upload_form" enctype="multipart/form-data" method="post">' +
  '  <input type="file" name="file1" id="file1" onchange="uploadFirmware()"><br>' +
  '  <progress id="progressBar" value="0" max="100" style="width:300px;"></progress>' +
  '  <p id="loaded_n_total"></p>' +
  '</form>';

var _uploadDatapackForm =
  '<p>Select the "data.bin" file from your computer to update your device with.</p>' +
  '<br>' +
  '<form id="upload_form" enctype="multipart/form-data" method="post">' +
  '<input type="file" name="file1" id="file1" onchange="uploadFile()"><br>' +
  '<progress id="progressBar" value="0" max="100" style="width:300px;"></progress>' +
  '<p id="loaded_n_total"></p>' +
  '</form>';

var _generalInfoForm =
  '<p>Access Point IP Addresss: %IP_AP%</p>' +
  '<p>Station IP Addresss: %IP_STA%</p>' +
  '<p>Free Storage: <span id="freespiffs">%FREESPIFFS%</span> | Used Storage: <span id="usedspiffs">%USEDSPIFFS%</span> | Total Storage: <span id="totalspiffs">%TOTALSPIFFS%</span></p>';

var _listFilesForms = '';

function _(el) {
  return document.getElementById(el);
}

function setCurrBtn(btn) {
  _(_currBtn).className = "";
  _currBtn = btn;
  _(btn).className = "sel";
}

function logoutButton() {
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/logout", true);
  xhr.send();
  setTimeout(function(){ window.open("/logged-out","_self"); }, 1000);
}

function rebootButton() {
  setCurrBtn("btnReboot");
  _("status").innerHTML = "Invoking Reboot ...";
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/reboot", true);
  xhr.send();
  window.open("/reboot","_self");
}

function showListForm() {
  setCurrBtn("btnList");
  xmlhttp=new XMLHttpRequest();
  xmlhttp.open("GET", "/list-files", false);
  xmlhttp.send();
  _("detailsheader").innerHTML = "<h2>Files</h2>";
  _("details").innerHTML = xmlhttp.responseText;
}

function downloadDeleteButton(filename, action) {
  var urltocall = "/file?name=" + filename + "&action=" + action;
  xmlhttp=new XMLHttpRequest();
  if (action == "delete") {
    xmlhttp.open("GET", urltocall, false);
    xmlhttp.send();
    _("status").innerHTML = xmlhttp.responseText;
    xmlhttp.open("GET", "/list-files", false);
    xmlhttp.send();
    _("details").innerHTML = xmlhttp.responseText;
  }
}

function showGeneralInfoForm() {
  setCurrBtn("btnGeneral");
  _("detailsheader").innerHTML = "<h2>General Info</h2>"
  _("status").innerHTML = "";
  _("details").innerHTML = _generalInfoForm;
}

function showUploadDataForm() {
  setCurrBtn("btnUlData");
  _("detailsheader").innerHTML = "<h2>Upload Data File</h2>"
  _("status").innerHTML = "";
  _("details").innerHTML = _uploadDatapackForm;
}

// LEFT OFF HERE> Firmware isn't uploading. Need other firmware form.

function showUploadFirmwareForm() {
  setCurrBtn("btnUlFw");
  _("detailsheader").innerHTML = "<h2>Upload Firmware</h2>"
  _("status").innerHTML = "";
  _("details").innerHTML = _uploadFirmwareForm;
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
  ajax.open("POST", "/");
  ajax.send(formdata);
}

function uploadFirmware() {
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
  _("loaded_n_total").innerHTML = "Uploaded " + event.loaded + " bytes";
  var percent = (event.loaded / event.total) * 100;
  _("progressBar").value = Math.round(percent);
  _("status").innerHTML = Math.round(percent) + "% uploaded... please wait";
  if (percent >= 100) {
    _("status").innerHTML = "Please wait, writing file to filesystem";
  }
}

function completeHandler(event) {
  _("status").innerHTML = "Upload Complete";
  _("progressBar").value = 0;
  xmlhttp=new XMLHttpRequest();
  xmlhttp.open("GET", "/list-files", false);
  xmlhttp.send();
  _("status").innerHTML = "File Uploaded";
  _("detailsheader").innerHTML = "<h2>Files<h2>";
  _("details").innerHTML = xmlhttp.responseText;
}

function errorHandler(event) {
  _("status").innerHTML = "Upload Failed";
}

function abortHandler(event) {
  _("status").innerHTML = "inUpload Aborted";
}

showGeneralInfoForm();

</script>
</body>
</html>
)rawliteral";


#endif