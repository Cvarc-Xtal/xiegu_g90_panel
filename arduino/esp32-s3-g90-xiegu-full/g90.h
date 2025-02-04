#include "crc32.h"
#include "driver/uart.h"

#define SIZE_BODY_BUF 372 //размер принимаемого из g90_body массива
#define SIZE_HEAD_BUF 96  //размер передаваемого массива в g90_body
//массив для приема данных из g90_body
uint8_t from_body[SIZE_BODY_BUF]={0xa1,0x00,0x00,0x86,0x00,0x00,0x79,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x45,0x20,0x45,0x20,0x45,0x20,0x45,0x20,0x45,0x20,0x45,0x20,0x49,0x20,0x45,0x20,0x45,0x45,0x20,0x45,0x20,0x45,0x20,0x45,0x20,0x45,0x20,0x00,0x03,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x08,0x00,0x04,0x00,0x09,0x00,0x06,0x00,0x00,0x00,0x01,0x00,0x08,0x00,0x05,0x00,0x0e,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x05,0x00,0x00,0x00,0x01,0x00,0x02,0x00,0x02,0x00,0x03,0x00,0x02,0x00,0x02,0x00,0x06,0x00,0x0a,0x00,0x08,0x00,0x03,0x00,0x05,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x06,0x00,0x06,0x00,0x07,0x00,0x14,0x00,0x0b,0x00,0x03,0x00,0x16,0x00,0x0c,0x00,0x0b,0x00,0x07,0x00,0x13,0x00,0x07,0x00,0x06,0x00,0x02,0x00,0x03,0x00,0x03,0x00,0x01,0x00,0x05,0x00,0x01,0x00,0x06,0x00,0x05,0x00,0x08,0x00,0x05,0x00,0x01,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x08,0x00,0x02,0x00,0x05,0x00,0x05,0x00,0x07,0x00,0x03,0x00,0x07,0x00,0x06,0x00,0x03,0x00,0x03,0x00,0x07,0x00,0x06,0x00,0x06,0x00,0x04,0x00,0x03,0x00,0x00,0x00,0x07,0x00,0x03,0x00,0x0a,0x00,0x07,0x00,0x06,0x00,0x07,0x00,0x06,0x00,0x02,0x00,0x06,0x00,0x05,0x00,0x01,0x00,0x07,0x00,0x06,0x00,0x07,0x00,0x05,0x00,0x06,0x00,0x06,0x00,0x05,0x00,0x01,0x00,0x0d,0x00,0x05,0x00,0x0f,0x00,0x0e,0x00,0x05,0x00,0x06,0x00,0x0b,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x07,0x00,0x00,0x00,0x03,0x00,0x08,0x00,0x03,0x00,0x02,0x00,0x03,0x00,0x05,0x00,0x03,0x00,0x01,0x00,0x05,0x00,0x00,0x00,0x02,0x00,0x04,0x00,0x01,0x00,0x02,0x00,0x03,0x00,0x06,0x00,0x01,0x00,0x00,0x00,0x03,0x00,0x04,0x00,0x04,0x00,0x01,0x00,0x02,0x00,0x00,0x00,0x05,0x00,0x03,0x00,0x05,0x00,0x00,0x00,0x9a,0x8c,0x88,0x76};
uint8_t array_to_g90[SIZE_HEAD_BUF]; //массив для отправки в g90_body



//переменные в которые пишутся параметры, принимаемые из body
uint8_t smeter_g90;   //текущий уровень приема  ( (256-from_body[0]) десятичное значение со знаком минус)
float txpower_g90;  //текущий уровень мощности передатчика (при передаче) from_body[1]/10 в ваттах с десятичным знаком
uint8_t swr_s90;      //КСВ при передаче from_body[2] Принимает значения 0..44
float power_g90;    //Десятичное значение напряжение питания from_body[3]/10 в вольтах с десятичным знаком
uint8_t alc_g90;      //ALC в процентах.Значения from_body[5] 0..255 сопоставить со значениями 0..100
uint8_t version_g90;



