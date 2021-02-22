#include "common-network.h"

uint32_t genMsgId() {
  return millis();
}


bool compareMacAddr(const uint8_t macAddr1[], const uint8_t macAddr2[]) {
  bool equal = false;
  for (int n = 0; n < 6; n++) {
    if (macAddr1[n] == macAddr2[n]) {
      equal = true;
    } else {
      equal = false;
      break;
    }
  }
  return equal;
}


bool getMacAddr(uint8_t dest[]) {
  uint8_t currMacAddr[6];
  if (WiFi.macAddress(&currMacAddr[0]) == NULL) {
    uint8_t noMac[] = { 0, 0, 0, 0, 0 };
    memcpy(&(dest[0]), &(noMac[0]), 6);
    return false;
  } else {
    memcpy(&(dest[0]), &(currMacAddr[0]), 6);
    return true;
  }
}

// Expecting char macAddr[18] for first param.
char* macAddrToChar(char macAddrChar[], uint8_t addr[]) {
  // sprintf(&(macAddrChar[0]), "%2X:%2X:%2X:%2X:%2X:%2X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  sprintf(&(macAddrChar[0]), "%X:%X:%X:%X:%X:%X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return &(macAddrChar[0]);
}

uint8_t* cpymac(uint8_t dest[], const uint8_t pos1, const uint8_t pos2, const uint8_t pos3, const uint8_t pos4, const uint8_t pos5, const uint8_t pos6) {
  dest[0] = pos1;  
  dest[1] = pos2;  
  dest[2] = pos3;  
  dest[3] = pos4;  
  dest[4] = pos5;  
  dest[5] = pos6;  
  return &(dest[0]);
}

uint8_t* cpymac(uint8_t dest[], const uint8_t source[]) {
  return (uint8_t*)memcpy(&(dest[0]), &(source[0]), 6);
}

msg_t* cpymsg(msg_t& dest, const msg_t& source) {
  return (msg_t*)memcpy(&dest, &source, sizeof(dest));
};

void resetMsg(msg_t& msg) {
  msg       .hdr_msgId          = 0;
  msg       .hdr_msgType        = MSG_TYPE::UNKNOWN;
  msg       .hdr_originPeerType = PEER_TYPE::UNKNOWN;
  cpymac(msg.hdr_originMacAddr  , emptyMac);
  cpymac(msg.hdr_destMacAddr    , emptyMac);
  msg       .confirmRequired    = false;
  msg       .data[0]            = '\0';
  msg       .deliveryState      = MSG_DELIVERY_STATE::UNUSED;
}

msg_t* prepMsg(msg_t& msg, const uint8_t destMacAddr[], MSG_TYPE msgType, PEER_TYPE originPeerType, bool confirmRequired) {
  uint8_t ourMacAddr[6];
  getMacAddr(ourMacAddr);
  msg       .hdr_msgId          = genMsgId();
  msg       .hdr_msgType        = msgType;
  msg       .hdr_originPeerType = originPeerType;
  cpymac(msg.hdr_originMacAddr  , ourMacAddr);
  cpymac(msg.hdr_destMacAddr    , destMacAddr);
  msg       .deliveryState      = MSG_DELIVERY_STATE::OUT_AWAITING_BUFFERING;
  msg       .confirmRequired    = confirmRequired;
  msg       .data[0]            = '\0';
  return &msg;
}

/**
 * Tracing and Logging.
 */

void traceMacAddr(const char* text, const uint8_t macAddr[]) {
#ifndef DISABLE_LOGGING

  lg.trace(F("%s %x:%x:%x:%x:%x:%x" CR),
             text,
             macAddr[0], macAddr[1], macAddr[2],
             macAddr[3], macAddr[4], macAddr[5]);

#endif
}

const char* MSG_TYPE_asStr(MSG_TYPE msgType) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (msgType)
  {
  case MSG_TYPE::PAIR_REQUEST: return "PAIR_REQUEST"; break;
  case MSG_TYPE::PAIR_ACCEPT:  return "PAIR_ACCEPT";  break;
  case MSG_TYPE::PAIR_REJECT:  return "PAIR_REJECT";  break;
  case MSG_TYPE::PING:         return "PING";         break;
  case MSG_TYPE::PONG:         return "PONG";         break;
  case MSG_TYPE::SENSOR_DATA:  return "SENSOR_DATA";  break;
  case MSG_TYPE::UNKNOWN:      return "UNKNOWN";      break;
  case MSG_TYPE::UNPAIR:       return "UNPAIR";       break;
  default: return "NULL"; break;
  }

#endif
}

