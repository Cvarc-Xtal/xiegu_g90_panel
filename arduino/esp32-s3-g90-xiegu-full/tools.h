void value_plus(){

  if(show_par=="F__L"){indent+=50;if(indent>750)indent=750;f_low.value=indent;show_time=SHOW_VALUE;}
  if(show_par=="F__H"){bandwidth+=50;if(bandwidth>6000)bandwidth=6000;f_high.value=bandwidth;show_time=SHOW_VALUE;}
  if(show_par=="POWR"){rf_power+=1;if(rf_power>20)rf_power=20;tmp_rf_power=rf_power;show_time=SHOW_VALUE;}
  if(show_par=="AUXI"){auxi+=1;if(auxi>15)auxi=15;tmp_auxi=auxi;show_time=SHOW_VALUE;}
  if(show_par=="AUXO"){auxo+=1;if(auxo>15)auxo=15;tmp_auxo=auxo;show_time=SHOW_VALUE;}
  if(show_par=="MICG"){mic_gain+=1;if(mic_gain>20)mic_gain=20;tmp_mic_gain=mic_gain;show_time=SHOW_VALUE;}
  if(show_par=="RF_G"){rf_gain+=1;if(rf_gain>100)rf_gain=100;show_time=SHOW_VALUE;}
}

void value_minus(){

  if(show_par=="F__L"){indent-=50;if(indent<100)indent=100;f_low.value=indent;show_time=SHOW_VALUE;}
  if(show_par=="F__H"){bandwidth-=50;if(bandwidth<indent+200)bandwidth=indent+200;f_high.value=bandwidth;show_time=SHOW_VALUE;}
  if(show_par=="POWR"){rf_power-=1;if(rf_power<1)rf_power=1;tmp_rf_power=rf_power;show_time=SHOW_VALUE;}
  if(show_par=="AUXI"){auxi-=1;if(auxi<0)auxi=0;tmp_auxi=auxi;show_time=SHOW_VALUE;}
  if(show_par=="AUXO"){auxo-=1;if(auxo<0)auxo=0;tmp_auxo=auxo;show_time=SHOW_VALUE;}
  if(show_par=="MICG"){mic_gain-=1;if(mic_gain<0)mic_gain=0;tmp_mic_gain=mic_gain;show_time=SHOW_VALUE;}
  if(show_par=="RF_G"){rf_gain-=1;if(rf_gain<1)rf_gain=1;show_time=SHOW_VALUE;}
}

void swr_fill(){
  int swr_step = (bands[numband].f_max - bands[numband].f_min)/160;
  for(int i=0;i<160;i++){
    swr_freq[i]=bands[numband].f_min + swr_step*i;
  }
}