//структура параметров для отправки в body
struct g90_body{
  uint8_t head[2]={0x55,0xaa};
  uint16_t pad1=0;
  //#4
  uint32_t freq1=0;//VFOa
  //#8
  uint8_t att_mode = 0;   //0=none,pre=1,att=2
  uint8_t modulation = 0; //lsb=0,l_d=1,usb=2,u_d=3,cw=4,cwr=5,mfm=6,am=7
  uint8_t agc = 1;        //false=0,slow=1,fast=2,auto=3
  uint8_t pad2 = 0;
  //#12
  uint8_t filter_high_raw = 59;//верхний склон фильтра
  uint8_t filter_low_raw = 5;  //нижний склон фильтра
  //#14
  uint8_t pad3[5]={0x00, 0x00, 0x00, 0x00, 0x00};
  //#19
  uint8_t fft_scale = 0;  //1=auto
  uint32_t freq2 = 7050000;//VFOb
  //#24
  uint8_t att_mode2 = 1;//0=none,pre=1,att=2
  uint8_t modulation2 = 0;//
  uint8_t agc2 = 3;
  uint8_t pad2b = 0;
  //#28
  uint8_t filter_high_raw2 = 59;
  uint8_t filter_low_raw2 = 5;
  //#30
  uint8_t pad5[5]={0x00,0x00,0x00,0x00,0x00};
  //#35
  uint8_t fft_scale2 = 1;

  uint8_t ctrl1 = 0b00101000;//bit7 - transmit
                             //bit6 - mem_en
                             //bit5 - tuner_en
                             //bit4 - nb_en
                             //bit3 - mic_compression
                             //bit2 - output_headphones 
                             //bit1 - split_en
                             //bit0 - panel_lock

  //#37
  uint8_t ctrl2 = 0b00000000;//bit7 - shutdown_req
                             //bit5 - tuning
                             //остальные биты = 0
  //#38
  uint8_t ctrl3 = 0b00000011;//bit7..5 - rclk_raw_low
                             //bit4 - cw_disp_en
                             //bit3 - vox_en
                             //bit2 - audio_in_line_en
                             //bit1 - tx_disable
                             //bit0 - cw_qsk
  uint8_t rclk_raw_high = 125;
  
  //#40
  uint8_t ctrl4_vox0 = 0b00000000;//bit7    - vox_anti_gain_low
                                  //bit6..0 - vox_gain
  uint8_t ctrl4_vox1 = 0b00000000;//bit7..6 - vox_delay_low
                                  //bit5..0 - vox_anti_gain_high
  uint8_t ctrl4_vox2 = 0b10010000;//bit7..3 - gain_low
                                  //bit2..0 - vox_delay_high
  uint8_t ctrl4_other = 0b00001001;//bit7..4 = 0
                                  //bit3 - beep_en
                                  //bit2 - band_stack_full
                                  //bit1..0 gain_high
  //#44
  uint8_t rf_power = 20;
  uint8_t sql_level = 0;
  uint8_t ctrl5 = 0b01010000;//bit7..4 - nb_level
                             //bit4..0 - nb_width
  uint8_t blank1;
  
  //#48
  uint8_t volume = 1;//0-28
  uint8_t mic_gain = 10;//0-20
  uint8_t unknown1 = 0x10;
  uint8_t cq_qsk_time_raw = 2;//0-20
  //#52
  uint8_t unknown2 = 2;
  uint8_t mem_ch = 0;//0-20
  
  //#54
  uint8_t ctrl6 = 0b00000000;//bit0 - vfo_b_en. Остальные биты = 0
  uint8_t blank2 = 0;
  uint8_t unknown3[4] = {0xff, 0xff, 0xff, 0xff};
  
  //#60
  uint8_t cw_wpm = 15;//5-50 wpm
  uint8_t ctrl7 = 0b10100000;//bit7..4 - cw_ratio_raw
                             //bit3=0
                             //bit2 - cw_mode_b
                             //bit1..0 - cw_mlr
                             
  uint8_t unknown4[20] = {0xff,0x0f,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1f,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x0a,0x32};

  //#82
  uint8_t ctrl8_swr = 6;//bit3..0 - swr_threshold_raw
  uint8_t ctrl8_aux = 0b11111000;//bit7..4 aux_out_vol
                                 //bit3..0 aux_in_vol
  //#84
  uint8_t ritval_low = 0;  //значащие все биты
  uint8_t ritval_high = 0; //значащие два мл.бита
  uint8_t unknown5[6] = {0xff, 0xff, 0x00, 0x00, 0x00, 0x00};
  //#92
  uint32_t checksum = 0;
}typedef g90;

