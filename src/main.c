//FRAKKING TOASTER!
#include <pebble.h>
#include <math.h>

#define SETTINGS_KEY 1

Window *my_window;

//text
TextLayer *textTime1_layer;
TextLayer *textTime2_layer;

//Fonts
static GFont bsg_font;

//Rectangle text layers
TextLayer *rectangle1;
TextLayer *rectangle2;
TextLayer *rectangle3;

//Bitmap Layer
static BitmapLayer *bitmapBackgroundLayer;
static GBitmap *bitmapLayer;

//Complications

//Stepc counting complications
static BitmapLayer *feetIconBitmapBackground;
static GBitmap *feetIconBitmap;
TextLayer *stepsCountTextLayer;
char stepsStatus[20];

//Date Complication
TextLayer *textDate_layer;

//Battery Complication
TextLayer *batteryStatusRectangle;
TextLayer *batteryStatusPercentage;
static BitmapLayer *batteryBitmapBackground;
static GBitmap *batteryBitmap;
static BitmapLayer *batteryChargingBitmapBackground;
static GBitmap *batteryChargingBitmap;
char batteryStatus[5];
float batteryBarCalculation;
int batteryStatusBarSize;

//Complications booleans struct
// A structure containing settings
typedef struct ClaySettings {
  bool displayBattery;
  bool displayDate;
  bool displaySteps;
} __attribute__((__packed__)) ClaySettings;

ClaySettings settings;

//Complications UPDATE

//Steps count update

static void health_handler(HealthEventType event, void *context) {
  
  if(settings.displaySteps){
  
    HealthMetric metric = HealthMetricStepCount;
    time_t start = time_start_of_today();
    time_t end = time(NULL);
    
    int totalSteps;
    
    HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
    start, end);
    
    if(mask & HealthServiceAccessibilityMaskAvailable) {
      
      totalSteps = (int)health_service_sum_today(metric);
      
      snprintf(stepsStatus,20, "%d", totalSteps);
      text_layer_set_text(stepsCountTextLayer, stepsStatus);
      
    }
    
    layer_set_hidden(bitmap_layer_get_layer(feetIconBitmapBackground),false);
    layer_set_hidden(text_layer_get_layer(stepsCountTextLayer),false);
    
  }else{
    
    layer_set_hidden(bitmap_layer_get_layer(feetIconBitmapBackground),true);
    layer_set_hidden(text_layer_get_layer(stepsCountTextLayer),true);
    
  }
  
}

//Battery update
static void updateBattery() {
  
  if(settings.displayBattery){
  
    //Battery status
    BatteryChargeState state = battery_state_service_peek();
    int statusValue = (int)state.charge_percent;
    snprintf(batteryStatus,5, "%d%%", statusValue);
    text_layer_set_text(batteryStatusPercentage, batteryStatus);
    
    batteryBarCalculation = statusValue*19;
    batteryBarCalculation = batteryBarCalculation/100;
    
    batteryStatusBarSize = roundf(batteryBarCalculation);
    
    //Change size
    text_layer_set_size(batteryStatusRectangle, GSize(batteryStatusBarSize,5));
    
    if(statusValue <= 20){
      text_layer_set_background_color(batteryStatusRectangle, GColorOrange);
    }else{
      text_layer_set_background_color(batteryStatusRectangle, GColorWhite);
    }
    
    //Check if battery is charging
    if(state.is_charging){
      //Hide normal battery bitmap and show charging battery bitmap
      
      layer_set_hidden(bitmap_layer_get_layer(batteryBitmapBackground),true);
      layer_set_hidden(bitmap_layer_get_layer(batteryChargingBitmapBackground),false);
      layer_set_hidden(text_layer_get_layer(batteryStatusRectangle),true);
      layer_set_hidden(text_layer_get_layer(batteryStatusPercentage),true);
      
    }else{
      //You know...
      
      layer_set_hidden(bitmap_layer_get_layer(batteryBitmapBackground),false);
      layer_set_hidden(bitmap_layer_get_layer(batteryChargingBitmapBackground),true);
      layer_set_hidden(text_layer_get_layer(batteryStatusRectangle),false);
      layer_set_hidden(text_layer_get_layer(batteryStatusPercentage),false);
      
    }
    
  }else{
    layer_set_hidden(bitmap_layer_get_layer(batteryBitmapBackground),true);
    layer_set_hidden(bitmap_layer_get_layer(batteryChargingBitmapBackground),true);
    layer_set_hidden(text_layer_get_layer(batteryStatusRectangle),true);
    layer_set_hidden(text_layer_get_layer(batteryStatusPercentage),true);
  }
  
  
}

