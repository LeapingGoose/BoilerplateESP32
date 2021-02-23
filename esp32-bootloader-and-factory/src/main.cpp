#include <FS.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
// #include <ESPAsyncWebServer.h>
#include <WebServer.h>
#include <Update.h>
#include "webpages.h"
// #include "update-firmware-html.h"

File _tempFile;
//holds the current upload
File fsUploadFile;

// SSID and password of the AP (Access Point) server created on the ESP32.
#define DEFAULT_BASE_AP_SSID     "WROOM-FACTORY"
#define DEFAULT_BASE_AP_PASSWORD "wroomwroom"
#define WIFI_CHANNEL             6
#define FACTORY_FIRMWARE_VERSION "0.1"

WebServer _server(80);

void   initSpiffs();
void   initWiFi();
void   initWebServer();
void   initRoutes();
String humanReadableSize(const size_t bytes);
String pageProcessor(const String& var);
String listFiles(bool ishtml);
void   onGet_root();
void   on_fileUpload();
void   onGet_listFiles();
void   on_file();
// void   onPost_firmwareUpdate();
// void   onPost_firmwareUpdater();
void   onGet_reboot();
void   onGet_logout();
void   onGet_loggedOut();
void   onGet_404();

void handleFileUpload();


void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  Serial.println("setup()");

  initSpiffs();
  initWiFi();
  initWebServer();
}

void loop(void) {
  _server.handleClient();
  delay(1);
}

void initSpiffs() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Cannot mount SPIFFS volume.");
  } else {
    Serial.println("Mounted SPIFFS volume.");
  }
}

String pageProcessor(const String& var) {
  Serial.println("pageProcessor(...)");
  // LEFT OFF HERE> Need something like this.
  if (var == "FIRMWARE") {
    return FACTORY_FIRMWARE_VERSION;
  }

  if (var == "FREESPIFFS") {
    return humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes()));
  }

  if (var == "USEDSPIFFS") {
    return humanReadableSize(SPIFFS.usedBytes());
  }

  if (var == "TOTALSPIFFS") {
    return humanReadableSize(SPIFFS.totalBytes());
  }

  if (var == "IP_AP") {
    return WiFi.softAPIP().toString();
  }

  if (var == "IP_STA") {
    return WiFi.localIP().toString();
  }
  return String();
}

void initWiFi() {
  Serial.println("initWiFi()");

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(DEFAULT_BASE_AP_SSID, DEFAULT_BASE_AP_PASSWORD, WIFI_CHANNEL, false);
}

void initWebServer() {
  Serial.println("initWebServer()");

  initRoutes();
  _server.begin();
}

void initRoutes() {
  Serial.println("initRoutes()");

  _server.on("/",                HTTP_GET,  onGet_root);
  // _server.onFileUpload(                     on_fileUpload);

  // _server.on("/upload-file", HTTP_POST, []() {
  _server.on("/", HTTP_POST, []() {
    Serial.println("HTTP_POST: /");
    _server.send(200, "text/plain", "");
  }, handleFileUpload);

  _server.on("/list-files",      HTTP_GET,  onGet_listFiles);
  // _server.on("/update-firmware", HTTP_POST, onPost_firmwareUpdate, onPost_firmwareUpdater);

    _server.on("/update-firmware", HTTP_POST, []() {
      Serial.println("/update-firmware (1)");
      _server.sendHeader("Connection", "close");
      _server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      Serial.println("/update-firmware (2)");
      HTTPUpload& upload = _server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin()) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
      } else {
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
      }
    });

  _server.on("/reboot",          HTTP_GET,  onGet_reboot);
  _server.on("/logout",          HTTP_GET,  onGet_logout);  // Indicate user is now logged out.
  _server.on("/logged-out",      HTTP_GET,  onGet_loggedOut);  // Indicate user is now logged out.
  _server.onNotFound(                       onGet_404);
}

void handleFileUpload() {
  Serial.println("handleFileUpload()");

  HTTPUpload& upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println("UPLOAD_FILE_START");
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.println("UPLOAD_FILE_WRITE");
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.println("UPLOAD_FILE_END");
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}
/** Route Handlers */

