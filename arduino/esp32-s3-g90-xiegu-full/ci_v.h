

#define BROADCAST_ADDRESS    0x00 //Broadcast address
#define CONTROLLER_ADDRESS   0xE0 //Controller address

#define START_BYTE       0xFE //Start byte
#define STOP_BYTE       0xFD //Stop byte

#define CMD_TRANS_FREQ      0x00 //Transfers operating cat_frequency data
#define CMD_TRANS_MODE      0x01 //Transfers operating mode data

#define CMD_READ_FREQ       0x03 //Read operating cat_frequency data
#define CMD_READ_MODE       0x04 //Read operating mode data

#define CMD_PTT             0x1C 

#define CMD_WRITE_FREQ       0x05 //Write operating cat_frequency data
#define CMD_WRITE_MODE       0x06 //Write operating mode data
#define OK                   0xFB
#define NG                   0xFA
uint8_t  radio_address     = 0x88;//xiegu g90 cat-identifier

const uint32_t decMulti[]    = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
uint8_t  read_buffer[12];
uint8_t ok[] = {START_BYTE, START_BYTE, 0, radio_address, OK, STOP_BYTE};
uint8_t ng[] = {START_BYTE, START_BYTE, 0, radio_address, NG, STOP_BYTE};

uint8_t convertByteToBCD(uint8_t value)
    {
      return (uint8_t)((value / 10 * 16) + (value % 10));
    }    

void uint32_to_bcd(uint32_t input, uint8_t output[5])
{
    for(uint8_t i = 0; i < 5; ++i) {
        output[i] = 0;
    }
    for(uint8_t i = 0; i < 32; ++i) {
        for(uint8_t j = 0; j < 5; ++j) {
            uint8_t w = output[j];
            uint8_t x = w & 0xF0;
            uint8_t y = w & 0x0F;
            if(y >= 5) {
                y = (y + 3) & 0xf;
            }
            if(x >= 0x50) {
                x += 0x30;
            }
            output[j] = x | y;
        }
        uint8_t spill = 0;
        if((input & 0x80000000) != 0) {
            spill = 1;
        }
        input <<= 1;
        for(int8_t j = 4; j >= 0; --j) {
            uint8_t new_spill = ((output[j] & 0x80) != 0) ? 1 : 0;
            output[j] = (output[j] << 1) | spill;
            spill = new_spill;
        }
    }
}

uint8_t readLine(void){
  uint8_t byte;
  uint8_t counter = 0;
  uint32_t readtimeout = 30000;
  uint32_t ed = readtimeout;
  
  while (true)
  {
    while (!Serial2.available()) {
      if (--ed == 0)return 0;
    }
    ed = readtimeout;
    byte = Serial2.read();
    if (byte == 0xFF)continue;
    read_buffer[counter++] = byte;
    if (STOP_BYTE == byte) break;
    if (counter >= sizeof(read_buffer))return 0;
  }
  return counter;
}

uint8_t get_Mode(uint8_t address){
  if(read_buffer[5] == 0xFD){
    ok[2]=address;
    Serial2.write(ok,sizeof(ok));
    return read_buffer[4];
  }else{return 0xff;}
  
}

uint32_t get_Frequency(uint8_t address)
{
  uint32_t cat_frequency = 0;
  if(read_buffer[9] != 0xFD){return 0;}
  else{
  for (uint8_t i = 0; i < 5; i++) {
    if(read_buffer[9 - i] == 0xFD)continue;
    cat_frequency += (read_buffer[9 - i] >> 4) * decMulti[i * 2];
    cat_frequency += (read_buffer[9 - i] & 0x0F) * decMulti[i * 2 + 1];}
    ok[2]=address;
    Serial2.write(ok,sizeof(ok));
    return cat_frequency;
  }
}

void send_PTT(uint8_t address){
 if(read_buffer[5] == 0xFD){
    uint8_t req[] = {START_BYTE, START_BYTE, address, radio_address, (uint8_t)ptt, STOP_BYTE};
    Serial2.write(req,sizeof(req));
 }else{
  if(read_buffer[5] == 0 && read_buffer[7] == 0xFD){
    cat_ptt = read_buffer[6];
    ok[2]=address;
    Serial2.write(ok,sizeof(ok));
  }
 }
}

void send_Mode(uint8_t address)
{
  uint8_t cat_mode=0;
  //0-lsb,1-usb,2-am,3-cw,4-cwr
  switch (rf_mode){
    case LSB: cat_mode=0;break;
    case USB: cat_mode=1;break;
    case AM: cat_mode=2;break;
    case CW: cat_mode=4;break;
    case CWR: cat_mode=7;break;
    default: break;
  }
  uint8_t req[] = {START_BYTE, START_BYTE, address, radio_address, cat_mode, STOP_BYTE};
  Serial2.write(req,sizeof(req));
}

void send_Frequency(uint8_t address){
  uint8_t bcd_freq[5];
  uint8_t req[] = {START_BYTE, START_BYTE, address, radio_address, 0,0,0,0,0, STOP_BYTE};
  uint32_to_bcd(freq,bcd_freq);
  for(int i=0;i<5;i++){
    req[8-i]=bcd_freq[i];
  }
  Serial2.write(req,sizeof(req));
}

void req_data_cat(){

  while (Serial2.available()) {
    if (readLine() > 0) {
      if (read_buffer[0] == START_BYTE && read_buffer[1] == START_BYTE) {
        if (read_buffer[2] == radio_address) {
            uint8_t req_address = read_buffer[3];
            switch (read_buffer[4]) {
              case CMD_READ_FREQ: //запрос частоты
                {send_Frequency(req_address);//передать контроллеру/компьютеру текущую частоту
                break;}
              case CMD_READ_MODE: //запрос rf_mode
                {send_Mode(req_address);//передать контроллеру/компьютеру текущий режим
                break;}
              case CMD_PTT:{send_PTT(req_address);break;}
              case CMD_WRITE_FREQ:
                {uint32_t tmp_freq = get_Frequency(req_address); //установка частоты
                freq = (tmp_freq)?tmp_freq:freq;
                break;}
              case CMD_WRITE_MODE:
                {uint8_t tmp_mode = get_Mode(req_address); //установка rf_mode
                tmp_rf_mode=rf_mode = (tmp_mode!=0xff)?tmp_mode:rf_mode;
                break;}
              default:break;
          }
        }
      }
    }
  }
}

void send_cat_broadcast(){
  send_Frequency(BROADCAST_ADDRESS);
  vTaskDelay(100);
  send_Mode(BROADCAST_ADDRESS);
  send_cat=false;
}

static void x_cat(void *args) {

 while(1){
      req_data_cat();//слушаем cat и выполняем запросы
      if(send_cat){
        send_cat_broadcast();//отправляем в cat изменение частоты/режима(при изменении на трансивере)
      }
      vTaskDelay(100);
 }
}