void action() { //обработка нажатий на кнопки
  //if (txrx_mode == RX_MODE) { //на экране приема
    if (lkey == 1) {tp_band=false;if (numband > N_BANDS - 1)numband = 0;tmp_rf_mode=rf_mode=bands[numband].mode;show_time = SHOW_VALUE;send_cat=true;swr_fill();}
    if (lkey == 2) {if(show_par!="____")value_plus();else{freq += i2s_sample_rate_rx/4;if (freq > 39999999)freq = 39999999;rx_freq=freq;}send_cat=true;}
    if (lkey == 3) {if(show_par!="____")value_minus();else{freq -= i2s_sample_rate_rx/4;if (freq < 1000000)freq = 1000000;rx_freq=freq;}send_cat=true;}
    if (lkey == 4) {}
    if (lkey == 5) {numstep++;if(numstep>4)numstep=0;}
    if (lkey == 6) {rf_mode++;tmp_rf_mode=rf_mode;if(rf_mode>AM)tmp_rf_mode=rf_mode=LSB;send_cat=true;}
    if (lkey == 7) {if (tun_mode == RIT){tun_mode = TUN;rx_freq=freq;}else tun_mode = RIT;}
    if (lkey == 8) {tmpfreq=freq;swr_scan=true;send_carrier=true;menu=false;rf_power=7;}
    if (lkey == 9) {volume+=1;if(volume>28)volume=28;show_time=SHOW_VALUE;}
    if (lkey ==10) {volume-=1;if(volume>28)volume=0;show_time=SHOW_VALUE;}
    if (lkey ==11) {flag_write_parameters = true; show_time = SHOW_VALUE;}//(сохранение текущих парметров)
    if (lkey ==12) {agc_speed++;if(agc_speed>3)agc_speed=0;}
    if (lkey ==13) {show_par = "POWR";show_time=SHOW_VALUE+5;}
    if (lkey ==14) {speaker=!speaker;if(speaker)fout.value=(char*)"Spkr";else fout.value=(char*)"Head";}
    if (lkey ==15) {tuner=!tuner;}
    if (lkey ==16) {mic_c=!mic_c;if(mic_c)fmic.value=(char*)"MicC";else fmic.value=(char*)"Mic-";}
    if (lkey ==17) {bands[numband].att_mode++;if(bands[numband].att_mode>2)bands[numband].att_mode=0;}
    if (lkey ==18) {more_menu=!more_menu;}
    if (lkey ==19) {mic_line=!mic_line;}
    if (lkey ==20) {if(swr_scan)freq=tmpfreq;menu = !menu; swr_scan=false; send_carrier=false;}
    if (lkey ==21) {tuning=true;}
    if (lkey ==22) {tp_band=true;}
    if (lkey ==23) {send_carrier=!send_carrier;}
    
    lkey = 0;
  //}

}

void time1() {
  int static ms1;
  if ((cur_ms < ms1) || ((cur_ms - ms1) > 1000 )) { //1sec
    ms1 = cur_ms;
    if (show_time == 1)show_time = 0;
    show_time--;
    if (show_time < 0){show_time = -1;show_par="____";tuning=false;}
    if(show_tap>0){tap_name="_______";show_tap=0;}
    if(tap_name!="_______")show_tap++;
    t_fps = fps;
    getLocalTime(&timeinfo,0);
    if(!power_button){shutdown_req++;}
    else shutdown_req=0;
  }
}

void time_001() {
  int static ms001 = 0;
  int static swr_count = 0;
  static bool swr_switch=false;
  if ((cur_ms < ms001) || ((cur_ms - ms001) > 40 )) {
    ms001 = cur_ms;
    if(txrx_mode==RX_MODE){smeter = map(256-smeter_g90,127,13,0,98);}
    else{float tx_smeter = ((float)smeter_g90)/10.0;smeter = map(tx_smeter,0.0,(float)rf_power,0,100);}
    if (smeter <= old_smeter && old_smeter - smeter >= 1) {old_smeter--;}
    if (smeter > old_smeter && smeter - old_smeter > 10)  {old_smeter += 10;}
    if (smeter > old_smeter && smeter - old_smeter > 5)   {old_smeter += 5;}
    if (smeter > old_smeter && smeter - old_smeter > 2)   {old_smeter += 2;}
    if (smeter > old_smeter && smeter - old_smeter >= 1)  {old_smeter++;}
    if (old_smeter < 0)old_smeter = 0;
    if (old_smeter > 150)old_smeter = 150;
    peak_down();//инкремент показателей спектра и панорамы
    if(!power_button){show_par="____";}
    /////сканируем диапазон SWR
    if(!swr_switch && swr_scan)freq=swr_freq[swr_count];
    if(swr_switch && swr_scan){
      swr_value[swr_count]=from_body[2]+1;
      swr_count++;if(swr_count>160)swr_count=0;
    }
    swr_switch=!swr_switch;
    /////

  }
}

void change_band() {
  static int old_band = 100;
  static bool first = true;
  if (old_band == numband)return;
  if (!first)freq = bands[numband].freq;
  old_band = numband; first = false;
}

