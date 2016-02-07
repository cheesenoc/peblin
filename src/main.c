#include <pebble.h>

#include "opendata_transport/opendata_transport.h"

static Window *s_window;

static ScrollLayer *s_scroll_layer;
static TextLayer *times_content_layer;
static TextLayer *stops_content_layer;
static ContentIndicator *s_indicator;
static Layer *s_indicator_up_layer, *s_indicator_down_layer;
static GRect bounds;
static int number;

static void opendata_transport_callback(OpendataTransportInfo *info, OpendataTransportStatus status) {
  layer_set_frame(text_layer_get_layer(times_content_layer),
                  GRect(bounds.origin.x, bounds.origin.y, 45, 2000));
  layer_set_frame(text_layer_get_layer(stops_content_layer),
                  GRect(bounds.origin.x+50, bounds.origin.y, 500, 2000));
  switch(status) {
    case OpendataTransportStatusAvailable:
      text_layer_set_text(times_content_layer, info->times);
      text_layer_set_text(stops_content_layer, info->stops);
      break;
    case OpendataTransportStatusNotYetFetched:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nNot Yet\nFetched\n");
      break;
    case OpendataTransportStatusBluetoothDisconnected:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nBluetooth\nDisconnected\n");
      break;
    case OpendataTransportStatusPending:
      text_layer_set_text(times_content_layer, "\n\n");
      text_layer_set_text(stops_content_layer, "\nPeblin\nLoading\n");
      break;
    case OpendataTransportStatusFailed:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nFailed\n");
      break;
    case OpendataTransportStatusBadLocationsUrl:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nBad Location Url\n");
      break;
    case OpendataTransportStatusBadStationboardUrl:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nBad Stationboard Url\n");
      break;
    case OpendataTransportStatusLocationUnavailable:
      text_layer_set_text(times_content_layer, "\n");
      text_layer_set_text(stops_content_layer, "\nStatus\nLocation\nUnavailable\n");
      break;
  }
  GSize text_size = text_layer_get_content_size(stops_content_layer);
  layer_set_frame(text_layer_get_layer(times_content_layer),
                  GRect(bounds.origin.x, bounds.origin.y+STATUS_BAR_LAYER_HEIGHT, 45, text_size.h+STATUS_BAR_LAYER_HEIGHT));
  layer_set_frame(text_layer_get_layer(stops_content_layer),
                  GRect(bounds.origin.x+50, bounds.origin.y+STATUS_BAR_LAYER_HEIGHT, 500, text_size.h+STATUS_BAR_LAYER_HEIGHT));
  scroll_layer_set_content_size(s_scroll_layer, text_size);
}

static void js_ready_handler(void *context) {
  number = 1;
  opendata_transport_fetch(1, opendata_transport_callback);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
  number = 1;
  opendata_transport_fetch(1, opendata_transport_callback);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  number = number % 10 + 1;
  opendata_transport_fetch(number, opendata_transport_callback);
}

static void click_config_provider(void *context) {
  // Register the ClickHandlers
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  s_scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
  scroll_layer_set_callbacks(s_scroll_layer,  (ScrollLayerCallbacks){.click_config_provider=click_config_provider});

  scroll_layer_set_shadow_hidden(s_scroll_layer, true);
  layer_add_child(window_layer, scroll_layer_get_layer(s_scroll_layer));

  // Get the ContentIndicator from the ScrollLayer
  s_indicator = scroll_layer_get_content_indicator(s_scroll_layer);

  // Create two Layers to draw the arrows
  s_indicator_up_layer = layer_create(GRect(bounds.origin.x, bounds.origin.y,
                                      bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  s_indicator_down_layer = layer_create(GRect(0, bounds.size.h - STATUS_BAR_LAYER_HEIGHT,
                                        bounds.size.w, STATUS_BAR_LAYER_HEIGHT));
  layer_add_child(window_layer, s_indicator_up_layer);
  layer_add_child(window_layer, s_indicator_down_layer);

  // Configure the properties of each indicator
  const ContentIndicatorConfig up_config = (ContentIndicatorConfig) {
    .layer = s_indicator_up_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorBlack,
      .background = GColorWhite
    }
  };
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionUp,
                                        &up_config);

  const ContentIndicatorConfig down_config = (ContentIndicatorConfig) {
    .layer = s_indicator_down_layer,
    .times_out = false,
    .alignment = GAlignCenter,
    .colors = {
      .foreground = GColorBlack,
      .background = GColorWhite
    }
  };
  content_indicator_configure_direction(s_indicator, ContentIndicatorDirectionDown,
                                        &down_config);

  times_content_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, 45, 2000));
  stops_content_layer = text_layer_create(GRect(bounds.origin.x+50, bounds.origin.y, 500, 2000));

  text_layer_set_text(times_content_layer, "\n");
  text_layer_set_text(stops_content_layer, "\nPeblin\nStarting\n");
  text_layer_set_text_alignment(times_content_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(stops_content_layer, GTextAlignmentLeft);
  text_layer_set_font(times_content_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_font(stops_content_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(times_content_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(stops_content_layer));
  GSize text_size = text_layer_get_content_size(stops_content_layer);
  layer_set_frame(text_layer_get_layer(times_content_layer),
                  GRect(bounds.origin.x, bounds.origin.y+STATUS_BAR_LAYER_HEIGHT, 45, text_size.h+STATUS_BAR_LAYER_HEIGHT));
  layer_set_frame(text_layer_get_layer(stops_content_layer),
                  GRect(bounds.origin.x+50, bounds.origin.y+STATUS_BAR_LAYER_HEIGHT, 500, text_size.h+STATUS_BAR_LAYER_HEIGHT));
  scroll_layer_set_content_size(s_scroll_layer, text_size);
}

static void window_unload(Window *window) {
  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(times_content_layer);
  text_layer_destroy(stops_content_layer);
  layer_destroy(s_indicator_up_layer);
  layer_destroy(s_indicator_down_layer);

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
