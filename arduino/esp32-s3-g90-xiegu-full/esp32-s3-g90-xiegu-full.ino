#include <WiFi.h>
#include "time.h"
#include "driver/pulse_cnt.h"
#include <EEPROM.h>
#include "src/tamc_gt911/TAMC_GT911.h" //https://github.com/TAMCTec/gt911-arduino
#include <Arduino_GFX_Library.h> //https://github.com/moononournation/Arduino_GFX
#include "include/FreeMonoBold8.h"
#include "include/Picopixel.h"
#include "include/CourierCyr6.h"
#include "include/FreeMonoBold14.h"
#include "_tmp.h"
#include "global.h"
#include "g90.h"
#include "ci_v.h"
#include "fft.h"
#include "include/s7.h"          //7-segments font
#include "include/images.h"
#include "screens.h"
#include "txrx.h"
#include "init.h"
#include "tools.h"



void setup()
{
  get_conf();//восстанавливаем параметры
  init_uart(); //2 канала uart для приема и передачи в/из g90
  lcd_init(1);
  encoder_init();
  tp_init();
  fft_init();
  start_ok(); //заставка, вкл wifi, получение времени,выкл wifi
  txrx_mode=RX_MODE;
  vSemaphoreCreateBinary(xIN);
  vSemaphoreCreateBinary(xUART);
  xTaskCreatePinnedToCore(x_in,"rxin",STACK_SIZE*2,NULL,14,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(x_uart_rx,"uart_rx",STACK_SIZE*2,NULL,12,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(x_uart_tx,"uart_tx",STACK_SIZE*2,NULL,11,NULL,0);
  delay(100);
  xTaskCreatePinnedToCore(x_encoder,"xencoder",STACK_SIZE/2, NULL, 13, NULL, 1);
  delay(1000);
  xTaskCreatePinnedToCore(x_cat,"cat",STACK_SIZE, NULL, 14, NULL, 1);
  delay(1000);
  
  xSemaphoreGive(xUART);
}

void loop(void){
    cur_ms=millis();
    screens(txrx_mode);
    gfx->flush();
    time_001();
    time1();
    control();
    fps=millis()-cur_ms;
}