g90 data_to_g90;

void send_data_to_body(){//готовим и отправляем пакет данных на основе структуры g90
  uint8_t *ptr_i=(uint8_t*)(&data_to_g90);
  uint8_t *ptr_o=(uint8_t*)(&array_to_g90[0]);
  uint8_t tmp[SIZE_HEAD_BUF];//временный массив для подсчета crc32-mpeg2
  //копируем в рабочий массив все параметры
  memcpy(ptr_o,ptr_i,SIZE_HEAD_BUF);
  //копируем все принятые данные во временный массив 
  //с переворотом каждых 4 байт: 1,2,3,4 -> 4,3,2,1 (требуется для crc32mpeg2)
  for(int i=0;i<sizeof(array_to_g90);i+=4){
    tmp[i]=array_to_g90[i+3];
    tmp[i+1]=array_to_g90[i+2];
    tmp[i+2]=array_to_g90[i+1];
    tmp[i+3]=array_to_g90[i];
  }

  //считаем crc32 данных временного массива и помещаем в общую структуру
  data_to_g90.checksum = calc_crc32mpeg2((uint8_t *)tmp, SIZE_HEAD_BUF-4);
  
  //снова копируем в рабочий массив все параметры уже с контрольной суммой
  memcpy(ptr_o,ptr_i,SIZE_HEAD_BUF);
  
  //переворот байт контрольной суммы: 1,2,3,4 -> 4,3,2,1 (требуется для crc32mpeg2)
  array_to_g90[95] = (uint8_t)(data_to_g90.checksum)&0x000000ff;
  array_to_g90[94] = (uint8_t)(data_to_g90.checksum>>8)&0x000000ff;
  array_to_g90[93] = (uint8_t)(data_to_g90.checksum>>16)&0x000000ff;
  array_to_g90[92] = (uint8_t)(data_to_g90.checksum>>24)&0x000000ff;
  //рабочий массив данных отправляется в нормальном (в порядке следования байт) виде.
  //Изменяется порядок байт только для контрольной суммы

  //отправляем подготовленный массив с параметрами в тело g90 
  Serial1.write(array_to_g90,SIZE_HEAD_BUF);
}


uint32_t get_crc_body(){
  uint8_t tmp[SIZE_BODY_BUF];
  uint8_t *ptr_i=(uint8_t*)(&from_body);
  uint8_t *ptr_o=(uint8_t*)(&tmp[0]);
  memcpy(ptr_o,ptr_i,SIZE_BODY_BUF);
  //копируем все принятые данные во временный массив 
  //с переворотом каждых 4 байт: 1,2,3,4 -> 4,3,2,1 (требуется для crc32mpeg2)
  for(int i=0;i<SIZE_BODY_BUF;i+=4){
    tmp[i]=from_body[i+3];
    tmp[i+1]=from_body[i+2];
    tmp[i+2]=from_body[i+1];
    tmp[i+3]=from_body[i];
  }
  uint32_t checksum = calc_crc32mpeg2((uint8_t *)tmp, SIZE_BODY_BUF-4);
  return checksum;
}

bool get_data_from_body(){
uint32_t check_crc,crc_from_g90;
const uart_port_t uart_num = UART_NUM_0;
////////////проверить/////////////////работает!
  size_t length;
  uart_get_buffered_data_len(uart_num, &length);
  if(length < SIZE_BODY_BUF) uart_flush_input(uart_num);
//////////////////////////////////////
uart_read_bytes(uart_num, from_body, SIZE_BODY_BUF, 100);
 
  check_crc=0;
  //переворачиваем байты принятой контрольной суммы (1-2-3-4 -> 4-3-2-1)
   for(int i=0;i<4;i++){
      uint32_t tmp = from_body[SIZE_BODY_BUF-1-i];tmp=tmp<<i*8;
      crc_from_g90 |= tmp;
   }
   check_crc = get_crc_body();//вычисляем контрольную сумму принятого пакета..
   if(check_crc == crc_from_g90)return true;//..и сравниваем с принятой суммой
   else return false;
}
