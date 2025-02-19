

xSemaphoreHandle xUART;
xSemaphoreHandle xIN;

void x_in(void *args)
{
      while(1){
        //
        if (xSemaphoreTake(xIN, portMAX_DELAY)) {
            //берем значения текущих параметров из принятого массива
            smeter_g90=(txrx_mode==RX_MODE)?from_body[0]:from_body[1];
            txpower_g90=(float)from_body[1]/10.0;
            swr_s90=from_body[2];
            power_g90=(float)from_body[3]/10.0;
            alc_g90=map(from_body[5],0,255,0,100);
            version_g90=from_body[6];
            int k=0;
            //берем данные fft из принятого массива
            //g90 выдает в панель спектр из 160 точек, поэтому растягиваем на весь экран. 1->3
            for(int i=0;i<NUM_SAMPLE_BUF;i+=3){ 
              fft_g90[i]=from_body[k+48];
              fft_g90[i+1]=from_body[k+48];
              fft_g90[i+2]=from_body[k+48];
              k+=2;//данные присутствуют только в каждом втором байте массива fft из g90
            }
            fft_for_display((uint8_t*)&fft_g90); //готовим спектр и панораму
           }
        if(txrx_mode==RX_MODE && shift_en)scroll_wp();//сдвинуть панораму на 1 строку вниз
        xSemaphoreGive(xUART);
      }
    vTaskDelete(NULL);
}

static void x_uart_rx(void *args) {
 while(1){
          
          xSemaphoreTake(xUART, portMAX_DELAY);
            //принимаем данные из g90
            if(get_data_from_body()){xSemaphoreGive(xIN);}//если контр.сумма ок - разрешаем обработку
            else {vTaskDelay(10);xSemaphoreGive(xUART);} //иначе повторяем прием 
      }
}

static void x_uart_tx(void *args) {

 while(1){
        //Заполняем структуру для отправки в g90
        if(send_carrier){//несущую на передачу для теста или сканирования swr
         mic_gain = 0;
         auxi = 0;
         auxo = 0;
         //rf_power = 7;
         rf_mode = 6;//MFM
         //show_par = "POWR";show_time=SHOW_VALUE;
        }else{
          mic_gain = tmp_mic_gain;
          auxi = tmp_auxi;
          auxo = tmp_auxo;
          rf_power = tmp_rf_power;
          rf_mode = tmp_rf_mode;
         }
        data_to_g90.filter_low_raw = (uint8_t)((indent/50)-1);
        data_to_g90.filter_high_raw = (uint8_t)(((bandwidth-indent)/50)-1);
        data_to_g90.rf_power = rf_power;
        data_to_g90.modulation = rf_mode;         //0..7
        data_to_g90.agc = agc_speed;              //0..3
        data_to_g90.volume = (uint8_t)volume;     //0..28
        data_to_g90.mic_gain = (uint8_t)mic_gain; //0..20
        data_to_g90.ctrl8_swr = (uint8_t)((swr_threshold-1.8)/1.2);
        if(!speaker)data_to_g90.ctrl1 |= (1 << 2);else data_to_g90.ctrl1 &= ~(1 << 2);
        if(tuner)data_to_g90.ctrl1 |= (1 << 5);else data_to_g90.ctrl1 &= ~(1 << 5);
        if(mic_c)data_to_g90.ctrl1 |= (1 << 3);else data_to_g90.ctrl1 &= ~(1 << 3);
        data_to_g90.att_mode = (uint8_t)bands[numband].att_mode;
        data_to_g90.ctrl8_aux = (((uint8_t)auxo)<<4)|((uint8_t)auxi);
        data_to_g90.mic_gain = (uint8_t)mic_gain;
        if(!mic_line)data_to_g90.ctrl3 |= (1 << 2);else data_to_g90.ctrl3 &= ~(1 << 2);
        if(tuning)data_to_g90.ctrl2 |= (1 << 5);else data_to_g90.ctrl2 &= ~(1 << 5);
        if(shutdown_req>2) data_to_g90.ctrl2 |= (1 << 7);else data_to_g90.ctrl2 &= ~(1 << 7);
        data_to_g90.ctrl4_rfgain = (data_to_g90.ctrl4_rfgain&7)|rf_gain<<3;
        data_to_g90.ctrl4_other  = (data_to_g90.ctrl4_other&252)|rf_gain>>5;

        if(txrx_mode==RX_MODE){//режим приема
            data_to_g90.freq1 = (tun_mode==RIT)?rx_freq:freq;
            data_to_g90.ctrl1 &= ~(1 << 7);
        }
        if(txrx_mode==TX_MODE){//режим передачи
            data_to_g90.freq1 = freq;
            data_to_g90.ctrl1 |= (1 << 7);
        }

      //отправка структуры в g90
      send_data_to_body();
      vTaskDelay(23);
      }
}