const char* SENSOR_PAIRING_STATE_asStr(SENSOR_PAIRING_STATE state) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (state)
  {
  case SENSOR_PAIRING_STATE::UNKNOWN:                            return "UNKNOWN";                            break;
  case SENSOR_PAIRING_STATE::REQUESTING_PAIRING:                 return "REQUESTING_PAIRING";                 break;
  case SENSOR_PAIRING_STATE::PAIRING_PROSPECT_CONTACTED:         return "PAIRING_PROSPECT_CONTACTED";         break;
  case SENSOR_PAIRING_STATE::AWAITING_PAIRING_PROSPECT_RESPONSE: return "AWAITING_PAIRING_PROSPECT_RESPONSE"; break;
  case SENSOR_PAIRING_STATE::PAIRING_REQUEST_ACCEPTED_BY_PEER:   return "PAIRING_REQUEST_ACCEPTED_BY_PEER";   break;
  case SENSOR_PAIRING_STATE::PAIRING_REQUEST_REJECTED_BY_PEER:   return "PAIRING_REQUEST_REJECTED_BY_PEER";   break;
  case SENSOR_PAIRING_STATE::PAIRED:                             return "PAIRED";                             break;
  case SENSOR_PAIRING_STATE::PAIRED_AND_ACTIVE:                  return "PAIRED_AND_ACTIVE";                  break;
  case SENSOR_PAIRING_STATE::UNPAIRED:                           return "UNPAIRED";                           break;
  case SENSOR_PAIRING_STATE::RETRY_PAIRING_REQUEST:              return "RETRY_PAIRING_REQUEST";              break;
  default: return "NULL"; break;
  }

#endif
}

const char* BASE_PAIRING_STATE_asStr(BASE_PAIRING_STATE state) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (state)
  {
  case BASE_PAIRING_STATE::UNKNOWN:                           return "UNKNOWN";                           break;
  case BASE_PAIRING_STATE::INCOMING_PAIRING_REQUEST:          return "INCOMING_PAIRING_REQUEST";          break;
  case BASE_PAIRING_STATE::REJECTED_INCOMING_PAIRING_REQUEST: return "REJECTED_INCOMING_PAIRING_REQUEST"; break;
  case BASE_PAIRING_STATE::PAIRED:                            return "PAIRED";                            break;
  case BASE_PAIRING_STATE::PAIRED_AND_ACTIVE:                 return "PAIRED_AND_ACTIVE";                 break;
  case BASE_PAIRING_STATE::UNPAIRED:                          return "UNPAIRED";                          break;
  default: return "NULL"; break;
  }

#endif
}

const char* MSG_DELIVERY_STATE_asStr(MSG_DELIVERY_STATE state) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (state)
  {
  case MSG_DELIVERY_STATE::UNKNOWN:                        return "UNKNOWN";
  case MSG_DELIVERY_STATE::UNUSED:                         return "UNUSED";
  case MSG_DELIVERY_STATE::OUT_AWAITING_BUFFERING:         return "OUT_AWAITING_BUFFERING";
  case MSG_DELIVERY_STATE::OUT_BUFFERED:                   return "OUT_BUFFERED";
  case MSG_DELIVERY_STATE::OUT_POPPED:                     return "OUT_POPPED";
  case MSG_DELIVERY_STATE::OUT_SEND_TRY:                   return "OUT_SEND_TRY";
  case MSG_DELIVERY_STATE::OUT_SEND_FAIL:                  return "OUT_SEND_FAIL";
  case MSG_DELIVERY_STATE::OUT_SEND_SUCCESS:               return "OUT_SEND_SUCCESS";
  case MSG_DELIVERY_STATE::OUT_CONFIRMATION_TIMEOUT:       return "OUT_CONFIRMATION_TIMEOUT";
  case MSG_DELIVERY_STATE::OUT_PEER_NOT_REGISTERED:        return "OUT_PEER_NOT_REGISTERED";
  case MSG_DELIVERY_STATE::OUT_AWAITING_PEER_CONFIRMATION: return "OUT_AWAITING_PEER_CONFIRMATION";
  case MSG_DELIVERY_STATE::OUT_CONFIRMED_BY_PEER:          return "OUT_CONFIRMED_BY_PEER";
  case MSG_DELIVERY_STATE::OUT_DONE:                       return "OUT_DONE";
  case MSG_DELIVERY_STATE::OUT_AWAITING_OUT_SEND_COMPLETE: return "OUT_AWAITING_OUT_SEND_COMPLETE";
  case MSG_DELIVERY_STATE::OUT_PAIRING_ACCEPTED_BY_PEER:   return "OUT_PAIRING_ACCEPTED_BY_PEER";
  case MSG_DELIVERY_STATE::OUT_PAIRING_REJECTED_BY_PEER:   return "OUT_PAIRING_REJECTED_BY_PEER";
  case MSG_DELIVERY_STATE::OUT_WRONG_CHANNEL:              return "OUT_WRONG_CHANNEL";
  case MSG_DELIVERY_STATE::IN_RECEIVED:                    return "IN_RECEIVED";
  case MSG_DELIVERY_STATE::IN_BUFFERED:                    return "IN_BUFFERED";
  case MSG_DELIVERY_STATE::IN_POPPED:                      return "IN_POPPED";
  case MSG_DELIVERY_STATE::IN_PROCESSING:                  return "IN_PROCESSING";
  case MSG_DELIVERY_STATE::IN_DONE:                        return "IN_DONE";
  case MSG_DELIVERY_STATE::IN_CONFIRM_REQUEST:             return "IN_CONFIRM_REQUEST";
  default: return "NULL"; break;
  }

