#include "task_network.h"
#include <webpages.h>

// LEFT OFF HERE> Get AP working with pretty URL
// LEFT OFF HERE> Get AP working without STA connecting.

namespace task_net {

  namespace {
 
    #define MAX_LEN_SSID_NAME 32
    #define MAX_LEN_SSID_PWD 64
    #define MAX_LEN_WEB_APP_URL 100

    /**
    *  Private
    ******************************************************************************************/
    /** Task */
    bool _isInitialized = false;

    /** Web Server */
    AsyncWebServer _server(80);
    String _stateAsTxt;

    /** WiFi */
    char _ipAddrTxt[16];
    IPAddress _ipAddr;
    // EzPzTimer _tmrWifiCheck;
    bool _hideServer;
    onNetStatusUpdateFn _onNetStatusUpdateCb;
    bool _apStarted                      = false;
    WIFI_SERVER_TYPE _srvType            = WIFI_SERVER_TYPE::AP_STA;
    uint8_t _macAddr[6]                  = { 0, 0, 0, 0, 0, 0 };
    char _websiteUsername[20]            = DEFAULT_WEBAPP_USERNAME;
    char _websitePassword[20]            = DEFAULT_WEBAPP_PASSWORD;
    char _apSsid[MAX_LEN_SSID_NAME]      = DEFAULT_BASE_AP_SSID;
    char _apPwd[MAX_LEN_SSID_PWD]        = DEFAULT_BASE_AP_PASSWORD;
    char _webAppUrl[MAX_LEN_WEB_APP_URL] = DEFAULT_WEBAPP_DOMAIN;

    IPAddress staticIp(0, 0, 0, 0);
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns;
    IPAddress emptyIp(0, 0, 0, 0);

    /** OTA Updating */
    NET_JOB_STATUS _otaState = NET_JOB_STATUS::UNKNOWN;

    /** Prototypes */
    void   initSpiffs();
    void   onWifiEvent(WiFiEvent_t event);
    void   setupWebServer();
    void   wifiConnectSta();
    void   startAsStandardBaseUnit();
    String humanReadableSize(const size_t bytes);
    String pageProcessor(const String& var);
    bool   checkUserWebAuth(AsyncWebServerRequest * request);
    String listFiles(bool ishtml);

    /** Private Functions */


    /******************************************************************************************
     * ROUTING: Start
     */

    /** Prototypes */

    void setupRoutes            ();
    void onGet_root             (AsyncWebServerRequest *request);
    void on_file                (AsyncWebServerRequest *request);
    void on_fileUpload          (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void onGet_listFiles        (AsyncWebServerRequest *request);
    void onPost_firmwareUpdate  (AsyncWebServerRequest *request);
    void onPost_firmwareUpdater (AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
    void onGet_reboot           (AsyncWebServerRequest *request);
    void onGet_logout           (AsyncWebServerRequest *request);
    void onGet_loggedOut        (AsyncWebServerRequest *request);
    void onGet_404              (AsyncWebServerRequest *request);

    /** Routing Calls */

    void setupRoutes() {
      // presents a "you are now logged out" webpage
      // _server.on("/",               HTTP_POST, onPost_root); // For ota update. Should fix.
      _server.on("/",                HTTP_GET,  onGet_root);
      _server.onFileUpload(                     on_fileUpload);
      _server.on("/list-files",      HTTP_GET,  onGet_listFiles);
      _server.on("/file",            HTTP_GET,  on_file);       // Download file
      // _server.on("/ota",             HTTP_GET,  onGet_ota);
      // _server.on("/ota-spiff",       HTTP_GET,  onGet_otaSpiff);
      _server.on("/update-firmware", HTTP_POST, onPost_firmwareUpdate, onPost_firmwareUpdater);
      _server.on("/reboot",          HTTP_GET,  onGet_reboot);
      _server.on("/logout",          HTTP_GET,  onGet_logout);  // Indicate user is now logged out.
      _server.on("/logged-out",      HTTP_GET,  onGet_loggedOut);  // Indicate user is now logged out.
      _server.onNotFound(                       onGet_404);
    }

    /** Route Handlers */

    void onGet_root(AsyncWebServerRequest * request) {
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());

      if (checkUserWebAuth(request)) {
        request->send_P(200, "text/html", _admin_html, pageProcessor);
      } else {
        return request->requestAuthentication();
      }
    }

    // void onPost_root(AsyncWebServerRequest * request) {
    //   lg.trace(F("task_net::HTTP_POST: '%s'\n"), request->url().c_str());

    //   if (checkUserWebAuth(request)) {
    //     request->send(200, "text/html", "POST ROOT");
    //   } else {
    //     return request->requestAuthentication();
    //   }
    // }

    void on_fileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      lg.trace(F("task_net::onFileUpload(...)\n"));
      // make sure authenticated before allowing upload
      if (checkUserWebAuth(request)) {
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
          lg.trace(F("Upload Complete.\n"));
          request->redirect("/");
        }
      } else {
        Serial.println("Auth: Failed");
        return request->requestAuthentication();
      }
    }