#define BTN_SCAN (tp_x>fscan.x_min&&tp_x<fscan.x_max&&tp_y>fscan.y_min&&tp_y<fscan.y_max)
#define BTN_RFG (tp_x>frf.x_min&&tp_x<frf.x_max&&tp_y>frf.y_min&&tp_y<frf.y_max)
#define BTN_PTT (tp_x>fptt.x_min&&tp_x<fptt.x_max&&tp_y>fptt.y_min&&tp_y<fptt.y_max)
#define BTN_TUNE (tp_x>ftune.x_min&&tp_x<ftune.x_max&&tp_y>ftune.y_min&&tp_y<ftune.y_max)
#define BTN_FLM (tp_x>flm.x_min&&tp_x<flm.x_max&&tp_y>flm.y_min&&tp_y<flm.y_max)
#define BTN_MICG (tp_x>fmicg.x_min&&tp_x<fmicg.x_max&&tp_y>fmicg.y_min&&tp_y<fmicg.y_max)
#define BTN_AUXI (tp_x>fauxi.x_min&&tp_x<fauxi.x_max&&tp_y>fauxi.y_min&&tp_y<fauxi.y_max)
#define BTN_AUXO (tp_x>fauxo.x_min&&tp_x<fauxo.x_max&&tp_y>fauxo.y_min&&tp_y<fauxo.y_max)
#define BTN_MIC (tp_x>fmic.x_min&&tp_x<fmic.x_max&&tp_y>fmic.y_min&&tp_y<fmic.y_max)
#define BTN_TUN (tp_x>ftun.x_min&&tp_x<ftun.x_max&&tp_y>ftun.y_min&&tp_y<ftun.y_max)
#define BTN_OUT (tp_x>fout.x_min&&tp_x<fout.x_max&&tp_y>fout.y_min&&tp_y<fout.y_max)
#define BTN_RFPOWER (tp_x>fpower.x_min&&tp_x<fpower.x_max&&tp_y>fpower.y_min&&tp_y<fpower.y_max)
#define BTN_AGC (tp_x>fagc.x_min&&tp_x<fagc.x_max&&tp_y>fagc.y_min&&tp_y<fagc.y_max)
#define BTN_ATT (tp_x>fatt.x_min&&tp_x<fatt.x_max&&tp_y>fatt.y_min&&tp_y<fatt.y_max)
#define BTN_MORE (tp_x>fmore.x_min&&tp_x<fmore.x_max&&tp_y>fmore.y_min&&tp_y<fmore.y_max)
#define BTN_STEP (tp_x>fstep.x_min&&tp_x<fstep.x_max&&tp_y>fstep.y_min&&tp_y<fstep.y_max)
#define BTN_RMODE (tp_x>rfmode.x_min&&tp_x<rfmode.x_max&&tp_y>rfmode.y_min&&tp_y<rfmode.y_max)
#define BTN_F_LOW (tp_x>f_low.x_min&&tp_x<f_low.x_max&&tp_y>f_low.y_min&&tp_y<f_low.y_max)
#define BTN_F_HIGH (tp_x>f_high.x_min&&tp_x<f_high.x_max&&tp_y>f_high.y_min&&tp_y<f_high.y_max)
#define BTN_CONTROL (tp_x>400&&tp_x<472&&tp_y>10&&tp_y<70&&tp_size>30)
#define BTN_RIT   (tp_x>tp_rit.x_min&&tp_x<tp_rit.x_max&&tp_y>tp_rit.y_min&&tp_y<tp_rit.y_max)
#define BTN_BAND   (tp_x>tp_nband.x_min&&tp_x<tp_nband.x_max&&tp_y>tp_nband.y_min&&tp_y<tp_nband.y_max)
#define BTN_SBAND  (tp_x>10&&tp_x<470&&tp_y>70&&tp_y<150&&tp_band)
#define BTN_SAVE_CONF  (tp_x>30&&tp_x<220&&tp_y>20&&tp_y<65)
#define BTN_FREQ_PLUS  (tp_x>160&&tp_x<320&&tp_y>80&&tp_y<150&&!tp_band)
#define BTN_FREQ_PLUS_FULL  (tp_x>280&&tp_x<470&&tp_y>80&&tp_y<150&&!tp_band)
#define BTN_FREQ_MINUS (tp_x>25&&tp_x<160&&tp_y>70&&tp_y<150&&!tp_band)
#define BTN_FREQ_MINUS_FULL (tp_x>25&&tp_x<220&&tp_y>70&&tp_y<150&&!tp_band)
#define BTN_VOL_PLUS   (tp_x>160&&tp_x<320&&tp_y>200&&tp_y<270&&!tp_band)
#define BTN_VOL_PLUS_FULL   (tp_x>270&&tp_x<470&&tp_y>200&&tp_y<270&&!tp_band)
#define BTN_VOL_MINUS  (tp_x>25&&tp_x<160&&tp_y>200&&tp_y<270&&!tp_band)
#define BTN_VOL_MINUS_FULL  (tp_x>25&&tp_x<220&&tp_y>200&&tp_y<270&&!tp_band)

