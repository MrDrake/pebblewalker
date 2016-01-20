#include <pebble.h>

#define KEY_POKEMON 0
#define KEY_ROUTE 1

static Window *s_main_window;

static TextLayer *s_time_layer;
static TextLayer *s_step_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static BitmapLayer *s_pokemon_layer;
static GBitmap *s_pokemon_bitmap;

static BitmapLayer *s_route_layer;
static GBitmap *s_route_bitmap;

static void draw_pokemon() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  int pokemon = persist_exists(KEY_POKEMON) ? persist_read_int(KEY_POKEMON) : 0;
  int resource_id;
              
  // POKEMON
  // Get resource id
  switch(pokemon) {
      case 1: resource_id = RESOURCE_ID_IMAGE_BULBASAUR; break;
      case 4: resource_id = RESOURCE_ID_IMAGE_CHARMANDER; break;
      case 7: resource_id = RESOURCE_ID_IMAGE_SQUIRTLE; break;
      case 137: resource_id = RESOURCE_ID_IMAGE_PORYGON; break;
      case 251: resource_id = RESOURCE_ID_IMAGE_MEW; break;
      default: resource_id = RESOURCE_ID_IMAGE_PIKACHU;
  };
  // Create GBitmap
  s_pokemon_bitmap = gbitmap_create_with_resource(resource_id);
  // Create BitmapLayer to display the GBitmap
  s_pokemon_layer = bitmap_layer_create(
      GRect(PBL_IF_ROUND_ELSE(80, 68), 48, 72, 72));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_pokemon_layer, s_pokemon_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_pokemon_layer));
}

static void draw_route() {
  Layer *window_layer = window_get_root_layer(s_main_window);
  int route = persist_exists(KEY_ROUTE) ? persist_read_int(KEY_ROUTE) : 0;
  int resource_id;
              
  // ROUTE
  // Get resource id
  switch(route) {
      case 1: resource_id = RESOURCE_ID_IMAGE_ROUTE_PATHS; break;
      case 2: resource_id = RESOURCE_ID_IMAGE_ROUTE_FORESTS; break;
      case 3: resource_id = RESOURCE_ID_IMAGE_ROUTE_SEA; break;
      case 4: resource_id = RESOURCE_ID_IMAGE_ROUTE_TOWNS; break;
      case 5: resource_id = RESOURCE_ID_IMAGE_ROUTE_CAVES; break;
      case 6: resource_id = RESOURCE_ID_IMAGE_ROUTE_LAKES; break;
      case 7: resource_id = RESOURCE_ID_IMAGE_ROUTE_CITY; break;
      default: resource_id = RESOURCE_ID_IMAGE_ROUTE_PLAINS;
  };
  // Create GBitmap
  s_route_bitmap = gbitmap_create_with_resource(resource_id);
  // Create BitmapLayer to display the GBitmap
  s_route_layer = bitmap_layer_create(
      GRect(0, 72, 64, 48));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_route_layer, s_route_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_route_layer));
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  Tuple *pokemon_t = dict_find(iterator, KEY_POKEMON);
  Tuple *route_t = dict_find(iterator, KEY_ROUTE);
  int pokemon, route;
    
  if(pokemon_t) {
    pokemon = (int)pokemon_t->value->int32;
    persist_write_int(KEY_POKEMON, pokemon);
    draw_pokemon();
  }

  if(route_t) {
    route = (int)route_t->value->int32;
    persist_write_int(KEY_ROUTE, route);
    draw_route();
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
      GRect(0, 120, bounds.size.w, 2));
  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));
    
  // draw the initial pokemon and route
  draw_pokemon();  
  draw_route();  
    
  // TIME
  // Create time layers
  s_time_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(50, 32), PBL_IF_ROUND_ELSE(16, 8), 80, 26));

  // Style the text
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));

  // Add layers to the window
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

  // STEP
  // Create step Layer
  s_step_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(80, 72), 120, 64, 24));

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