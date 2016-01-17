#include "owm_weather.h"

typedef enum {
  OWMWeatherAppMessageKeyReply,
  OWMWeatherAppMessageKeyStop,
  OWMWeatherAppMessageKeyLine,
  OWMWeatherAppMessageKeyDestination,
  OWMWeatherAppMessageKeyDeparture,
  OWMWeatherAppMessageKeyBadKey = 91,
  OWMWeatherAppMessageKeyLocationUnavailable = 92
} OWMWeatherAppMessageKey;

static OWMWeatherInfo *s_info;
static OWMWeatherCallback *s_callback;
static OWMWeatherStatus s_status;

static void inbox_received_handler(DictionaryIterator *iter, void *context) {

  APP_LOG(APP_LOG_LEVEL_DEBUG, "In received handler");

  Tuple *reply_tuple = dict_find(iter, OWMWeatherAppMessageKeyReply);

  if(reply_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Reply received!");

    Tuple *stop_tuple = dict_find(iter, OWMWeatherAppMessageKeyStop);
    strncpy(s_info->stop, stop_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *line_tuple = dict_find(iter, OWMWeatherAppMessageKeyLine);
    strncpy(s_info->line, line_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *destination_tuple = dict_find(iter, OWMWeatherAppMessageKeyDestination);
    strncpy(s_info->destination, destination_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *departure_tuple = dict_find(iter, OWMWeatherAppMessageKeyDeparture);
    strncpy(s_info->departure, departure_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    // s_info->timestamp = time(NULL);

    s_status = OWMWeatherStatusAvailable;
    app_message_deregister_callbacks();
    s_callback(s_info, s_status);
  } else {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "No reply received!");
  }

  Tuple *err_tuple = dict_find(iter, OWMWeatherAppMessageKeyBadKey);
  if(err_tuple) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Bad key!");
    s_status = OWMWeatherStatusBadKey;
    s_callback(s_info, s_status);
  }

  err_tuple = dict_find(iter, OWMWeatherAppMessageKeyLocationUnavailable);
  if(err_tuple) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Location unavailable!");
    s_status = OWMWeatherStatusLocationUnavailable;
    s_callback(s_info, s_status);
  }
}

static void fail_and_callback() {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
  s_status = OWMWeatherStatusFailed;
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

  s_status = OWMWeatherStatusPending;
  s_callback(s_info, s_status);
  return true;
}

void owm_weather_init() {
  if(s_info) {
    free(s_info);
  }

  s_info = (OWMWeatherInfo*)malloc(sizeof(OWMWeatherInfo));
  s_status = OWMWeatherStatusNotYetFetched;
}

bool owm_weather_fetch(OWMWeatherCallback *callback) {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
    return false;
  }

  if(!callback) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWMWeatherCallback was NULL!");
    return false;
  }

  s_callback = callback;

  if(!bluetooth_connection_service_peek()) {
    s_status = OWMWeatherStatusBluetoothDisconnected;
    s_callback(s_info, s_status);
    return false;
  }

  app_message_deregister_callbacks();
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(2026, 656);

  return fetch();
}

void owm_weather_deinit() {
  if(s_info) {
    free(s_info);
    s_info = NULL;
    s_callback = NULL;
  }
}

OWMWeatherInfo* owm_weather_peek() {
  if(!s_info) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
    return NULL;
  }

  return s_info;
}