void t_touched(){ //опрос тачскрина
    static bool start = true;
    static int l_key=0;
    static int b_key=0;
    if(start){t_press=true;l_key=1;start=false;}
    tp.read();
    if(tp.isTouched){
      tp_x=tp.points[0].x;tp_y=tp.points[0].y;tp_size=tp.points[0].size;
      if(!more_menu && !menu && !swr_scan){
        if(BTN_F_LOW)       {t_press=true;show_par = "F__L";show_time=SHOW_VALUE+5;}
        if(BTN_F_HIGH)      {t_press=true;show_par = "F__H";show_time=SHOW_VALUE+5;}      
        if(BTN_RFPOWER)     {t_press=true;show_par = "POWR";show_time=SHOW_VALUE+5;}
        if(BTN_RMODE)       {l_key=6;t_press=true;tap_name="RF_MODE";}      
        if(BTN_STEP)        {l_key=5;t_press=true;tap_name="STEP";}
        if(BTN_AGC)         {l_key=12;t_press=true;tap_name="AGC";}
        if(BTN_OUT)         {l_key=14;t_press=true;tap_name="OUT";}
        if(BTN_TUN)         {l_key=15;t_press=true;tap_name="TUN";}
        if(BTN_MIC)         {l_key=16;t_press=true;tap_name="MIC";}
        if(BTN_MORE)        {l_key=18;t_press=true;tap_name="MORE";}
        if(BTN_ATT)         {l_key=17;t_press=true;tap_name="ATT";}
        if(BTN_TUNE)        {l_key=21;t_press=true;tap_name="TUNE";show_par = "TUNE";show_time=SHOW_VALUE;}
        
      }
      if(more_menu && !menu && !swr_scan){
        if(BTN_AUXI)       {t_press=true;show_par = "AUXI";show_time=SHOW_VALUE+5;}
        if(BTN_AUXO)       {t_press=true;show_par = "AUXO";show_time=SHOW_VALUE+5;}
        if(BTN_MICG)       {t_press=true;show_par = "MICG";show_time=SHOW_VALUE+5;}
        if(BTN_FLM)        {l_key=19;t_press=true;show_par = "L__M";show_time=SHOW_VALUE;}
        if(BTN_MORE)       {l_key=18;t_press=true;tap_name="MORE";}
        if(BTN_PTT)        {l_key=23;t_press=true;tap_name="CPTT";}
        if(BTN_RFG)        {t_press=true;show_par = "RF_G";show_time=SHOW_VALUE+5;}
        if(BTN_SCAN)       {l_key=8;t_press=true;tap_name="Scan";}
      }
      if(BTN_RIT && !swr_scan)         {l_key=7;t_press=true;}//режим отстройки
      if(BTN_FREQ_PLUS && !menu && !swr_scan)   {l_key=2;t_press=true;}//+ SR/4 kHz
      if(BTN_FREQ_PLUS_FULL && menu && !swr_scan)   {l_key=2;t_press=true;}//+ SR/4 kHz
      if(BTN_FREQ_MINUS && !menu && !swr_scan)  {l_key=3;t_press=true;}//- SR/4 kHz
      if(BTN_FREQ_MINUS_FULL && menu && !swr_scan)  {l_key=3;t_press=true;}//- SR/4 kHz
      if(BTN_VOL_PLUS && !menu && !swr_scan)    {l_key=9;t_press=true;show_par="VOL_";}//+
      if(BTN_VOL_PLUS_FULL && menu && !swr_scan)    {l_key=9;t_press=true;show_par="VOL_";}//+
      if(BTN_VOL_MINUS && !menu && !swr_scan)   {l_key=10;t_press=true;show_par="VOL_";}//-
      if(BTN_VOL_MINUS_FULL && menu && !swr_scan)   {l_key=10;t_press=true;show_par="VOL_";}//-
      if(BTN_SAVE_CONF && !swr_scan)   {l_key=11;t_press=true;show_par = "PAR_";}//сохранение текущих параметров

      if(BTN_SBAND){ //выбор диапазона
        int x=20;int y=80;
        for(int i=0;i<N_BANDS;i++){
          if(tp_x>x&&tp_x<x+50&&tp_y>y&&tp_y<y+20){numband=i; l_key=1;t_press=true;show_par = "BAND"; break;}
          x+=60;if(x>300){x=20;y+=30;}
        }
      }
      if(BTN_CONTROL){l_key=20;t_press=true;}
      if(BTN_BAND && !swr_scan){l_key=22;t_press=true;}
    }
    else{
      if(t_press&&l_key!=0){lkey=l_key;l_key=0;t_press=false;t_release=true;}
    }
}