#endif
}

const char* PEER_TYPE_asStr(PEER_TYPE type) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (type)
  {
  case PEER_TYPE::UNKNOWN:         return "UNKNOWN";
  case PEER_TYPE::BASE:            return "BASE";
  case PEER_TYPE::HEAT_HUMD_LIGHT: return "HEAT_HUMD_LIGHT";
  case PEER_TYPE::PAIRING:         return "PAIRING";
  default: return "NULL"; break;
  }

#endif
}


const char* PEER_STATE_asStr(PEER_STATE state) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (state)
  {
  case PEER_STATE::IDLE:                       return "IDLE";
  case PEER_STATE::AWAITING_SEND_COMPLETE:     return "AWAITING_SEND_COMPLETE";
  case PEER_STATE::AWAITING_PEER_CONFIRMATION: return "AWAITING_PEER_CONFIRMATION";
  case PEER_STATE::SENDING_MSG:                return "SENDING_MSG";
  case PEER_STATE::PAIRING_REQUESTED:          return "PAIRING_REQUESTED";
  default: return "NULL"; break;
  }

#endif
}

const char* SEND_MSG_RSLT_asStr(SEND_MSG_RSLT val) {
#ifdef DISABLE_LOGGING
  return NULL;
#else

  switch (val)
  {
  case SEND_MSG_RSLT::ERROR_NOT_INITIALIZED:                return "ERROR_NOT_INITIALIZED";
  case SEND_MSG_RSLT::BUFFERED:                             return "BUFFERED";
  case SEND_MSG_RSLT::ERROR_UNKNOWN_PEER_ADDR:              return "ERROR_UNKNOWN_PEER_ADDR";
  case SEND_MSG_RSLT::ERROR_NO_COMPATABLE_PEER:             return "ERROR_NO_COMPATABLE_PEER";
  case SEND_MSG_RSLT::ERROR_CURRENTLY_PAIRING:              return "ERROR_CURRENTLY_PAIRING";
  case SEND_MSG_RSLT::ERROR_PAIRING_MSG_ON_PRIMARY_CHANNEL: return "ERROR_PAIRING_MSG_ON_PRIMARY_CHANNEL";
  case SEND_MSG_RSLT::ERROR_PRIMARY_MSG_ON_PAIRING_CHANNEL: return "ERROR_PRIMARY_MSG_ON_PAIRING_CHANNEL";
  default: return "NULL"; break;
  }

#endif
}


void traceMsg(msg_t* msg, const char* prefix) {
  return;
#ifndef DISABLE_LOGGING

  if (lg.getLevel() < LOG_LEVEL_TRACE) {
    return;
  }

  // Make a copy so the msg doesn't get garbage
  // collected before we're done writing it out.
  // May not be required but playing safe for now.
  msg_t msgDup;
  cpymsg(msgDup, *msg);

  lg.verbose(F(
    "\n\n-- msg_t (%s) -----------------\n" \
    "     hdr_msgId............ %u\n" \
    "     hdr_msgType.......... %s\n" \
    "     hdr_originMacAddr.... %x:%x:%x:%x:%x:%x\n" \
    "     hdr_destMacAddr...... %x:%x:%x:%x:%x:%x\n" \
    "     hdr_originPeerType... %u\n" \
    "     responseToMsgId...... %u\n" \
    "     data................. %s\n" \
    "     confirmRequired...... %T\n" \
    "   --------------------------\n\n"
  ),  prefix == NULL ? "" : prefix,
      msgDup.hdr_msgId,
      MSG_TYPE_asStr(msgDup.hdr_msgType),
      msgDup.hdr_originMacAddr[0],
      msgDup.hdr_originMacAddr[1],
      msgDup.hdr_originMacAddr[2],
      msgDup.hdr_originMacAddr[3],
      msgDup.hdr_originMacAddr[4],
      msgDup.hdr_originMacAddr[5],
      msgDup.hdr_destMacAddr[0],
      msgDup.hdr_destMacAddr[1],
      msgDup.hdr_destMacAddr[2],
      msgDup.hdr_destMacAddr[3],
      msgDup.hdr_destMacAddr[4],
      msgDup.hdr_destMacAddr[5],
      PEER_TYPE_asStr(msgDup.hdr_originPeerType),
      msgDup.responseToMsgId,
      msgDup.data,
      msgDup.confirmRequired
  );
#endif // ifndef DISABLE_LOGGING
}