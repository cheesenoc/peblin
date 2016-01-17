#include "opendata_transport.h"

typedef enum {
  PeblinAppMessageKeyReply,
  PeblinAppMessageKeyStop,
  PeblinAppMessageKeyLine,
  PeblinAppMessageKeyDestination,
  PeblinAppMessageKeyDeparture,
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

    Tuple *stop_tuple = dict_find(iter, PeblinAppMessageKeyStop);
    strncpy(s_info->stop, stop_tuple->value->cstring, OPENDATA_TRANSPORT_BUFFER_SIZE);

    Tuple *line_tuple = dict_find(iter, PeblinAppMessageKeyLine);
    strncpy(s_info->line, line_tuple->value->cstring, OPENDATA_TRANSPORT_BUFFER_SIZE);

    Tuple *destination_tuple = dict_find(iter, PeblinAppMessageKeyDestination);
    strncpy(s_info->destination, destination_tuple->value->cstring, OPENDATA_TRANSPORT_BUFFER_SIZE);

    Tuple *departure_tuple = dict_find(iter, PeblinAppMessageKeyDeparture);
    // strncpy(s_info->departure, departure_tuple->value->int32, OPENDATA_TRANSPORT_BUFFER_SIZE);

    s_info->timestamp = (time_t)departure_tuple->value->int32;

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

static bool fetch() {
  DictionaryIterator *out;
  AppMessageResult result = app_message_outbox_begin(&out);
  if(result != APP_MSG_OK) {
    fail_and_callback();
    return false;
  }

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

bool opendata_transport_fetch(OpendataTransportCallback *callback) {
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

  return fetch();
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
