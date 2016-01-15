#include <pebble.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_POKEMON 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_step_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static BitmapLayer *s_pokemon_layer;
static GBitmap *s_pokemon_bitmap;

static GFont s_time_font;
static GFont s_step_font;

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char step_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    snprintf(step_layer_buffer, sizeof(step_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
    text_layer_set_text(s_step_layer, step_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();

  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // BACKGROUND (LINE)  
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LINE);
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(120, 112), bounds.size.w, 2));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

  // POKEMON
  // Create GBitmap
  s_pokemon_bitmap = gbitmap_create_with_resource(
      KEY_POKEMON != 4 ? RESOURCE_ID_IMAGE_CHARMANDER : RESOURCE_ID_IMAGE_PIKACHU);
  // Create BitmapLayer to display the GBitmap
  s_pokemon_layer = bitmap_layer_create(
      GRect(PBL_IF_ROUND_ELSE(75, 60), PBL_IF_ROUND_ELSE(38, 30), 80, 80));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_pokemon_layer, s_pokemon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_pokemon_layer));
    
  // TIME
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(8, 0), PBL_IF_ROUND_ELSE(58, 52), 60, 26));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  // Create GFont
  s_time_font = fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_font(s_time_layer, s_time_font);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // STEP
  // Create step Layer
  s_step_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(90, 72), PBL_IF_ROUND_ELSE(122, 114), 68, 20));
  // Style the text
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentRight);
  text_layer_set_text(s_step_layer, "Loading...");
  // Create second custom font, apply it and add to Window
  s_step_font = fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD);
  text_layer_set_font(s_step_layer, s_step_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_step_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);

  // Unload GFont
  // fonts_unload_custom_font(s_time_font);

  // Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);

  // Destroy GBitmap
  gbitmap_destroy(s_pokemon_bitmap);
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_pokemon_layer);

  // Destroy step elements
  text_layer_destroy(s_step_layer);
  // fonts_unload_custom_font(s_step_font);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set the background color
  window_set_background_color(s_main_window, GColorWhite);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}