#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Update.h>
#include "update-firmware-html.h"

// SSID and password of the AP (Access Point) server created on the ESP32.
#define DEFAULT_BASE_AP_SSID     "WROOM-FACTORY"
#define DEFAULT_BASE_AP_PASSWORD "wroomwroom"

WebServer server(80);

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

  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send_P(200, "text/html", _updateFirmwareHtml);
  });

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

  server.begin();
}

void loop(void) {
  server.handleClient();
  delay(1);
}