static void battery_state_handler(BatteryChargeState charge) {
  updateBattery();
}


//Time loop
static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  // Write the current hours and minutes into a buffer
  static char H_buffer[8];
  static char M_buffer[8];
  
  strftime(H_buffer, sizeof(H_buffer), clock_is_24h_style() ?
                                          "%H" : "%I", tick_time);
  strftime(M_buffer, sizeof(M_buffer), clock_is_24h_style() ?
                                          "%M" : "%M", tick_time);
  
  // Display this time on the TextLayer
  text_layer_set_text(textTime1_layer, H_buffer);
  text_layer_set_text(textTime2_layer, M_buffer);
  
  if(settings.displayDate){
  
    //date
    static char date_buffer[18];
    strftime(date_buffer, sizeof(date_buffer), "%a\n%d %b", tick_time);
    
    // Show date
    text_layer_set_text(textDate_layer, date_buffer);
  }else{
    text_layer_set_text(textDate_layer, "");
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}
//End of time loop




//Clay settings

static void prv_default_settings() {
  settings.displayBattery = true;
  settings.displayDate = true;
  settings.displaySteps = true;
}

// Read settings from persistent storage
static void prv_load_settings() {
  // Load the default settings
  prv_default_settings();
  // Read settings from persistent storage, if they exist
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

// Save the settings to persistent storage
static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}


static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  
  // Read boolean preferences
  Tuple *toggleBattery = dict_find(iter, MESSAGE_KEY_showBattery);
  if(toggleBattery) {
    settings.displayBattery = toggleBattery->value->int32 == 1;
  }
  
  Tuple *toggleDate = dict_find(iter, MESSAGE_KEY_showDate);
  if(toggleDate) {
    settings.displayDate = toggleDate->value->int32 == 1;
  }
  
  Tuple *toggleSteps = dict_find(iter, MESSAGE_KEY_showSteps);
  if(toggleSteps) {
    settings.displaySteps = toggleSteps->value->int32 == 1;
  }
  
  updateBattery();
  update_time();
  
  if(!settings.displaySteps){
    layer_set_hidden(bitmap_layer_get_layer(feetIconBitmapBackground),true);
    layer_set_hidden(text_layer_get_layer(stepsCountTextLayer),true);
  }else{
    layer_set_hidden(bitmap_layer_get_layer(feetIconBitmapBackground),false);
    layer_set_hidden(text_layer_get_layer(stepsCountTextLayer),false);
  }
  
  // Save the new settings to persistent storage
  prv_save_settings();
  
}

void prv_init(void) {
  
  //Load default or stored settings
  prv_load_settings();
  
  // Open AppMessage connection
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(128, 128);
}