void onGet_root() {
  Serial.println("onGet_root()");
  // LEFT OFF HERE> process the page before sending it out.

  // _server.send_P(200, "text/html", _admin_html, pageProcessor);
  _server.send_P(200, "text/html", _admin_html);
}

// // void on_fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
// void on_fileUpload() {
//   // LEFT OFF HERE> CONVERT to WebServer

//   Serial.println("task_net::onFileUpload(...)");
//   // String logmessage; // = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
//   // Serial.println(logmessage);

//   if (!index) {
//     // logmessage = "Upload Start: " + String(filename);
//     // open the file on first call and store the file handle in the request object
//     _tempFile = SPIFFS.open("/" + filename, "w");
//     Serial.printf("Upload Start: %s\n", filename.c_str());
//   }

//   if (len) {
//     // stream the incoming chunk to the opened file
//     _tempFile.write(data, len);
//     // logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
//     // Serial.println(logmessage);
//   }

//   if (final) {
//     // logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
//     // close the file handle as the upload is now done
//     _tempFile.close();
//     // Serial.println(logmessage);
//     Serial.println(F("Upload Complete."));
//     // request->redirect("/");
//   }
// }

void onGet_listFiles() {
  Serial.println("onGet_listFiles()");
  _server.send(200, "text/plain", listFiles(true));
}

String listFiles(bool ishtml) {
  String returnText = "";
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();

  if (ishtml) {
    returnText += "<table><tr><th align='left'>Name</th><th align='left'>Size</th><th></th><th></th></tr>";
  }

  while (foundfile) {

    if (ishtml) {
      returnText += "<tr align='left'><td>" + String(foundfile.name()) + "</td><td>" + humanReadableSize(foundfile.size()) + "</td>";
      // returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'download\')\">Download</button>";
      returnText += "<td><button onclick=\"downloadDeleteButton(\'" + String(foundfile.name()) + "\', \'delete\')\">Delete</button></tr>";

    } else {
      returnText += "File: " + String(foundfile.name()) + " Size: " + humanReadableSize(foundfile.size()) + "\n";
    }

    foundfile = root.openNextFile();
  }

  if (ishtml) {
    returnText += "</table>";
  }

  root.close();
  foundfile.close();
  return returnText;
}

// void onPost_firmwareUpdate() {
//    Serial.println("onPost_firmwareUpdate(...)");
//   // The request handler is triggered after the upload has finished... 
//   // Create the response, add header, and send response
//   // AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
//   AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
//     Update.hasError() ? _ota_fail_html : _ota_success_html);

//   response->addHeader("Connection", "close");
//   response->addHeader("Access-Control-Allow-Origin", "*");
//   request->send(response);
// }

// // void onPost_firmwareUpdater(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
// void onPost_firmwareUpdater() {
//   // LEFT OFF HERE> Convert

//   // Upload handler chunks in data.
//   if (!index) {
//     Serial.printf("UploadStart: %s\n", filename.c_str());
//     // calculate sketch space required for the update
//     uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
//     // _otaState = NET_JOB_STATUS::START;
//     // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);

//     // Start with max available size
//     if (!Update.begin(maxSketchSpace)) {
//       Update.printError(Serial);
//     }
//   }
  
//   // Write chunked data to the free sketch space
//   if (Update.write(data, len) != len) {
//     Update.printError(Serial);
//   }

//   // If the final flag is set then this is the last frame of data
//   if (final) {
//     // True to set the size to the current progress
//     if (Update.end(true)) {
//       // _otaState = NET_JOB_STATUS::COMPLETE;
//       // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
//       // _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);

//     } else {
//       // _otaState = NET_JOB_STATUS::ERROR;
//       Update.printError(Serial);
//       // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
//     }
//   }
// }

void onGet_reboot() {
  Serial.println("onGet_reboot(...)");
  _server.send(200, "text/html", reboot_html);
}

void onGet_logout() {
  Serial.println("onGet_logout(...)");
  _server.send(401);
};

void onGet_loggedOut() {
  Serial.println("onGet_loggedOut(...)");
  _server.send_P(200, "text/html", logout_html);
};

void onGet_404() {
  Serial.println("onGet_404(...)");
  _server.send(404, "text/plain", "404 Error - Page Not found");
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}
