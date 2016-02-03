#include <pebble.h>

#include "opendata_transport/opendata_transport.h"

static Window *s_window;
static TextLayer *s_text_layer;

static ScrollLayer *s_scroll_layer;
static TextLayer *s_content_layer;
static TextLayer *p_content_layer;
static ContentIndicator *s_indicator;
static Layer *s_indicator_up_layer, *s_indicator_down_layer;
static GRect bounds;

static char s_content[] = "\n0\n1\n3\n5\n10\n12\n13\n20\n22\n\n";
static char p_content[] = "Bern,Hirschengraben\nNFB19 Elfenau\nS9 Blinzern-Köniz\n6 Bern,Bahnhof\nNFB19 Elfenau\nS9 Blinzern\n6 Bern,Bahnhof\nNFB19 Elfenau\nS9 Blinzern\n6 Bern, Bahnhof\n\n";

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
      // text_layer_set_text(s_text_layer, s_buffer);
      text_layer_set_text(s_content_layer, s_content);
      text_layer_set_text(p_content_layer, p_content);
      GSize text_size = text_layer_get_content_size(s_content_layer);
      layer_set_frame(text_layer_get_layer(s_content_layer),
                      GRect(bounds.origin.x, bounds.origin.y, 45, text_size.h));
      layer_set_frame(text_layer_get_layer(p_content_layer),
                      GRect(bounds.origin.x+50, bounds.origin.y, 500, text_size.h));
      scroll_layer_set_content_size(s_scroll_layer, text_size);
    }
      break;
    case OpendataTransportStatusNotYetFetched:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusNotYetFetched");
      break;
    case OpendataTransportStatusBluetoothDisconnected:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusBluetoothDisconnected");
      break;
    case OpendataTransportStatusPending:
      text_layer_set_text(s_content_layer, "\n\n");
      text_layer_set_text(p_content_layer, "\nStatus\nPending");
      break;
    case OpendataTransportStatusFailed:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusFailed");
      break;
    case OpendataTransportStatusBadLocationsUrl:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusBadLocationUrl");
      break;
    case OpendataTransportStatusBadStationboardUrl:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusBadStationboardUrl");
      break;
    case OpendataTransportStatusLocationUnavailable:
      text_layer_set_text(s_content_layer, "\n");
      text_layer_set_text(p_content_layer, "\nStatusLocationUnavailable");
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
  // Layer *window_layer = window_get_root_layer(window);
  // GRect bounds = layer_get_bounds(window_layer);
  //
  // s_text_layer = text_layer_create(PBL_IF_ROUND_ELSE(
  //   grect_inset(bounds, GEdgeInsets(20, 0, 0, 0)),
  //   bounds));
  // text_layer_set_text(s_text_layer, "\nPeblin starting up.");
  // text_layer_set_text_alignment(s_text_layer, PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft));
  // layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  s_scroll_layer = scroll_layer_create(bounds);
  scroll_layer_set_click_config_onto_window(s_scroll_layer, window);
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

  s_content_layer = text_layer_create(GRect(bounds.origin.x, bounds.origin.y, 45, 2000));
  p_content_layer = text_layer_create(GRect(bounds.origin.x+50, bounds.origin.y, 500, 2000));
  // text_layer_set_text(s_content_layer, s_content);
  // text_layer_set_text(p_content_layer, p_content);
  text_layer_set_text(s_content_layer, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
  text_layer_set_text(p_content_layer, "\nPeblin\nstartin'\n...");
  text_layer_set_text_alignment(s_content_layer, GTextAlignmentRight);
  text_layer_set_text_alignment(p_content_layer, GTextAlignmentLeft);
  text_layer_set_font(s_content_layer, fonts_get_system_font(FONT_KEY_LECO_28_LIGHT_NUMBERS));
  text_layer_set_font(p_content_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(s_content_layer));
  scroll_layer_add_child(s_scroll_layer, text_layer_get_layer(p_content_layer));
  GSize text_size = text_layer_get_content_size(s_content_layer);
  layer_set_frame(text_layer_get_layer(s_content_layer),
                  GRect(bounds.origin.x, bounds.origin.y, 45, text_size.h));
  layer_set_frame(text_layer_get_layer(p_content_layer),
                  GRect(bounds.origin.x+50, bounds.origin.y, 500, text_size.h));
  scroll_layer_set_content_size(s_scroll_layer, text_size);
}

static void window_unload(Window *window) {
  // text_layer_destroy(s_text_layer);

  scroll_layer_destroy(s_scroll_layer);
  text_layer_destroy(s_content_layer);
  text_layer_destroy(p_content_layer);
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