void check_ptt(){
    if(send_carrier)ptt=1;else ptt=!digitalRead(PTT)|cat_ptt;
    if (ptt){
      txrx_mode = TX_MODE;
    }
    if (!ptt){
      txrx_mode = RX_MODE;
    }
}

void select_bandwidth(){
  static int old_mode;
  if(old_mode==rf_mode)return;
  if(rf_mode==LSB||rf_mode==USB){f_low.value=indent=200;f_high.value=bandwidth=2700;}
  if(rf_mode==L_D||rf_mode==U_D){f_low.value=indent = 100;f_high.value=bandwidth=3000;}
  if(rf_mode==CW||rf_mode==CWR){f_low.value=indent = 500;f_high.value=bandwidth=1000;}
  if(rf_mode==AM){f_low.value=indent = 100;f_high.value=bandwidth=6000;}
  if(rf_mode==MFM){f_low.value=indent = 100;f_high.value=bandwidth=6000;}
  old_mode=rf_mode;
}

void control() {
  check_ptt();
  change_band();
  t_touched();//опрос тачскина
  if (t_release) {t_release= false; action();}//если отпущена кнопка на экране вызов обработчика
  write_parameters(false);
  if(tun_mode == TUN) rx_freq=freq;
  select_bandwidth();
  power_button=digitalRead(POWER_PIN);
}

void readConfig() {
  /*номер     параметр
     0        test first
     1        значение ацп для кнопки №1
     2        значение ацп для кнопки №2
     3        значение ацп для кнопки №3
     4        значение ацп для кнопки №4
     5        значение ацп для кнопки №5
     6        значение ацп для кнопки №6
     7        значение ацп для кнопки №7
     8        номер диапазона
     9        частота настройки
     10       шаг перестройки
     11       rf_power
     12       agc (false=0,slow=1,fast=2,auto=3)
     13       mic_gain 0..20
     14       filter low
     15       filter high
     16       rf_gain
     
  */
  int i;
  for (i = 0; i < 8; i++) {
    value_button[i] = EEPROM.readUInt(i * sizeof(uint32_t));
    if (value_button[i] > 4095)value_button[i] = 9999;
  }
  numband = EEPROM.readUInt(i * sizeof(uint32_t));    if (numband > N_BANDS - 1 || numband < 0)numband = 0; i++;
  freq = EEPROM.readUInt(i * sizeof(uint32_t));       if (freq > 30000000 || freq < 100000)freq = bands[numband].freq; i++;
  numstep = EEPROM.readUInt(i * sizeof(uint32_t));    if (numstep > 5 || numstep < 0)numstep = 3; i++;
  rf_power= (uint8_t)(EEPROM.readUInt(i * sizeof(uint32_t)));    if (rf_power > 20 || rf_power < 1)rf_power = 1; i++;
  agc_speed = (uint8_t)(EEPROM.readUInt(i * sizeof(uint32_t)));    if (agc_speed > 3  || agc_speed <= 0)agc_speed = 1; i++;
  mic_gain= (uint8_t)(EEPROM.readUInt(i * sizeof(uint32_t)));    if (mic_gain > 20  || mic_gain <= 0)mic_gain = 10; i++;
  indent = (EEPROM.readUInt(i * sizeof(uint32_t)));    if (indent > 750  || indent < 100)indent = 200; i++;
  bandwidth = (EEPROM.readUInt(i * sizeof(uint32_t)));    if (bandwidth > 6000  || bandwidth <= indent+100)bandwidth = 2700; i++;
  rf_gain = (uint8_t)(EEPROM.readUInt(i * sizeof(uint32_t))); if (rf_gain > 100  || rf_gain < 1) rf_gain=50; i++;
  f_low.value=indent;f_high.value=bandwidth;
  tmp_rf_power=rf_power;
  tmp_mic_gain = mic_gain;
  tmp_rf_mode=rf_mode=bands[numband].mode;
} 


