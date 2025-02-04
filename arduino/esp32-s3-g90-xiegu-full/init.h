
void init_uart(){
    QueueHandle_t uart_queue;
    //uart для приема данных из g90 в панель
    const uart_port_t uart_num = UART_NUM_0;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
    };
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(UART_NUM_0, -1, RXPIN0, -1, -1);
    const int uart_buffer_size = (1024 * 2);
    uart_driver_install(UART_NUM_0, uart_buffer_size,uart_buffer_size, 10, &uart_queue, 0);

    //uart для передачи данных из панели в g90
    Serial1.begin(115200, SERIAL_8N1, -1, TXPIN1);
    //uart для приема/передачи данных по CAT
    Serial2.begin(19200, SERIAL_8N1, RXPIN2, TXPIN2);

}

void get_conf(){
  EEPROM.begin(sizeof(uint32_t)*32);
  readConfig();
  //if(value_button[0]!=0x5aa5){
  //  write_parameters(true);
  //}
}


pcnt_unit_handle_t pcnt_unit = NULL;
pcnt_unit_config_t unit_config;

void encoder_init(){
    unit_config.high_limit = 2;
    unit_config.low_limit = -2;
    pcnt_new_unit(&unit_config, &pcnt_unit);
    pcnt_glitch_filter_config_t filter_config = {.max_glitch_ns = 1000,};
    pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config);
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = ROTARY_ENCODER_B_PIN,
        .level_gpio_num = ROTARY_ENCODER_A_PIN,
    };
    pcnt_channel_handle_t pcnt_chan_a = NULL;
    pcnt_new_channel(pcnt_unit, &chan_a_config, &pcnt_chan_a);
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num = ROTARY_ENCODER_A_PIN,
        .level_gpio_num = ROTARY_ENCODER_B_PIN,
    };
    pcnt_channel_handle_t pcnt_chan_b = NULL;
    pcnt_new_channel(pcnt_unit, &chan_b_config, &pcnt_chan_b);

    pcnt_channel_set_edge_action(pcnt_chan_a, PCNT_CHANNEL_EDGE_ACTION_DECREASE, PCNT_CHANNEL_EDGE_ACTION_INCREASE);
    pcnt_channel_set_level_action(pcnt_chan_a, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    pcnt_channel_set_edge_action(pcnt_chan_b, PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_DECREASE);
    pcnt_channel_set_level_action(pcnt_chan_b, PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_INVERSE);
    int watch_points[] = {-2, -1, 0, 1, 2};
    for (size_t i = 0; i < sizeof(watch_points) / sizeof(watch_points[0]); i++) {
        pcnt_unit_add_watch_point(pcnt_unit, watch_points[i]);
    }
    pcnt_unit_enable(pcnt_unit);
    pcnt_unit_clear_count(pcnt_unit);
    pcnt_unit_start(pcnt_unit);
    pinMode(ROTARY_ENCODER_BUTTON_PIN,INPUT_PULLUP);
}

static void tp_init(){
  tp.begin();
  tp.setRotation(ROTATION_INVERTED);
}

    

void lcd_init(int l){
  gfx->begin(60000000);
  gfx->setTextWrap(false);
  gfx->fillRect(0,0,480,272,BLACK);
  gfx->flush();gfx->flush();
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, l);
}



void start_ok(){
 pinMode(PTT,INPUT_PULLUP);
 WiFi.begin(ssid, password);
 int i_time=0;
 gfx->fillScreen(BLACK);
 gfx->setFont(&FreeMonoBold14pt8b);
  
 for(int x = 70;x<300;x+=4){ //заставка
   gfx->setTextSize(1);
   gfx->setCursor(30+xwin,100+xwin);
   gfx->print("Panel ESP32S3 for xiegu-g90");
   gfx->fillRect(80+xwin,170+ywin,x,5,colors[5]);
   gfx->setCursor(70+xwin,230+ywin);gfx->setTextColor(WHITE,BLACK);
   gfx->flush();
    if(WiFi.status() == WL_CONNECTED && i_time == 0){
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        getLocalTime(&timeinfo,5000);
        i_time++;
        wifi=true;
    } 
 }
 WiFi.disconnect(true);
 WiFi.mode(WIFI_OFF);
 gfx->fillScreen(BLACK);
 gfx->flush();
 gfx->fillScreen(BLACK);
}