void handle_init(void) {
  
  //Window
  my_window = window_create();
  window_set_background_color(my_window, GColorChromeYellow);
  
  //Fonts
  bsg_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_BSG_FONT_40));
  
  //Time, battery, date and steps text layers
  
  #if defined(PBL_COLOR)
    
    textTime1_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(20, 0), PBL_IF_ROUND_ELSE(21, 8), 72, 41));
    textTime2_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(20, 0), PBL_IF_ROUND_ELSE(111, 113), 72, 41));
    
    text_layer_set_text_color(textTime1_layer, GColorBulgarianRose);
    text_layer_set_text_color(textTime2_layer, GColorBulgarianRose);
    
    batteryStatusPercentage = text_layer_create(GRect(PBL_IF_ROUND_ELSE(98, 80), PBL_IF_ROUND_ELSE(27, 17), 40, 15));
    text_layer_set_text_color(batteryStatusPercentage, GColorWhite);
    
    textDate_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(98, 83), PBL_IF_ROUND_ELSE(134, 134), 37, 47));
    text_layer_set_text_color(textDate_layer, GColorWhite);
    text_layer_set_text_alignment(textDate_layer, PBL_IF_ROUND_ELSE(GTextAlignmentRight, GTextAlignmentLeft));
  
    stepsCountTextLayer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(89, 83), PBL_IF_ROUND_ELSE(72, 54), 19));
    
  #elif defined(PBL_BW)
  
    textTime1_layer = text_layer_create(GRect(0, 8, 72, 41));
    textTime2_layer = text_layer_create(GRect(0, 113, 72, 41));
    
    text_layer_set_text_color(textTime1_layer, GColorDarkGray);
    text_layer_set_text_color(textTime2_layer, GColorDarkGray);
    
    batteryStatusPercentage = text_layer_create(GRect(80, 17, 40, 15));
    text_layer_set_text_color(batteryStatusPercentage, GColorWhite);
    
    textDate_layer = text_layer_create(GRect(83, 134, 37, 47));
    text_layer_set_text_color(textDate_layer, GColorWhite);
    text_layer_set_text_alignment(textDate_layer, GTextAlignmentLeft);
    
  #endif
  
  text_layer_set_background_color(textTime1_layer,GColorClear);
  text_layer_set_background_color(textTime2_layer,GColorClear);
  
  text_layer_set_font(textTime1_layer, bsg_font);
  text_layer_set_font(textTime2_layer, bsg_font);
  
  text_layer_set_text_alignment(textTime1_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(textTime2_layer, GTextAlignmentCenter);
  
  layer_add_child(window_get_root_layer(my_window), (Layer*) textTime1_layer);
  layer_add_child(window_get_root_layer(my_window), (Layer*) textTime2_layer);
  
  text_layer_set_background_color(batteryStatusPercentage,GColorClear);
  text_layer_set_font(batteryStatusPercentage, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(batteryStatusPercentage, GTextAlignmentCenter);
  
  text_layer_set_background_color(textDate_layer,GColorClear);
  text_layer_set_font(textDate_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  
  
  //End of text layers
  
  //Rectangles
  
  #if defined(PBL_COLOR)
    
    rectangle1 = text_layer_create(GRect(PBL_IF_ROUND_ELSE(90, 72), 0, 5, PBL_IF_ROUND_ELSE(180, 168)));
    rectangle2 = text_layer_create(GRect(PBL_IF_ROUND_ELSE(98, 80), 0, 40, PBL_IF_ROUND_ELSE(180, 168)));
    rectangle3 = text_layer_create(GRect(PBL_IF_ROUND_ELSE(141, 123), 0, 5, PBL_IF_ROUND_ELSE(180, 168)));
    
    text_layer_set_background_color(rectangle1,GColorBulgarianRose);
    text_layer_set_background_color(rectangle2,GColorBulgarianRose);
    text_layer_set_background_color(rectangle3,GColorBulgarianRose);
    
    batteryStatusRectangle = text_layer_create(GRect(PBL_IF_ROUND_ELSE(108, 90), PBL_IF_ROUND_ELSE(17, 6), 19, 5));
    text_layer_set_background_color(batteryStatusRectangle,GColorWhite);
    
  #elif defined(PBL_BW)
    
    rectangle1 = text_layer_create(GRect(72, 0, 5, 168));
    rectangle2 = text_layer_create(GRect(80, 0, 40, 168));
    rectangle3 = text_layer_create(GRect(123, 0, 5, 168));
    
    text_layer_set_background_color(rectangle1,GColorBlack);
    text_layer_set_background_color(rectangle2,GColorBlack);
    text_layer_set_background_color(rectangle3,GColorBlack);
    
    batteryStatusRectangle = text_layer_create(GRect(90, 6, 19, 5));
    text_layer_set_background_color(batteryStatusRectangle,GColorWhite);
    
  #endif
  
  layer_add_child(window_get_root_layer(my_window), (Layer*) rectangle1);
  layer_add_child(window_get_root_layer(my_window), (Layer*) rectangle2);
  layer_add_child(window_get_root_layer(my_window), (Layer*) rectangle3);
  
  layer_add_child(window_get_root_layer(my_window), (Layer*) batteryStatusRectangle);
  
  //Bitmap Layers
  
  #if defined(PBL_COLOR)
    bitmapLayer = gbitmap_create_with_resource(RESOURCE_ID_BSG_INSIGNIA);
    bitmapBackgroundLayer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(72, 54),PBL_IF_ROUND_ELSE(45, 39),92,91));
    
    batteryBitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_NORMAL);
    batteryBitmapBackground = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(105, 87), PBL_IF_ROUND_ELSE(14, 3), 27, 11));
    
    batteryChargingBitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
    batteryChargingBitmapBackground = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(105, 87), PBL_IF_ROUND_ELSE(14, 3), 27, 11));
    
  #elif defined(PBL_BW)
    bitmapLayer = gbitmap_create_with_resource(RESOURCE_ID_BSG_PB_INSIGNIA);
    bitmapBackgroundLayer = bitmap_layer_create(GRect(54,39,92,91));
    
    batteryBitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_NORMAL);
    batteryBitmapBackground = bitmap_layer_create(GRect(87, 3, 27, 11));
    
    batteryChargingBitmap = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGING);
    batteryChargingBitmapBackground = bitmap_layer_create(GRect(87, 3, 27, 11));
    
  #endif
  
  bitmap_layer_set_bitmap(bitmapBackgroundLayer, bitmapLayer);
  bitmap_layer_set_background_color(bitmapBackgroundLayer,GColorClear);
  bitmap_layer_set_compositing_mode(bitmapBackgroundLayer, GCompOpSet);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(bitmapBackgroundLayer));
  
  bitmap_layer_set_bitmap(batteryBitmapBackground, batteryBitmap);
  bitmap_layer_set_background_color(batteryBitmapBackground,GColorClear);
  bitmap_layer_set_compositing_mode(batteryBitmapBackground, GCompOpSet);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(batteryBitmapBackground));
  
  bitmap_layer_set_bitmap(batteryChargingBitmapBackground, batteryChargingBitmap);
  bitmap_layer_set_background_color(batteryChargingBitmapBackground,GColorClear);
  bitmap_layer_set_compositing_mode(batteryChargingBitmapBackground, GCompOpSet);
  layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(batteryChargingBitmapBackground));
  
  
  
  //Add battery and date text layer to window
  layer_add_child(window_get_root_layer(my_window), (Layer*) batteryStatusPercentage);
  layer_add_child(window_get_root_layer(my_window), (Layer*) textDate_layer);
  
  //Events
  
  //Time events
  update_time();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Battery events
  updateBattery();
  battery_state_service_subscribe(battery_state_handler);
  
  
  //Steps counting elements
  #if defined(PBL_HEALTH)
    
    if(health_service_events_subscribe(health_handler, NULL)) {
  
      feetIconBitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_FEET);
      feetIconBitmapBackground = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(21, 12),PBL_IF_ROUND_ELSE(73, 67),30,18));
      
      bitmap_layer_set_bitmap(feetIconBitmapBackground, feetIconBitmap);
      bitmap_layer_set_background_color(feetIconBitmapBackground,GColorClear);
      bitmap_layer_set_compositing_mode(feetIconBitmapBackground, GCompOpSet);
      layer_add_child(window_get_root_layer(my_window), bitmap_layer_get_layer(feetIconBitmapBackground));
      
      //Add steps text layer
      text_layer_set_text_color(stepsCountTextLayer, GColorBulgarianRose );
      text_layer_set_background_color(stepsCountTextLayer,GColorClear);
      text_layer_set_font(stepsCountTextLayer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
      text_layer_set_text_alignment(stepsCountTextLayer, GTextAlignmentCenter);
      layer_add_child(window_get_root_layer(my_window), (Layer*) stepsCountTextLayer);
      
      //Test only
      //text_layer_set_text(stepsCountTextLayer, "4242");
  
    }
      
  #endif
  
  
  window_stack_push(my_window, true);
}