void writeConfig(uint32_t numpar, uint32_t value) {
  if(numpar==0)return;
  value_button[numpar] = value;
  EEPROM.writeUInt(numpar * sizeof(uint32_t), value);
  EEPROM.commit();
}

void write_parameters(bool first) {
  /*номер     параметр
   * 0        test first
     1..7     значения АЦП для кнопок
     8        номер диапазона
     9        частота настройки
     10       шаг перестройки
     11       rf_power 0..20
     12       agc 0..3
     13       mic_gain 0..20
     14       filter low
     15       filter high
     16       rf_gain
  */
  if(first){
    flag_write_parameters = true;
    numband = 8;
    freq=14200000;
    numstep=2;
    rf_power=1;
    agc_speed=1;
    mic_gain=1;
    indent=200;
    bandwidth=2700;
    f_low.value=indent;f_high.value=bandwidth;
    EEPROM.writeUInt(0 * sizeof(uint32_t), (uint32_t)0x5aa5);
    EEPROM.commit();
  }
  if (flag_write_parameters) {
    EEPROM.writeUInt(8  * sizeof(uint32_t), (uint32_t)numband);
    EEPROM.writeUInt(9  * sizeof(uint32_t), (uint32_t)freq);
    EEPROM.writeUInt(10 * sizeof(uint32_t), (uint32_t)numstep);
    EEPROM.writeUInt(11 * sizeof(uint32_t), (uint32_t)rf_power);
    EEPROM.writeUInt(12 * sizeof(uint32_t), (uint32_t)agc_speed);
    EEPROM.writeUInt(13 * sizeof(uint32_t), (uint32_t)mic_gain);
    EEPROM.writeUInt(14 * sizeof(uint32_t), (uint32_t)indent);
    EEPROM.writeUInt(15 * sizeof(uint32_t), (uint32_t)bandwidth);
    EEPROM.writeUInt(16 * sizeof(uint32_t), (uint32_t)rf_gain);
    EEPROM.commit();
    flag_write_parameters = false;
  }
}

void shift_fft(bool right,int shift_hz){
  int shift_points=shift_hz/((float)i2s_sample_rate_rx/(float)NUM_SAMPLE_BUF);
  int k=0;
  shift_en=false;
  if(right){
      for(int k=0;k<WP_LINE;k++){
        for(int i=0;i<NUM_SAMPLE_BUF;i++){
          if(i<NUM_SAMPLE_BUF-shift_points)wp[wp_num[k]][i]=wp[wp_num[k]][i+shift_points];
          else wp[wp_num[k]][i]=0;
        }
      }
  }else{
      for(int k=0;k<WP_LINE;k++){
        for(int i=NUM_SAMPLE_BUF-1;i>0;i--){
          if(i>shift_points)wp[wp_num[k]][i]=wp[wp_num[k]][i-shift_points];
          else wp[wp_num[k]][i]=0;
        }
   }
  }
  shift_en=true;
}

