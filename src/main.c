#include <pebble.h>

#define KEY_POKEMON 0

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_step_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static BitmapLayer *s_pokemon_layer;
static GBitmap *s_pokemon_bitmap;

static BitmapLayer *s_route_layer;
static GBitmap *s_route_bitmap;

static GFont s_poketch_font;

static void draw_pokemon() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  int pokemon = persist_exists(KEY_POKEMON) ? persist_read_int(KEY_POKEMON) : 0;
              
  // POKEMON
  // Create GBitmap
  s_pokemon_bitmap = gbitmap_create_with_resource(
      pokemon == 4 ? RESOURCE_ID_IMAGE_CHARMANDER : RESOURCE_ID_IMAGE_PIKACHU);
  // Create BitmapLayer to display the GBitmap
  s_pokemon_layer = bitmap_layer_create(
      GRect(PBL_IF_ROUND_ELSE(80, 68), 40, 72, 72));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_pokemon_layer, s_pokemon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_pokemon_layer));
}

static void draw_route(int route) {
  Layer *window_layer = window_get_root_layer(s_main_window);
              
  // ROUTE
  // Create GBitmap
  s_route_bitmap = gbitmap_create_with_resource(
      route == 0 ? RESOURCE_ID_IMAGE_ROUTE_TOWNS : RESOURCE_ID_IMAGE_ROUTE_TOWNS);
  // Create BitmapLayer to display the GBitmap
  s_route_layer = bitmap_layer_create(
      GRect(0, 64, 64, 48));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_route_layer, s_route_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_route_layer));
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *pokemon_t = dict_find(iterator, KEY_POKEMON);
  int pokemon;
    
  if(pokemon_t) {
    pokemon = (int)pokemon_t->value->int32;
    persist_write_int(KEY_POKEMON, pokemon);
    draw_pokemon();
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *t = localtime(&temp);

  // time buffers
  static char s_buffer[8];
  static char s_tbuffer[8];

  // Display time
  strftime(s_buffer, sizeof(s_buffer), "00%H%M", t);
  text_layer_set_text(s_step_layer, s_buffer);
  strftime(s_tbuffer, sizeof(s_tbuffer), "%H:%M", t);
  text_layer_set_text(s_time_layer, s_tbuffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
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
      GRect(0, 112, bounds.size.w, 2));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
    
  // draw the initial pokemon and route
  draw_pokemon();  
  draw_route(0);  
    
  // TIME
  // Create time layers
  s_time_layer = text_layer_create(
      GRect(0, 40, 64, 26));

  // Load custom font
  s_poketch_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_POKETCH_24));
    
  // Style the hour text
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentRight);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, s_poketch_font);

  // Add layers to the window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // STEP
  // Create step Layer
  s_step_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(80, 72), 112, 64, 26));

  // Style the text
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentRight);
  text_layer_set_text(s_step_layer, "0");
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

  // Add layer to the window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_step_layer));
}

static void main_window_unload(Window *window) {
  // Destroy bitmap elements and layers
  gbitmap_destroy(s_background_bitmap);
  bitmap_layer_destroy(s_background_layer);

  gbitmap_destroy(s_pokemon_bitmap);
  bitmap_layer_destroy(s_pokemon_layer);

  // Destroy text elements
  text_layer_destroy(s_step_layer);
  text_layer_destroy(s_time_layer);
  fonts_unload_custom_font(s_poketch_font);
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