#ifndef TASK_NETWORK_H
#define TASK_NETWORK_H

#define FIRMWARE_VERSION "0.4"

#include <Arduino.h>
#include <SPIFFS.h>
#include <ArduinoLog.h>
#include <IPAddress.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EzPzTimer.h>
#include <common-network.h>
#include <WiFi.h>
#include <stdint.h>
#include <Update.h>
#include <ESPmDNS.h>

enum ParseResult: uint8_t {
  PARSED_OK = 0,
  PARSE_FAILED,
  NOTHING_CHANGED,
};

/**
 *  This network task takes care of WiFi, Mesh and Web Server.
 */
typedef void (*onSocketInFn) (AsyncWebSocketClient* client, AwsEventType type, const char* data);
typedef String (*pageProcessorFn) (const String& var);

namespace task_net {

  #define TMR_MS_WIFI_CHECK_SLOW  10000 // Used to idly check for a wifi connection.

  /**
   * General
   ***********************************************************************************************/
  void           setup(onNetStatusUpdateFn onNetStatusUpdateCb, bool hideServer);
  bool           isInitialized();
  void           reset();

  /**
   * MAC Address
   */
  String         getMacAddrStr();
  uint8_t*       getMacAddr();
  void           getMacAddr(uint8_t* macAddr);

  /**
   * WiFi
  */
  bool           isWifiConnected();
  bool           isWiFiObtainingIp();
  void           saveIp(IPAddress ipAddr);
  IPAddress      getStationIp();
  IPAddress      getAccessPointIp();
  int32_t        getWifiChannel();
  void           setApSsid(char* ssid);
  char*          getApSsid();
  void           setApPwd(char* pwd);
  char*          getApPwd();

  /**
   * Website
   */
  void           setWebsiteUsername(char* username);
  char*          getWebsiteUsername();
  void           setWebsitePassword(char* password);
  char*          getWebsitePassword();
  void           setWebsiteUrl(char* url);
  char*          getWebsiteUrl();

}

#endif