void x_encoder(void *args)
{
  while(1){
    int encoderDelta;
    pcnt_unit_get_count(pcnt_unit, &encoderDelta);
    pcnt_unit_clear_count(pcnt_unit);
    if (txrx_mode == RX_MODE) { //реакция на поворот энкодера на 0 экране
     if(show_par=="VOL_"){
      if (encoderDelta > 0){volume+=1;if(volume>28)volume=28;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){volume-=1;if(volume>28 )volume=0;show_time=SHOW_VALUE;}
      }
     if(show_par=="F__L"){
      if (encoderDelta > 0){indent+=50;if(indent>750)indent=750;f_low.value=indent;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){indent-=50;if(indent <100 )indent=100;f_low.value=indent;show_time=SHOW_VALUE;}
     }
     if(show_par=="F__H"){
      if (encoderDelta > 0){bandwidth+=50;if(bandwidth>6000)bandwidth=6000;f_high.value=bandwidth;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){bandwidth-=50;if(bandwidth<indent+200)bandwidth=indent+200;f_high.value=bandwidth;show_time=SHOW_VALUE;}
     }
     if(show_par=="POWR"){
      if (encoderDelta > 0){rf_power+=1;if(rf_power>20)rf_power=20;tmp_rf_power=rf_power;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){rf_power-=1;if(rf_power<1)rf_power=1;tmp_rf_power=rf_power;show_time=SHOW_VALUE;}
     }
     if(show_par=="AUXI"){
      if (encoderDelta > 0){auxi+=1;if(auxi>15)auxi=15;tmp_auxi=auxi;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){auxi-=1;if(auxi<0)auxi=0;tmp_auxi=auxi;show_time=SHOW_VALUE;}
     }
     if(show_par=="AUXO"){
      if (encoderDelta > 0){auxo+=1;if(auxo>15)auxo=15;tmp_auxo=tmp_auxo;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){auxo-=1;if(auxo<0)auxo=0;tmp_auxo=tmp_auxo;show_time=SHOW_VALUE;}
     }
     if(show_par=="MICG"){
      if (encoderDelta > 0){mic_gain+=1;if(mic_gain>20)mic_gain=20;tmp_mic_gain=mic_gain;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){mic_gain-=1;if(mic_gain<0)mic_gain=0;tmp_mic_gain=mic_gain;show_time=SHOW_VALUE;}
     }
     if(show_par=="RF_G"){
      if (encoderDelta > 0){rf_gain+=1;if(rf_gain>100)rf_gain=100;show_time=SHOW_VALUE;}
      if (encoderDelta < 0){rf_gain-=1;if(rf_gain<1)rf_gain=1;show_time=SHOW_VALUE;}
     }
     if(show_par=="____"){
      if ((encoderDelta > 0) && (tun_mode == TUN)){shift_fft(true,step_freq[numstep]);freq = freq + step_freq[numstep];if (freq > 39999999)freq = 39999999;bands[numband].freq=freq;rx_freq = freq;}
      if ((encoderDelta < 0) && (tun_mode == TUN)){shift_fft(false,step_freq[numstep]);freq = freq - step_freq[numstep];if (freq < 100000)freq = 100000;bands[numband].freq=freq;rx_freq = freq;}
      if ((encoderDelta > 0) && (tun_mode == RIT)){shift_fft(true,step_freq[numstep]);rx_freq = rx_freq + step_freq[numstep];if (rx_freq > 39999999)rx_freq = 39999999;bands[numband].freq=rx_freq;}
      if ((encoderDelta < 0) && (tun_mode == RIT)){shift_fft(false,step_freq[numstep]);rx_freq = rx_freq - step_freq[numstep];if (rx_freq < 100000)freq = 100000;bands[numband].freq=rx_freq;}
      }
    }
    if (txrx_mode == SETUP_MODE) { //реакция на поворот энкодера на 2 экране
        if (encoderDelta > 0) {n_button++;if (n_button > 8)n_button = 8;redraw = true;}
        if (encoderDelta < 0) {n_button--;if (n_button < 1)n_button = 1;redraw = true;}
     }
     vTaskDelay(1);
    }
}
