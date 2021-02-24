/**
 * See admin-html.raw for uncompressed version of this file.
 * Compression was done using the online tool at https://htmlcompressor.com/compressor/
 */
#ifndef ADMIN_HTML_H
#define ADMIN_HTML_H
const char _admin_html[] PROGMEM = R"rawliteral(



<!DOCTYPE HTML>
<html lang=en>
<head>
<meta name=viewport content="width=device-width, initial-scale=1, user-scalable=no"/>
<meta charset=UTF-8>
</head>
<style>body{font-family:verdana;background-color:grey;font-size:1em}.c{text-align:center}td,th{padding:0 15px;font-weight:400}th{font-size:1.2em}input{width:95%}.container{display:block;float:left;min-width:260px;max-width:1024px;background-color:#fff;padding:50px;margin:40px}.content{float:left}button{border-width:0;margin:0 0 15px 0;border-radius:.3rem;background-color:#9a9a9a;color:#fff;line-height:2.4rem;font-size:1rem;width:100%;cursor:pointer}.sel{background-color:#545353}#details{float:left;text-align:left;min-width:250px;font-size:.8em;margin-top:0}#status{color:#000;font-size:.9em;font-weight:600;display:inline-block;padding-top:15px}#progressBar{margin-top:10px}h1,h2{padding:0 0 5px 0;margin-top:0;border-bottom:1px solid #ccc;font-size:1.4em;font-weight:400;margin-bottom:40px}h2{font-size:1.1em;margin-bottom:0}.q{float:right;width:64px;text-align:right}.nav{float:left;width:250px;padding:5px 15px 5px 5px}@media only screen and (max-width:750px){.container{padding:25px;margin:0}.content{width:100%;padding-top:20px}body{font-family:verdana;background-color:#fff;font-size:1em}h1,h2{margin-bottom:10px}button{margin:0 0 5px 0}}</style>
<body>
<div class=container>
<span style=float:right;font-size:.8em>v%FIRMWARE% (build# %BUILDNUM%)</span>
<h1>Factory Administration</h1>
<div class=nav>
<button id=btnGeneral onclick=showGeneralInfoForm()>General Info</button>
<button id=btnUlFw onclick=showUploadFirmwareForm()>Upload Firmware</button>
<button id=btnUlData onclick=showUploadDataForm()>Upload Data File</button>
<button id=btnList onclick=showListForm()>List Data Files</button>
<button id=btnReboot onclick=rebootButton()>Reboot</button>
</div>
<div class=content>
<div id=detailsheader></div>
<div id=details></div>
<div id=status></div>
</div>
</div>
<script>var _currBtn="btnGeneral";var _uploadFirmwareForm='<p>Select the "firmware.bin" file from your computer to update your device with.</p><br><form id="upload_form" enctype="multipart/form-data" method="post">  <input type="file" name="file1" id="file1" onchange="uploadFirmware()"><br>  <progress id="progressBar" value="0" max="100" style="width:300px;"></progress></form>';var _uploadDatapackForm='<p>Select the "data.bin" file from your computer to update your device with.</p><br><form id="upload_form" enctype="multipart/form-data" method="post"><input type="file" name="file1" id="file1" onchange="uploadFile()"><br><progress id="progressBar" value="0" max="100" style="width:300px;"></progress></form>';var _generalInfoForm='<p>Access Point IP Addresss: %IP_AP%</p><p>Station IP Addresss: %IP_STA%</p><p>Free Storage: <span id="freespiffs">%FREESPIFFS%</span> | Used Storage: <span id="usedspiffs">%USEDSPIFFS%</span> | Total Storage: <span id="totalspiffs">%TOTALSPIFFS%</span></p>';var _listFilesForms="";function _(a){return document.getElementById(a)}function setCurrBtn(a){_(_currBtn).className="";_currBtn=a;_(a).className="sel"}function rebootButton(){setCurrBtn("btnReboot");_("status").innerHTML="Invoking Reboot ...";var a=new XMLHttpRequest();a.open("GET","/reboot",true);a.send();window.open("/reboot","_self")}function showListForm(){setCurrBtn("btnList");xmlhttp=new XMLHttpRequest();xmlhttp.open("GET","/list-files",false);xmlhttp.send();_("detailsheader").innerHTML="<h2>Files</h2>";_("details").innerHTML=xmlhttp.responseText}function downloadDeleteButton(a,c){var b="/datafile?name="+a+"&action="+c;xmlhttp=new XMLHttpRequest();if(c=="delete"){xmlhttp.open("DELETE",b,false);xmlhttp.send();_("status").innerHTML=xmlhttp.responseText;xmlhttp.open("GET","/list-files",false);xmlhttp.send();_("details").innerHTML=xmlhttp.responseText}}function showGeneralInfoForm(){setCurrBtn("btnGeneral");_("detailsheader").innerHTML="<h2>General Info</h2>";_("status").innerHTML="";_("details").innerHTML=_generalInfoForm}function showUploadDataForm(){setCurrBtn("btnUlData");_("detailsheader").innerHTML="<h2>Upload Data File</h2>";_("status").innerHTML="";_("details").innerHTML=_uploadDatapackForm}function showUploadFirmwareForm(){setCurrBtn("btnUlFw");_("detailsheader").innerHTML="<h2>Upload Firmware</h2>";_("status").innerHTML="";_("details").innerHTML=_uploadFirmwareForm}function uploadFile(){var b=_("file1").files[0];var a=new FormData();a.append("file1",b);var c=new XMLHttpRequest();c.upload.addEventListener("progress",progressHandler,false);c.addEventListener("load",datafileUploadCompleteHandler,false);c.addEventListener("error",errorHandler,false);c.addEventListener("abort",abortHandler,false);c.open("POST","/datafile");c.send(a)}function uploadFirmware(){var b=_("file1").files[0];var a=new FormData();a.append("file1",b);var c=new XMLHttpRequest();c.upload.addEventListener("progress",progressHandler,false);c.addEventListener("load",firmwareUploadCompleteHandler,false);c.addEventListener("error",errorHandler,false);c.addEventListener("abort",abortHandler,false);c.open("POST","/update-firmware");c.send(a)}function progressHandler(b){var a=(b.loaded/b.total)*100;_("progressBar").value=Math.round(a);_("status").innerHTML=Math.round(a)+"% uploaded... please wait";if(a>=100){_("status").innerHTML="Saving file..."}}function firmwareUploadCompleteHandler(a){_("status").innerHTML="The firmware update is complete!";_("progressBar").value=0}function datafileUploadCompleteHandler(a){_("status").innerHTML="File upload is complete!";_("progressBar").value=0;xmlhttp=new XMLHttpRequest();xmlhttp.open("GET","/list-files",false);xmlhttp.send();_("status").innerHTML="File Uploaded";_("detailsheader").innerHTML="<h2>Files</h2>";_("details").innerHTML=xmlhttp.responseText}function errorHandler(a){_("status").innerHTML="Upload Failed"}function abortHandler(a){_("status").innerHTML="inUpload Aborted"}showGeneralInfoForm();</script>
</body>
</html>



)rawliteral";
#endif