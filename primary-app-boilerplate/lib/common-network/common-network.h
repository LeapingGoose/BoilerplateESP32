#ifndef COMMON_NETWORK_H
#define COMMON_NETWORK_H

#include <Arduino.h>
#include <stdint.h>
#include <ArduinoLog.h>
#include <esp_now.h>
#include <WiFi.h>
#include <secret.h>

/**************************************************************************************************
 *  WiFi
 */

enum class WIFI_SERVER_TYPE:uint8_t {
  UNKNOWN      = 1,
  STATION      = 2,
  ACCESS_POINT = 4,
  AP_STA       = 8
};

enum class NET_JOB:uint8_t {
  UNKNOWN,
  WIFI,
  ACCESS_POINT,
  STATION,
  OTA,
  REBOOT
};

enum class NET_JOB_STATUS:uint8_t {
  UNKNOWN,
  READY,
  START,
  ACTIVE,
  COMPLETE,
  CONNECTED,
  DISCONNECTED,
  ERROR
};

/*
 *  End WiFi
 *************************************************************************************************/


/**************************************************************************************************
 *  Messaging
 */

#define PEER_BUF_SIZE         3
#define MSG_BUF_SIZE          10
#define MSG_IN_BUFF_SIZE      10
#define MSG_OUT_BUFF_SIZE     10
// We should really only need one element since we only ever
// send one message at a time and then wait for confirmation.
#define MSG_OUT_COMPLETE_SIZE 10
#define MSG_SEND_BUFF_SIZE    10
// Max size, in bytes, of the json packets being transmitted via mesh.
// 250 is the maximum size of packets used with ESP-Now so the total below can't exceed that
#define MSG_HDR_SIZE                6    // Message details
#define MSG_DATA_SIZE               150  // Message data size as text
#define MSG_SIZE                    (MSG_HDR_SIZE + MSG_DATA_SIZE)
#define WIFI_CHANNEL                6
#define PAIRING_CHANNEL             2

#define HIGHEST_CHANNEL	            11
#define LOWEST_CHANNEL              1

#define ESPNOW_CHANNEL              WIFI_CHANNEL
#define ACCEPT_PAIRING_TIMEOUT_MS   10000
#define PAIR_MODE_TIMEOUT_MS        10000
#define WAIT_FOR_CONFIRM_TIMEOUT_MS 10000

#define WEBSITE_USERNAME "admin"
#define WEBSITE_PASSWORD "growit"

#define MsgOutBuf             etl::list<msg_t,              MSG_OUT_BUFF_SIZE>
#define MsgSendBuf            etl::list<msg_t,              MSG_SEND_BUFF_SIZE>
#define MsgOutRcvdBuf         etl::list<msg_out_complete_t, MSG_OUT_BUFF_SIZE>
#define MsgInBuf              etl::list<msg_t,              MSG_IN_BUFF_SIZE>
#define msg_buf_t             etl::list<msg_t,              MSG_IN_BUFF_SIZE>
#define msg_buf_out_complete  etl::list<bool,               MSG_OUT_COMPLETE_SIZE>
#define msg_buf_itr_t         etl::ilist<msg_t>::iterator

#define MAC_ADDR_STR_LEN 18
const uint8_t emptyMac[6] = { 0, 0, 0, 0, 0, 0 };

/** @note TESTING ONLY: START **/
const uint8_t _baseMacAddr_TMP[6]    = { 0xFC, 0xF5, 0xC4, 0x2F, 0x53, 0x74 };
const uint8_t _sensorMacAddr_TMP[6]  = { 0xFC, 0xF5, 0xC4, 0x2F, 0x54, 0xD4 };
const uint8_t _sensor2MacAddr_TMP[6] = { 0x24, 0x62, 0xAB, 0xE1, 0x28, 0xD8 };
/** @note TESTING ONLY: END **/

const uint8_t _basePairingMacAddr[6] = { 0xF2, 0xAB, 0xCD, 0xEF, 0xBA, 0xDC };

// enum class MSG_COMPLETE_STATUS:uint32_t {
//   SUCCESS               = 1,
//   SEND_FAILED           = 2,
//   UNPAIR                = 4,
//   PAIR_REQUEST_TIMEOUT  = 8,
//   PAIR_REQUEST_ACCEPT   = 16,
//   PAIR_REQUEST_REJECT   = 32,
//   CONFIRMATION_TIMEOUT  = 64,
// };

