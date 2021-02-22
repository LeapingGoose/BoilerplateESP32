#include <SPIFFS.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
// #include <WebServer.h>
#include <Update.h>
#include "webpages.h"
// #include "update-firmware-html.h"

// SSID and password of the AP (Access Point) server created on the ESP32.
#define DEFAULT_BASE_AP_SSID     "WROOM-FACTORY"
#define DEFAULT_BASE_AP_PASSWORD "wroomwroom"
#define WIFI_CHANNEL             6
#define FACTORY_FIRMWARE_VERSION "0.1"
AsyncWebServer _server(80);

void   initSpiffs();
void   initWiFi();
void   initWebServer();
void   initRoutes();
void   onWifiEvent(WiFiEvent_t event);
String humanReadableSize(const size_t bytes);
String pageProcessor(const String& var);
String listFiles(bool ishtml);

void onGet_root        (AsyncWebServerRequest *request);
void on_fileUpload     (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void onGet_listFiles   (AsyncWebServerRequest *request);
void on_file           (AsyncWebServerRequest *request);
void onGet_ota         (AsyncWebServerRequest *request);
void onGet_otaSpiff    (AsyncWebServerRequest *request);
void onPost_otaUpdate  (AsyncWebServerRequest *request);
void onPost_otaUpdater (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void onGet_reboot      (AsyncWebServerRequest *request);
void onGet_logout      (AsyncWebServerRequest *request);
void onGet_loggedOut   (AsyncWebServerRequest *request);
void onGet_404         (AsyncWebServerRequest *request);


void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  initSpiffs();
  initWiFi();
  initWebServer();
}

void loop(void) {
  // _server.handleClient();
  // delay(1);
}

void initSpiffs() {
  if (!SPIFFS.begin(true)) {
    Serial.println("Cannot mount SPIFFS volume.");
  } else {
    Serial.println("Mounted SPIFFS volume.");
  }
}
String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}

String pageProcessor(const String& var) {
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
  initRoutes();
  _server.begin();
}

void initRoutes() {
  _server.on("/",                HTTP_GET,  onGet_root);
  _server.onFileUpload(                     on_fileUpload);
  _server.on("/list-files",      HTTP_GET,  onGet_listFiles);
  _server.on("/file",            HTTP_GET,  on_file);       // Download file
  _server.on("/ota",             HTTP_GET,  onGet_ota);
  _server.on("/ota-spiff",       HTTP_GET,  onGet_otaSpiff);
  _server.on("/update-firmware", HTTP_POST, onPost_otaUpdate, onPost_otaUpdater);
  _server.on("/reboot",          HTTP_GET,  onGet_reboot);
  _server.on("/logout",          HTTP_GET,  onGet_logout);  // Indicate user is now logged out.
  _server.on("/logged-out",      HTTP_GET,  onGet_loggedOut);  // Indicate user is now logged out.
  _server.onNotFound(                       onGet_404);
}

/** Route Handlers */

void onGet_root(AsyncWebServerRequest * request) {
  Serial.println("task_net::HTTP_GET: '\'\n");
  request->send_P(200, "text/html", _admin_html, pageProcessor);
}

void on_fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  Serial.println("task_net::onFileUpload(...)");
  // String logmessage; // = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  // Serial.println(logmessage);

  if (!index) {
    // logmessage = "Upload Start: " + String(filename);
    // open the file on first call and store the file handle in the request object
    request->_tempFile = SPIFFS.open("/" + filename, "w");
    Serial.printf("Upload Start: %s\n", filename.c_str());
  }

  if (len) {
    // stream the incoming chunk to the opened file
    request->_tempFile.write(data, len);
    // logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
    // Serial.println(logmessage);
  }

  if (final) {
    // logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
    // close the file handle as the upload is now done
    request->_tempFile.close();
    // Serial.println(logmessage);
    Serial.println(F("Upload Complete."));
    request->redirect("/");
  }
}

