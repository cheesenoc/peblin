#include <pebble.h>

#include "opendata_transport/opendata_transport.h"

static Window *s_window;
static TextLayer *s_text_layer;

static void opendata_transport_callback(OpendataTransportInfo *info, OpendataTransportStatus status) {
  switch(status) {
    case OpendataTransportStatusAvailable:
    {
      // APP_LOG(APP_LOG_LEVEL_DEBUG, "In main callback");

      static char timestamp[7];
      strftime(timestamp, 7, "%H:%M", localtime(&info->timestamp));

      time_t now = time(NULL);
      int difference = (info->timestamp - now)/60;
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Time from now: %d minutes", difference);

      static char s_buffer[256];
      snprintf(s_buffer, sizeof(s_buffer),
        "Closest station:\n%s\n\nLine/Destination:\n%s/%s\n\nDeparture: %s\nThat's in %d'!",
        info->stop,
        info->line,
        info->destination,
        timestamp,
        difference
        );
      text_layer_set_text(s_text_layer, s_buffer);
    }
      break;
    case OpendataTransportStatusNotYetFetched:
      text_layer_set_text(s_text_layer, "\nStatusNotYetFetched");
      break;
    case OpendataTransportStatusBluetoothDisconnected:
      text_layer_set_text(s_text_layer, "\nStatusBluetoothDisconnected");
      break;
    case OpendataTransportStatusPending:
      text_layer_set_text(s_text_layer, "\nStatusPending");
      break;
    case OpendataTransportStatusFailed:
      text_layer_set_text(s_text_layer, "\nStatusFailed");
      break;
    case OpendataTransportStatusBadLocationsUrl:
      text_layer_set_text(s_text_layer, "\nStatusBadLocationUrl");
      break;
    case OpendataTransportStatusBadStationboardUrl:
      text_layer_set_text(s_text_layer, "\nStatusBadStationboardUrl");
      break;
    case OpendataTransportStatusLocationUnavailable:
      text_layer_set_text(s_text_layer, "\nStatusLocationUnavailable");
      break;
  }
}

static void js_ready_handler(void *context) {
  opendata_transport_fetch(opendata_transport_callback);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  opendata_transport_fetch(opendata_transport_callback);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(PBL_IF_ROUND_ELSE(
    grect_inset(bounds, GEdgeInsets(20, 0, 0, 0)),
    bounds));
  text_layer_set_text(s_text_layer, "\nPeblin starting up.");
  text_layer_set_text_alignment(s_text_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void window_unload(Window *window) {
  text_layer_destroy(s_text_layer);

  window_destroy(window);
}

static void init() {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  opendata_transport_init();

  app_timer_register(3000, js_ready_handler, NULL);

  accel_tap_service_subscribe(tap_handler);
}

static void deinit() {
  opendata_transport_deinit();
  accel_tap_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}