enum class MSG_TYPE:uint8_t {
  UNKNOWN = 1,
  PAIR_REQUEST,
  PAIR_ACCEPT,
  PAIR_REJECT,
  UNPAIR,
  PING,
  PONG,
  SENSOR_DATA,
};
const char* MSG_TYPE_asStr(MSG_TYPE msgType);

/** @todo Clean these up. */
enum class SEND_MSG_RSLT:uint8_t {
  ERROR_NOT_INITIALIZED                 = 1,
  BUFFERED                              = 2,
  ERROR_UNKNOWN_PEER_ADDR               = 3,
  ERROR_NO_COMPATABLE_PEER              = 4,
  ERROR_CURRENTLY_PAIRING               = 5,
  ERROR_PAIRING_MSG_ON_PRIMARY_CHANNEL  = 6,
  ERROR_PRIMARY_MSG_ON_PAIRING_CHANNEL  = 7,
};
const char* SEND_MSG_RSLT_asStr(SEND_MSG_RSLT val);


enum SENSOR_PAIRING_STATE:uint16_t {
  UNKNOWN                               = 1,
  PAIRED                                = 64,
  PAIRED_AND_ACTIVE                     = 128,
  UNPAIRED                              = 256,
  REQUESTING_PAIRING                    = 2,
  PAIRING_PROSPECT_CONTACTED            = 4,
  AWAITING_PAIRING_PROSPECT_RESPONSE    = 8,
  PAIRING_REQUEST_ACCEPTED_BY_PEER      = 16,
  PAIRING_REQUEST_REJECTED_BY_PEER      = 32,
  RETRY_PAIRING_REQUEST                 = 512,
};
const char* SENSOR_PAIRING_STATE_asStr(SENSOR_PAIRING_STATE state);

enum class BASE_PAIRING_STATE:uint16_t {
  UNKNOWN                               = 1,
  PAIRED                                = 2,
  PAIRED_AND_ACTIVE                     = 4,
  UNPAIRED                              = 8,
  INCOMING_PAIRING_REQUEST              = 16,
  REJECTED_INCOMING_PAIRING_REQUEST     = 32,
};
const char* BASE_PAIRING_STATE_asStr(BASE_PAIRING_STATE state);

enum class PAIRING_STATE {
  UNKNOWN     = 1,
  IN_PROGRESS = 2,
  PAIRED      = 4,
  UNPAIRED    = 8,
  TIMEOUT     = 16,
  SEARCHING   = 32,
};

enum class PAIRING_REQUEST_RESULT:uint8_t {
  ALREADY_PAIRING  = 1,
  INVALID_MAC_ADDR = 2,
  DISPATCHED       = 4,
  SEND_FAILED      = 8,
};


enum class PEER_TYPE:uint8_t {
  UNKNOWN          = 1,
  BASE             = 2,
  HEAT_HUMD_LIGHT  = 4,
  PAIRING          = 8,
};
const char* PEER_TYPE_asStr(PEER_TYPE type);


enum PEER_STATE:uint8_t {
  IDLE = 0,
  AWAITING_SEND_COMPLETE = 1,
  AWAITING_PEER_CONFIRMATION = 2, // After successful send of message requiring confirmation
  SENDING_MSG = 3,
  PAIRING_REQUESTED = 4,
};
const char* PEER_STATE_asStr(PEER_STATE state);


enum class MSG_DELIVERY_STATE:uint8_t {
  UNKNOWN,
  UNUSED,
  OUT_AWAITING_BUFFERING,          // Waiting to be put in the buffer
  OUT_BUFFERED,                    // The message has been put into a FIFO buffer and is awaiting processing
  OUT_POPPED,                      // The message has been removed from the buffer to be sent.
  OUT_SEND_TRY,                    // When we pass the message to the ESP-Now library for the message to be sent
  OUT_SEND_FAIL,                   // When we get confirmation from ESP-Now that the previously sent message was received.
  OUT_SEND_SUCCESS,                // The message has reached the sender.
  OUT_CONFIRMATION_TIMEOUT,        // We timed out while waiting for a confirmation message from a peer.
  OUT_PEER_NOT_REGISTERED,         // The Peer has not been registered with the ESP-Now library.
  OUT_AWAITING_PEER_CONFIRMATION,  // After successful send of message requiring confirmation
  OUT_CONFIRMED_BY_PEER,           // We received confirmation from the peer 
  OUT_DONE,                        // The message has reached for the sender.
  OUT_AWAITING_OUT_SEND_COMPLETE,
  OUT_PAIRING_ACCEPTED_BY_PEER,
  OUT_PAIRING_REJECTED_BY_PEER,
  OUT_WRONG_CHANNEL,               // When attempting to send a msg on the wrong channel (i.e. Pairing request on primary channel)

