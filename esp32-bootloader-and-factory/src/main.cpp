#include <SPIFFS.h>
#include <FS.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include "update-firmware-html.h"

// SSID and password of the AP (Access Point) server created on the ESP32.
#define DEFAULT_BASE_AP_SSID     "WROOM-FACTORY"
#define DEFAULT_BASE_AP_PASSWORD "wroomwroom"

WebServer server(80);
File fsUploadFile;

void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
bool exists(String path);

void setup() {
  delay(1000);
  Serial.begin(115200);
  while (!Serial);
  delay(1000);

  Serial.printf("\n\n\nStarting FACTORY App...\n\n");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(DEFAULT_BASE_AP_SSID, DEFAULT_BASE_AP_PASSWORD);

  Serial.printf("IP:        %s\n", WiFi.softAPIP().toString().c_str());
  Serial.printf("WiFi Name: %s\n", DEFAULT_BASE_AP_SSID);
  Serial.printf("WiFi Pwd:  %s\n\n", DEFAULT_BASE_AP_PASSWORD);

  /** 
   * HTTP: "/"
   */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send_P(200, "text/html", _updateFirmwareHtml);
  });

  /** 
   * HTTP: "/update-firmware"
   * OTA Firmware Update
   */
  server.on("/update-firmware", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {

    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());

      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
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

    }
  });

  server.on("/list", HTTP_GET, handleFileList);
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    // if (!handleFileRead(server.uri())) {
    server.send(404, "text/plain", "404 Error - Not Found");
    // }
  });  

  server.begin();
}


void loop(void) {
  server.handleClient();
  delay(1);
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    Serial.print("handleFileUpload Name: ");
    Serial.println(filename);

    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  Serial.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  Serial.println("handleFileList: " + path);


  File root = SPIFFS.open(path);
  path = String();

  String output = "[";
  if(root.isDirectory()){
      File file = root.openNextFile();
      while(file){
          if (output != "[") {
            output += ',';
          }
          output += "{\"type\":\"";
          output += (file.isDirectory()) ? "dir" : "file";
          output += "\",\"name\":\"";
          output += String(file.name()).substring(1);
          output += "\"}";
          file = root.openNextFile();
      }
  }
  output += "]";
  server.send(200, "text/json", output);
}

bool exists(String path) {
  bool yes = false;
  File file = SPIFFS.open(path, "r");
  if(!file.isDirectory()){
    yes = true;
  }
  file.close();
  return yes;
}