void onGet_listFiles(AsyncWebServerRequest * request) {
  Serial.println("task_net::HTTP_GET: '/list-files'");
  request->send(200, "text/plain", listFiles(true));
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

// Download file
void on_file(AsyncWebServerRequest * request) {
  Serial.println("task_net::HTTP_GET: '/file'");

  if (request->hasParam("name") && request->hasParam("action")) {
    const char *fileName = request->getParam("name")->value().c_str();
    const char *fileAction = request->getParam("action")->value().c_str();

    if (!SPIFFS.exists(fileName)) {
      Serial.println("ERROR: SPIFFS file requested does not exist.");
      request->send(400, "text/plain", "ERROR: file does not exist");
    } else {

      if (strcmp(fileAction, "download") == 0) {
        Serial.println("task_net::on_file(...): Download");
        request->send(SPIFFS, fileName, "application/octet-stream");

      } else if (strcmp(fileAction, "delete") == 0) {
        Serial.println("task_net::on_file(...): DELETE");
        SPIFFS.remove(fileName);
        request->send(200, "text/plain", "Deleted File: " + String(fileName));

      } else {
        request->send(400, "text/plain", "ERROR: Invalid action param supplied");
      }
    }
  } else {
    request->send(400, "text/plain", "ERROR: Name and action params required");
  }
}

void onGet_ota(AsyncWebServerRequest *request) {
  Serial.println("onGet_ota(...)");
  // request->send_P(200, "text/html", _otaFirmwareIndexHtml);
}

void onGet_otaSpiff(AsyncWebServerRequest *request) {
  Serial.println("onGet_otaSpiff");
  // request->send_P(200, "text/html", _otaFirmwareIndexHtml);
}

void onPost_otaUpdate(AsyncWebServerRequest *request) {
   Serial.println("onPost_otaUpdate(...)");
  // The request handler is triggered after the upload has finished... 
  // Create the response, add header, and send response
  // AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
    Update.hasError() ? _ota_fail_html : _ota_success_html);

  response->addHeader("Connection", "close");
  response->addHeader("Access-Control-Allow-Origin", "*");
  request->send(response);
}

void onPost_otaUpdater(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

  // Upload handler chunks in data.
  if (!index) {
    Serial.printf("UploadStart: %s\n", filename.c_str());
    // calculate sketch space required for the update
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    // _otaState = NET_JOB_STATUS::START;
    // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);

    // Start with max available size
    if (!Update.begin(maxSketchSpace)) {
      Update.printError(Serial);
    }
  }
  
  // Write chunked data to the free sketch space
  if (Update.write(data, len) != len) {
    Update.printError(Serial);
  }

  // If the final flag is set then this is the last frame of data
  if (final) {
    // True to set the size to the current progress
    if (Update.end(true)) {
      // _otaState = NET_JOB_STATUS::COMPLETE;
      // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
      // _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);

    } else {
      // _otaState = NET_JOB_STATUS::ERROR;
      Update.printError(Serial);
      // _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
    }
  }
}

// void onPost_otaUpdater(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
//   // Upload handler chunks in data.
//   if (!index) {
//     Serial.printf("UploadStart: %s\n", filename.c_str());
//     // calculate sketch space required for the update
//     uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
//     _otaState = NET_JOB_STATUS::START;
//     _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);

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
//       _otaState = NET_JOB_STATUS::COMPLETE;
//       _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
//       _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);

//     } else {
//       _otaState = NET_JOB_STATUS::ERROR;
//       Update.printError(Serial);
//       _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
//     }
//   }
// }

void onGet_reboot(AsyncWebServerRequest * request) {
  Serial.println("onGet_reboot(...)");

  request->send(200, "text/html", reboot_html);
  // _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);
}

void onGet_logout(AsyncWebServerRequest *request) {
  Serial.println("onGet_logout(...)");
  request->send(401);
};

void onGet_loggedOut(AsyncWebServerRequest * request) {
  Serial.println("onGet_loggedOut(...)");
  // String logmessage = "You are now logged out. Client:" + request->client()->remoteIP().toString() + " " + request->url();
  // Serial.println(logmessage);
  // request->send_P(401, "text/html", logout_html);
  request->send_P(200, "text/html", logout_html);
};

void onGet_404(AsyncWebServerRequest *request) {
  Serial.println("onGet_404(...)");
  request->send(404, "text/plain", "404 Error - Page Not found");
}