  IN_RECEIVED,
  IN_BUFFERED,
  IN_POPPED,
  IN_PROCESSING,
  IN_DONE,
  IN_CONFIRM_REQUEST,
};
const char* MSG_DELIVERY_STATE_asStr(MSG_DELIVERY_STATE state);

typedef struct {
  uint8_t               macAddr[6] = { 0, 0, 0, 0, 0, 0 };
  esp_now_send_status_t status;
} msg_out_complete_t;


// Anywhere we're not setting NODE_TYPE is because
// we can only perform those requests when we're
// paired with the master device and it will
// already know what our NODE_TYPE is.
typedef struct {
  uint32_t           hdr_msgId            = 0;                           // 4 bytes
  MSG_TYPE           hdr_msgType          = MSG_TYPE::UNKNOWN;           // 1 byte
  PEER_TYPE          hdr_originPeerType   = PEER_TYPE::UNKNOWN;          // 1 byte
  uint8_t            hdr_originMacAddr[6] = { 0, 0, 0, 0, 0, 0 };        // 6 bytes
  uint8_t            hdr_destMacAddr[6]   = { 0, 0, 0, 0, 0, 0 };        // 6 bytes
  MSG_DELIVERY_STATE deliveryState        = MSG_DELIVERY_STATE::UNKNOWN; // 1 byte
  uint32_t           responseToMsgId      = 0;                           // 4 bytes
  bool               confirmRequired      = false;                       // 1 byte
  char               data[MSG_DATA_SIZE]  = "";                          // 150 bytes
  // 250 bytes max - 150 - 25 = 75 bytes remaining.
} msg_t;

typedef bool (*msg_predicate_fn_t)  (msg_t& msg);
typedef bool (*msg_predicate2_fn_t) (msg_t& msg, uint8_t compareMacAddr[]);
typedef void (*onNetStatusUpdateFn) (NET_JOB, NET_JOB_STATUS);
typedef void (*onSensorDataFn)      (float tmpr, float humd, bool lightsOn, uint8_t macAddr[]);
typedef void (*onPairReqResFn)      (bool accepted, uint8_t macAddr[]);
typedef void (*onMsgOutCompleteFn)  (msg_t* msg);
// confirmResponse is only used when "confirmRequired" is true in msg_t and is to be set by the
// callee and then sent back to the originating peer of the message. Otherwise it can be ignored.
typedef void (*onMsgInFn)           (msg_t* msgIn);
typedef void (*onMsgSendFn)         (msg_t* msg);
typedef void (*onSensorPairingFn)   (SENSOR_PAIRING_STATE pairingState, const uint8_t* peerMacAddr);
typedef void (*onBasePairingFn)     (BASE_PAIRING_STATE pairingState, const uint8_t* peerMacAddr);

uint32_t    genMsgId();
bool        compareMacAddr(const uint8_t macAddr1[], const uint8_t macAddr2[]);
bool        getMacAddr(const uint8_t dest[]);
char*       macAddrToChar(char macAddrChar[], uint8_t macAddr[]);
uint8_t*    cpymac(uint8_t dest[], const uint8_t pos1, const uint8_t pos2, const uint8_t pos3, const uint8_t pos4, const uint8_t pos5, const uint8_t pos6);
uint8_t*    cpymac(uint8_t dest[], const uint8_t source[]);
msg_t*      cpymsg(msg_t& dest, const msg_t& source);

void        traceMacAddr(const char* text, const uint8_t macAddr[]);

void        traceMsg(msg_t* msg, const char* prefix = NULL);
void        resetMsg(msg_t& msg);

msg_t*      prepMsg(msg_t& msg, const uint8_t destMacAddr[], MSG_TYPE msgType = MSG_TYPE::UNKNOWN, PEER_TYPE originPeerType = PEER_TYPE::UNKNOWN, bool confirmRequired = false);

/*
 *  End Messaging
 *************************************************************************************************/

#endif