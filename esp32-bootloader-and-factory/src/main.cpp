/**
 * @todo Ensure datafile uploads will fit in available space (SPIFFS)
 * @todo Ensure progress bar in datafile upload on web is accurate.
 */

#include <FS.h>
#include <SPIFFS.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include "webpages.h"

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
String* processAdminHtml(const String& var);
String listFiles(bool ishtml);
void   onGet_root();
void   on_fileUpload();
void   onGet_listFiles();
void   on_file();
void   onGet_reboot();
void   onGet_logout();
void   onGet_loggedOut();
void   onGet_404();

void handleFileUpload();
void handleFileDelete();
bool exists(String path);

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  Serial.println(F("setup()"));

  initSpiffs();
  initWiFi();
  initWebServer();
}

void loop() {
  _server.handleClient();
  // if (_reboot == true) {
  //   Serial.println(F("\n\n\n! Rebooting...\n\n\n"));
  //   delay(2000);
  //   ESP.restart();
  //   return;    
  // }
  delay(1);
}

void initSpiffs() {
  if (!SPIFFS.begin(true)) {
    Serial.println(F("Cannot mount SPIFFS volume."));
  } else {
    Serial.println(F("Mounted SPIFFS volume."));
  }
}

/** @todo Look into a better way than using 'String' to handle html templates and text replacement */
String* processAdminHtml(String *page) {
  Serial.println(F("processAdminHtml(...)"));

  page->replace("%FIRMWARE%", FACTORY_FIRMWARE_VERSION);
  page->replace("%FREESPIFFS%", humanReadableSize((SPIFFS.totalBytes() - SPIFFS.usedBytes())));
  page->replace("%USEDSPIFFS%", humanReadableSize(SPIFFS.usedBytes()));
  page->replace("%TOTALSPIFFS%", humanReadableSize(SPIFFS.totalBytes()));
  page->replace("%IP_AP%", WiFi.softAPIP().toString());
  page->replace("%IP_STA%", WiFi.localIP().toString());
  return page;
}

void initWiFi() {
  Serial.println(F("initWiFi()"));

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(DEFAULT_BASE_AP_SSID, DEFAULT_BASE_AP_PASSWORD, WIFI_CHANNEL, false);
}

void initWebServer() {
  Serial.println(F("initWebServer()"));

  initRoutes();
  _server.begin();
}

void initRoutes() {
  Serial.println(F("initRoutes()"));

  _server.on("/", HTTP_GET,  onGet_root);

  _server.on("/datafile", HTTP_POST, []() {
    Serial.println(F("HTTP_POST: /"));
    _server.send(200, "text/plain", "");
  }, handleFileUpload);

  _server.on("/datafile", HTTP_DELETE, handleFileDelete);

  _server.on("/list-files",      HTTP_GET,  onGet_listFiles);
    _server.on("/update-firmware", HTTP_POST, []() {
      Serial.println(F("/update-firmware (1)"));
      _server.sendHeader("Connection", "close");
      _server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    }, []() {
      Serial.println(F("/update-firmware (2)"));
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

  _server.on("/reboot",     HTTP_GET, onGet_reboot);
  _server.on("/logout",     HTTP_GET, onGet_logout);  // Indicate user is now logged out.
  _server.on("/logged-out", HTTP_GET, onGet_loggedOut);  // Indicate user is now logged out.
  _server.onNotFound(                 onGet_404);
}

void handleFileUpload() {
  Serial.println(F("handleFileUpload()"));

  HTTPUpload& upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.println(F("UPLOAD_FILE_START"));
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    Serial.println(F("UPLOAD_FILE_WRITE"));
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    Serial.println(F("UPLOAD_FILE_END"));
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print(F("handleFileUpload Size: "));
    Serial.println(upload.totalSize);
  }
}

bool exists(String path){
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}

void handleFileDelete() {
  Serial.println(F("handleFileDelete()"));

  if (_server.args() == 0) {
    return _server.send(500, "text/plain", "BAD ARGS");
  }
  String path = _server.arg(0);
  Serial.print(F("handleFileDelete: "));
  Serial.println(path);
  // if (path == "/") {
  //   return _server.send(500, "text/plain", "BAD PATH");
  // }
  if (!exists(path)) {
    return _server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  _server.send(200, "text/plain", "");
  path = String();
}
/** Route Handlers */

void onGet_root() {
  Serial.println(F("onGet_root()"));
  String adminPage = _admin_html;
  _server.send_P(200, "text/html", processAdminHtml(&adminPage)->c_str());
}

void onGet_listFiles() {
  Serial.println(F("onGet_listFiles()"));
  _server.send(200, "text/plain", listFiles(true));
}

String listFiles(bool ishtml) {
  Serial.println(F("listFiles(...)"));
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

void onGet_reboot() {
  Serial.println(F("onGet_reboot(...)2"));
  _server.send(200, "text/html", reboot_html);
  Serial.println(F("Rebooting..."));
  delay(2000);
  ESP.restart();
}

void onGet_logout() {
  Serial.println(F("onGet_logout(...)"));
  _server.send(401);
};

void onGet_loggedOut() {
  Serial.println(F("onGet_loggedOut(...)"));
  _server.send_P(200, "text/html", logout_html);
};

void onGet_404() {
  Serial.println(F("onGet_404(...)"));
  _server.send(404, "text/plain", "404 Error - Page Not found");
}

String humanReadableSize(const size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
  else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
}
