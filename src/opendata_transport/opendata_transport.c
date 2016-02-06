#include "opendata_transport.h"

typedef enum {
  PeblinAppMessageKeyReply,
  PeblinAppMessageKeyStops,
  PeblinAppMessageKeyTimes,
  PeblinAppMessageKeyBadLocationsUrl = 90,
  PeblinAppMessageKeyBadStationboardUrl = 91,
  PeblinAppMessageKeyLocationUnavailable = 92
} PeblinAppMessageKey;

static OpendataTransportInfo *s_info;
static OpendataTransportCallback *s_callback;
static OpendataTransportStatus s_status;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received handler");

  Tuple *reply_tuple = dict_find(iter, PeblinAppMessageKeyReply);

  if(reply_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Reply received!");

    Tuple *stops_tuple = dict_find(iter, PeblinAppMessageKeyStops);
    strncpy(s_info->stops, stops_tuple->value->cstring, OPENDATA_TRANSPORT_BUFFER_SIZE);

    Tuple *times_tuple = dict_find(iter, PeblinAppMessageKeyTimes);
    strncpy(s_info->times, times_tuple->value->cstring, OPENDATA_TRANSPORT_BUFFER_SIZE);

    s_status = OpendataTransportStatusAvailable;
    app_message_deregister_callbacks();
    s_callback(s_info, s_status);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No reply received!");
  }

  Tuple *err_tuple = dict_find(iter, PeblinAppMessageKeyBadLocationsUrl);
  if(err_tuple) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Bad location url!");
    s_status = OpendataTransportStatusBadLocationsUrl;
    s_callback(s_info, s_status);
  }

  err_tuple = dict_find(iter, PeblinAppMessageKeyBadStationboardUrl);
  if(err_tuple) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Bad stationboard url!");
    s_status = OpendataTransportStatusBadStationboardUrl;
    s_callback(s_info, s_status);
  }

  err_tuple = dict_find(iter, PeblinAppMessageKeyLocationUnavailable);
  if(err_tuple) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Location unavailable!");
    s_status = OpendataTransportStatusLocationUnavailable;
    s_callback(s_info, s_status);
  }
}

static void fail_and_callback() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
  s_status = OpendataTransportStatusFailed;
  s_callback(s_info, s_status);
}

static bool fetch(int stop) {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  int value = stop;
  dict_write_int(out, 1, &value, sizeof(int), true);
  dict_write_end(out);

  result = app_message_outbox_send();
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

  s_status = OpendataTransportStatusPending;
  s_callback(s_info, s_status);
  return true;
}

void opendata_transport_init() {
  if(s_info) {
    free(s_info);
  }

  s_info = (OpendataTransportInfo*)malloc(sizeof(OpendataTransportInfo));
  s_status = OpendataTransportStatusNotYetFetched;
}

bool opendata_transport_fetch(int stop, OpendataTransportCallback *callback) {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OpendataTransport library is not initialized!");
    return false;
  }

  if(!callback) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OpendataTransportCallback was NULL!");
    return false;
  }

  s_callback = callback;

  if(!bluetooth_connection_service_peek()) {
    s_status = OpendataTransportStatusBluetoothDisconnected;
    s_callback(s_info, s_status);
    return false;
  }

  app_message_deregister_callbacks();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);

  return fetch(stop);
}

void opendata_transport_deinit() {
  if(s_info) {
    free(s_info);
    s_info = NULL;
    s_callback = NULL;
  }
}

OpendataTransportInfo* opendata_transport_peek() {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OpendataTransport library is not initialized!");
    return NULL;
  }

  return s_info;
}