void handle_deinit(void) {
  //Deinit time text layers
  text_layer_destroy(textTime1_layer);
  text_layer_destroy(textTime2_layer);
  
  //Deinit sidebar
  text_layer_destroy(rectangle1);
  text_layer_destroy(rectangle2);
  text_layer_destroy(rectangle3);
  
  //Deinit bsg symbol
  gbitmap_destroy(bitmapLayer);
  bitmap_layer_destroy(bitmapBackgroundLayer);
  
  //Deinit battery complication
  text_layer_destroy(batteryStatusRectangle);
  text_layer_destroy(batteryStatusPercentage);
  
  gbitmap_destroy(batteryBitmap);
  bitmap_layer_destroy(batteryBitmapBackground);
  
  gbitmap_destroy(batteryChargingBitmap);
  bitmap_layer_destroy(batteryChargingBitmapBackground);
  
  //Deinit date complication
  text_layer_destroy(textDate_layer);
  
  //Deinit steps count complication
  gbitmap_destroy(feetIconBitmap);
  bitmap_layer_destroy(feetIconBitmapBackground);
  text_layer_destroy(stepsCountTextLayer);
  
  //Deinit fonts
  fonts_unload_custom_font(bsg_font);
  
  //Unsubscribe events
  battery_state_service_unsubscribe();
  tick_timer_service_unsubscribe();
  health_service_events_unsubscribe();
  
  //Deinit window
  window_destroy(my_window);
  
  //So say we all!
  
}

int main(void) {
  prv_init();
  handle_init();
  app_event_loop();
  handle_deinit();
}