    void onGet_listFiles(AsyncWebServerRequest * request) {
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());

      if (checkUserWebAuth(request)) {
        request->send(200, "text/plain", listFiles(true));
      } else {
        return request->requestAuthentication();
      }
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
      lg.trace(F("task_net::on_file(...): '%s'\n"), request->url().c_str());

      if (checkUserWebAuth(request)) {

        if (request->hasParam("name") && request->hasParam("action")) {
          const char *fileName = request->getParam("name")->value().c_str();
          const char *fileAction = request->getParam("action")->value().c_str();

          if (!SPIFFS.exists(fileName)) {
            lg.verbose(F("ERROR: SPIFFS file requested does not exist.\n"));
            request->send(400, "text/plain", "ERROR: file does not exist");
          } else {

            if (strcmp(fileAction, "download") == 0) {
              lg.trace(F("task_net::on_file(...): Download"));
              request->send(SPIFFS, fileName, "application/octet-stream");

            } else if (strcmp(fileAction, "delete") == 0) {
              lg.trace(F("task_net::on_file(...): DELETE"));
              SPIFFS.remove(fileName);
              request->send(200, "text/plain", "Deleted File: " + String(fileName));

            } else {
              request->send(400, "text/plain", "ERROR: Invalid action param supplied");
            }
          }
        } else {
          request->send(400, "text/plain", "ERROR: Name and action params required");
        }
      } else {
        return request->requestAuthentication();
      }
    }

    // void onGet_ota(AsyncWebServerRequest *request) {
    //   lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());
    //   // request->send_P(200, "text/html", _otaFirmwareIndexHtml);
    // }

    // void onGet_otaSpiff(AsyncWebServerRequest *request) {
    //   lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());
    //   // request->send_P(200, "text/html", _otaFirmwareIndexHtml);
    // }

    void onPost_firmwareUpdate(AsyncWebServerRequest *request) {
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());
      // The request handler is triggered after the upload has finished... 
      // Create the response, add header, and send response
      // AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", Update.hasError() ? "FAIL" : "OK");
      AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html",
        Update.hasError() ? _ota_fail_html : _ota_success_html);

      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    }

    void onPost_firmwareUpdater(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {

      // Upload handler chunks in data.
      if (!index) {
        Serial.printf("UploadStart: %s\n", filename.c_str());
        // calculate sketch space required for the update
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        _otaState = NET_JOB_STATUS::START;
        _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);

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
          _otaState = NET_JOB_STATUS::COMPLETE;
          _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
          _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);

        } else {
          _otaState = NET_JOB_STATUS::ERROR;
          Update.printError(Serial);
          _onNetStatusUpdateCb(NET_JOB::OTA, _otaState);
        }
      }
    }

    // void onPost_firmwareUpdater(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
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
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());

      // String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
      // Serial.println(logmessage);

      if (checkUserWebAuth(request)) {
        request->send(200, "text/html", reboot_html);
        _onNetStatusUpdateCb(NET_JOB::REBOOT, NET_JOB_STATUS::UNKNOWN);
      } else {
        return request->requestAuthentication();
      }
    }

    void onGet_logout(AsyncWebServerRequest *request) {
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());
      request->send(401);
    };

    void onGet_loggedOut(AsyncWebServerRequest * request) {
      lg.trace(F("task_net::HTTP_GET: '%s'\n"), request->url().c_str());
      // String logmessage = "You are now logged out. Client:" + request->client()->remoteIP().toString() + " " + request->url();
      // Serial.println(logmessage);
      // request->send_P(401, "text/html", logout_html);
      request->send_P(200, "text/html", logout_html);
    };

    void onGet_404(AsyncWebServerRequest *request) {
      lg.trace(F("Page Not Found: %s: '%s'\n"), request->methodToString(), request->url().c_str());
      request->send(404, "text/plain", "404 Error - Page Not found");
    }

    /** ROUTING: End
    ******************************************************************************************/

    void initSpiffs() {
      if (!SPIFFS.begin(true)) {
        lg.error(F("Cannot mount SPIFFS volume.\n"));
      } else {
        lg.verbose(F("Mounted SPIFFS volume.\n"));
      }
    }

    void onWifiEvent(WiFiEvent_t event) {

      switch(event)
      {
      case SYSTEM_EVENT_STA_START:
        lg.verbose(F("task_network:: SYSTEM_EVENT_STA_START" CR));
        /** @todo not an accurate state but works for now **/
        _onNetStatusUpdateCb(NET_JOB::STATION, NET_JOB_STATUS::CONNECTED);
        break;

      case SYSTEM_EVENT_STA_DISCONNECTED:         /**< ESP32 station disconnected from AP */
        lg.verbose(F("task_network:: SYSTEM_EVENT_STA_DISCONNECTED" CR));
        _onNetStatusUpdateCb(NET_JOB::STATION, NET_JOB_STATUS::DISCONNECTED);
        break;

      case SYSTEM_EVENT_STA_GOT_IP: {              /**< ESP32 station got IP from connected AP */
        lg.verbose(F("task_network:: SYSTEM_EVENT_STA_GOT_IP" CR));
        lg.notice(F("Station IP: %s" CR), WiFi.localIP().toString().c_str());

        // // True if we're connecting for the first time in which case we
        // // got a dynamic IP address. Next we'll reconnect to that address
        // // statically.
        // if (staticIp == IPAddress(0, 0, 0, 0)) {
        //   lg.verbose(F("task_net:: Dynamic Connection.\n"));
        //   staticIp = WiFi.localIP();
        //   gateway = WiFi.gatewayIP();
        //   subnet = WiFi.subnetMask();
        //   dns = WiFi.dnsIP(0);
        //   // Reconnect statically with the IP address we were assigned.
        //   wifiConnectSta();
        // } else {
        //   lg.verbose(F("task_net:: Static Connection.\n"));
        // }

        lg.trace(F("\n\ntask_network:: \\o/ WiFi Station Connected!" CR));
        lg.trace(F("[WIFI] Local IP....: %s\n"), staticIp.toString().c_str());
        lg.trace(F("[WIFI] Subnet Mark.: %s\n"), subnet.toString().c_str());
        lg.trace(F("[WIFI] Gateway IP..: %s\n"), gateway.toString().c_str());
        lg.trace(F("[WIFI] DNS 1.......: %s\n"), dns.toString().c_str());
        lg.trace(F("[WIFI] DNS 2.......: %s\n"), WiFi.dnsIP(1).toString().c_str());
        lg.trace(F("[WIFI] SSID........: %s\n"), WiFi.SSID().c_str());
        lg.trace(F("[WIFI] BSSID.......: %s\n"), WiFi.BSSIDstr().c_str());
        lg.trace(F("[WIFI] MAC.........: %s\n"), WiFi.macAddress().c_str());
        lg.trace(F("[WIFI] Channel.....: %d\n\n"), WiFi.channel());

        _onNetStatusUpdateCb(NET_JOB::STATION, NET_JOB_STATUS::CONNECTED);

        break;
      }
      case SYSTEM_EVENT_STA_LOST_IP:              /**< ESP32 station lost IP and the IP is reset to 0 */
        lg.verbose(F("task_network:: SYSTEM_EVENT_STA_LOST_IP" CR));        // _obtainingIp = true;
        _onNetStatusUpdateCb(NET_JOB::STATION, NET_JOB_STATUS::DISCONNECTED);
        break;

      case SYSTEM_EVENT_AP_START:                 /**< ESP32 soft-AP start */
        if (_apStarted != true) {
          _apStarted = true;
          lg.verbose(F("task_network:: SYSTEM_EVENT_AP_START" CR));
          lg.notice(F("task_network:: Access Point IP: %s" CR), WiFi.softAPIP().toString().c_str());
          _onNetStatusUpdateCb(NET_JOB::ACCESS_POINT, NET_JOB_STATUS::CONNECTED);
          lg.notice(F("This Unit's MAC: %s" CR), WiFi.macAddress().c_str());
        }
        break;

      case SYSTEM_EVENT_AP_STOP:                  /**< ESP32 soft-AP stop */
        lg.verbose(F("task_network:: SYSTEM_EVENT_AP_STOP" CR));
        _apStarted = false;
        _onNetStatusUpdateCb(NET_JOB::ACCESS_POINT, NET_JOB_STATUS::DISCONNECTED);
        break;

      default:
        break;
      }
    }

    bool checkUserWebAuth(AsyncWebServerRequest * request) {
      bool isAuthenticated = false;

      if (request->authenticate(_websiteUsername, _websitePassword)) {
        lg.trace(F("\\o/ Authenticated.\n"));
        isAuthenticated = true;
      } else {
        lg.trace(F("!!! Authentication failed.\n"));
      }
      return isAuthenticated;
    }

    String humanReadableSize(const size_t bytes) {
      if (bytes < 1024) return String(bytes) + " B";
      else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
      else if (bytes < (1024 * 1024 * 1024)) return String(bytes / 1024.0 / 1024.0) + " MB";
      else return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
    }

    String pageProcessor(const String& var) {
      if (var == "BUILDNUM") {
        return APP_FIRMWARE_BUILD_NUM;
      }

      if (var == "FIRMWARE") {
        return APP_FIRMWARE_VERSION;
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
        return getAccessPointIp().toString();
      }

      if (var == "IP_STA") {
        return getStationIp().toString();
      }
      return String();
    }

    void setupWebServer() {
      lg.verbose(F("task_network::setupWebServer()\n\n"));

      if (_srvType == WIFI_SERVER_TYPE::UNKNOWN) {
        // Removing web server.
        _server.end();
        lg.verbose(F("task_network:: Disabled Web Server." CR));
      } else {

        /** HTTP_GET: '/'
        ******************************************************************************************/
        /** a.bin == index.html */
        setupRoutes();
        _server.begin();
      }
    }
    
    void wifiConnectSta() {
      lg.verbose(F("task_network::wifiConnectSta()" CR));

      Serial.printf("wifiConnectSta() getMacAddrStr() = %s\n", getMacAddrStr().c_str());

      if (WiFi.isConnected() == true) {
        lg.trace(F("task_network::wifiConnectSta() WiFi already connected. Disconnecting now." CR));
        // WiFi.disconnect();
      }

      WiFi.begin(STATION_SSID, STATION_PASSWORD, WIFI_CHANNEL);

      uint8_t connectTries = 10;
      while ((connectTries > 0) && (WiFi.status() != WL_CONNECTED)) {
        /** @todo Remove this delay and use a timer */
        delay(50);
        connectTries--;
      }
    }

    // LEFT OFF HERE> Put IP Addresses in a stats area on the web client.

    void startAsStandardBaseUnit() {
      lg.verbose(F("task_network::startAsStandardBaseUnit(...)" CR));
      reset();

      WiFi.onEvent(onWifiEvent);

      WiFi.mode(WIFI_AP_STA);
      WiFi.softAP(DEFAULT_BASE_AP_SSID, DEFAULT_BASE_AP_PASSWORD, WIFI_CHANNEL, false);
      wifiConnectSta();

      // if (WiFi.status() == WL_CONNECTED) {
      //   _onNetStatusUpdateCb(NET_JOB::STATION, NET_JOB_STATUS::CONNECTED);
      //   // _onNetStatusUpdateCb(NET_JOB::WIFI, NET_JOB_STATUS::CONNECTED);
      // }

      if (!MDNS.begin(_webAppUrl)) {
        lg.verbose(F("!o! mDNS Failed to start.\n"));
        return;
      } else {
        lg.verbose(F("\\o/ mDNS Started.\n"));
        MDNS.addService("http", "tcp", 80);
        //set hostname
        mdns_hostname_set(DEFAULT_WEBAPP_DOMAIN);
        //set default instance
        // mdns_instance_name_set("WROOM 32.");
      }

      setupWebServer();

      // _tmrWifiCheck.start(TMR_MS_WIFI_CHECK_SLOW);

      _isInitialized = true;
    }

  } /** End Private */





  /**
  *  Public
  ******************************************************************************************/

  /** 
   *  @todo Break this up.
   *  @todo Hide the server. Currently issue with sensor and base seeing eachother with that.
   *  @todo If the Sensor AP is hidden it can't connect to base after the base is turned off and the sensor is left on.
   */
  void setup(onNetStatusUpdateFn onNetStatusUpdateCb, bool hideServer) {
    lg.verbose(F("task_network::setup(...)\n"));
    _onNetStatusUpdateCb = onNetStatusUpdateCb;
    _hideServer = hideServer;
    initSpiffs();
    startAsStandardBaseUnit();
  }

  bool isInitialized() {
    return _isInitialized;
  }

  /**
   *  Public
   */

  /**
   *  Routines
   */

  void reset() {
    _isInitialized = false;
    // Stop WiFi
    WiFi.persistent(false);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    // _tmrWifiCheck.expire();
    // Stop the web server
    _server.end();

    /** @todo Look into removing this delay. May not need it and if we do, use a timer. */
    delay(100);
  }

  IPAddress getStationIp() {
    return WiFi.localIP();
  }

  IPAddress getAccessPointIp() {
    return WiFi.softAPIP();
  }

  bool isWifiConnected() {
    return WiFi.status() != WL_CONNECTED;
  }

  String getMacAddrStr() {
    return WiFi.macAddress();
  }

  uint8_t* getMacAddr() {
    WiFi.macAddress(_macAddr);
    return &_macAddr[0];
  }

  void getMacAddr(uint8_t* macAddr) {
    WiFi.macAddress(macAddr);
  }

  void saveIp(IPAddress ipAddr) {
    _ipAddr = ipAddr;

    if (ipAddr == IPAddress(0, 0, 0, 0)) {
      strcpy(_ipAddrTxt, "---.---.---.---");
    } else {
      sprintf(_ipAddrTxt, "...%d.%d.%d", _ipAddr[1], _ipAddr[2], _ipAddr[3]);
    }
  }

  int32_t getWifiChannel() {
    return WiFi.channel();
  }

  void setWebsiteUsername(char* username) {
    strncpy(&_websiteUsername[0], username, sizeof(_websiteUsername));
  }

  char* getWebsiteUsername() {
    return _websiteUsername;
  }

  void setWebsitePassword(char* password) {
    strncpy(&_websitePassword[0], password, sizeof(_websitePassword));
  }

  char* getWebsitePassword() {
    return _websitePassword;
  }

  void setApSsid(char* ssid) {
    strncpy(_apSsid, ssid, MAX_LEN_SSID_NAME);
  }

  char* getApSsid() {
    return &_apSsid[0];
  }

  void  setApPwd(char* pwd) {
    strncpy(_apPwd, pwd, MAX_LEN_SSID_PWD);
  }

  char* getApPwd() {
    return &_apPwd[0];
  }

  void setWebsiteUrl(char* url) {
    strncpy(_webAppUrl, url, MAX_LEN_WEB_APP_URL);
  }

  char* getWebsiteUrl() {
    return &_webAppUrl[0];
  }

}