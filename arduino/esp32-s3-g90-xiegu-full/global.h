const char *ssid = "Dlink123";       
const char *password = "Rt4swe123";  
const char* ntpServer = "ntp0.ntp-servers.net";
const long  gmtOffset_sec = 0; 

int xwin=0;//начало координат экрана(если экран больше 480х272)
int ywin=0;

//uart 
#define RXPIN0 (gpio_num_t)(44)  //прием данных (g90->панель)
#define TXPIN1 (gpio_num_t)(43)  //передача данных (панель->g90)
#define TXPIN2 (gpio_num_t)(6)   //передача данных по cat (CI-V)
#define RXPIN2 (gpio_num_t)(7)   //прием данных по cat (CI-V)

#define PTT (gpio_num_t)(15)

//Encoder pins
#define ROTARY_ENCODER_A_PIN      (gpio_num_t)(17)
#define ROTARY_ENCODER_B_PIN      (gpio_num_t)(18)
#define ROTARY_ENCODER_BUTTON_PIN (gpio_num_t)(16)

#define POWER_PIN ROTARY_ENCODER_BUTTON_PIN

//пины и размер встроенного тачскрина. Не менять для панели JC4827W543
#define TOUCH_SDA  (gpio_num_t)8
#define TOUCH_SCL  (gpio_num_t)4
#define TOUCH_INT  (gpio_num_t)3
#define TOUCH_RST  (gpio_num_t)38
#define TOUCH_WIDTH  480
#define TOUCH_HEIGHT 272

//LCD  Не менять для панели JC4827W543
#define LCD_BL  (gpio_num_t)1
#define LCD_CS  (gpio_num_t)45
#define LCD_SCK (gpio_num_t)47
#define LCD_D0  (gpio_num_t)21
#define LCD_D1  (gpio_num_t)48
#define LCD_D2  (gpio_num_t)40
#define LCD_D3  (gpio_num_t)39
#define LCD_RST (gpio_num_t)-1

#define STACK_SIZE 2048

//параметры драйвера дисплея. Не менять для панели JC4827W543
Arduino_DataBus *bus = new Arduino_ESP32QSPI(LCD_CS,LCD_SCK,LCD_D0,LCD_D1,LCD_D2,LCD_D3);
Arduino_GFX *g = new Arduino_NV3041A(bus, LCD_RST, 0 /* rotation */, true /* IPS */);
Arduino_GFX *gfx = new Arduino_Canvas(480 /* width */, 272 /* height */, g);

int fps;
int t_fps=99;
bool mark = false;
bool draw_info=true;
bool test;

float sum_fft = 0.0f;
float max_fft = 0.0f;
float min_fft = 10000000.0f;
float s_level = 0.0f;

struct tm timeinfo;
const int   daylightOffset_sec = 0;


uint32_t i2s_sample_rate_rx = 48000;    //частота дискретизации кодеков в g90

#define NUM_SAMPLE_BUF 480 

void readConfig();
void writeConfig(uint32_t numpar, uint32_t value);
void write_parameters(bool first);

// for touch
bool tp_control = true;
bool tp_band = false;
TAMC_GT911 tp = TAMC_GT911(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH, TOUCH_HEIGHT);
int tp_x,tp_y,tp_size;

#define WP_LINE 81  //кол-во строк "водопада"
uint16_t wp[WP_LINE][NUM_SAMPLE_BUF];//массив строк для водопада[кол-во строк][кол-во точек в строке]
uint16_t wp_value[NUM_SAMPLE_BUF];
uint8_t wp_num[WP_LINE];          //массив для хранения порядка номеров строк водопада для вывода на экран
uint8_t fft_g90[NUM_SAMPLE_BUF];    //рабочий fft-буфер для g90
uint8_t fft_inter[NUM_SAMPLE_BUF]; //отображаемый fft буфер (все элементы постоянно уменьшаются)

#define PCH (NUM_SAMPLE_BUF/2) //середина отображаемомго спектра
int pos_fft = PCH; //позиция указателя на панораме/спектре
float hz=(float)i2s_sample_rate_rx/(float)NUM_SAMPLE_BUF;
int limited_fft = 32; //10000000;

uint8_t volume = 1;
bool mic_c = true;
bool mic_line = true;
uint8_t agc_speed = 1;
uint8_t shutdown_req=0;
const char* agc[4]={"---","Slow","Fast","Auto"};
float swr_threshold = 3.0; //порог КСВ для автотюнера
uint8_t rf_power = 1;
uint8_t tmp_rf_power = 1;
uint8_t rf_mode = 0;
uint8_t tmp_rf_mode = 0;
uint8_t mic_gain = 10;
uint8_t tmp_mic_gain = 10;
uint8_t auxi = 8;
uint8_t tmp_auxi = 8;
uint8_t auxo = 15;
uint8_t tmp_auxo = 15;
uint8_t tun_mode =0;//0-tuning, 1-rit
uint8_t num_filter = 0; //номер текущего фильтра основной селекции
int smeter = 0;
int old_smeter = 0;
int tmp_smeter = 0;
uint32_t freq = 14200000; //частота настройки
uint32_t rx_freq = freq; //частота приема (RIT)
uint32_t step_freq[5] = {50,100,500,1000,5000}; //шаг перестройки гц
uint32_t numband = 8;//номер текущего диапазона
uint32_t numstep = 2;
bool speaker = true;
bool tuner   = true;
bool tuning  = false;
bool send_carrier = false;
bool send_cat = true;
int bandwidth; //текущая полоса пропускания, отображаемая на спектре
int indent;     
int ptt=0;
int cat_ptt=0;

uint16_t colors[] = { //
  0b1000000000000000, //0 
  0b1100000000000000, //1
  0b1000010000000000, //2
  0b1100010000000000, //3
  0b1000011000000000, //4
  0b1100011000000000, //5
  0b0000010000000000, //6
  0b0000011000000000, //7
  0b0000011000010000, //8
  0b0000010000010000, //9
  0b0000000000010000, //10
  0b0000000000011000, //11
  0b1000000000010000, //12
  0b1000000000011000, //13
  0b1100000000011000, //14
  0b1000010000011000, //15
  0b1000011000010000, //16
  0b1100010000010000, //17
  0b1000010000010000, //18
  0b1100011000011000  //19
  };

#define GRAY    0b0011100111100111


unsigned long cur_ms   = 0;

uint16_t in_left = 999; //знач.ацп лев.кнопок
bool l_release = false; //признак отпущенной лев.кнопки
bool t_release = false; //признак отпущенной кнопки на экране
bool l_press = false;   //признак нажатой лев.кнопки
bool t_press = false;   //признак нажатой кнопки на экране

uint8_t lkey = 0;       //код лев.кнопки для дальн.действия 1..5 (0-нет действия)

bool redraw = false;
int power_button = 1;
bool wifi = false;
bool locked = true;
int n_button = 0;
uint16_t value_button[8]={0,1985,2672,3028,3266,3438,3571,3677};
bool flag_exit_setup=false;
uint16_t value_adc = 0;
bool flag_write_config = false;
bool flag_write_parameters = false;
bool flag_start_ok = false;

#define SHOW_VALUE 3  //время показа параметров сек
const char* show_par="    ";
int show_time = SHOW_VALUE;

const int LSB = 0; //lsb=0,l_d=1,usb=2,u_d=3,cw=4,cwr=5,mfm=6,am=7
const int L_D = 1;
const int USB = 2;
const int U_D = 3;
const int CW =  4;
const int CWR = 5;
const int MFM = 6;
const int AM  = 7;
const int TX_MODE = 0;
const int RX_MODE = 1;
const int SETUP_MODE = 2;
const int TUN = 0;
const int RIT = 1;

uint8_t txrx_mode = RX_MODE;      //1-прием, 0-передача, 2-настройка

struct band {
  uint32_t freq; // frequency in Hz
  char* name;        // name of band
  uint32_t mode;
  uint8_t att_mode;
};

#define N_BANDS 13
struct band bands[N_BANDS] = {
  1850000,(char *)"160M" ,LSB,2,
  3700000,(char *)"80M " ,LSB,2,
  3995000,(char *)"75M " ,AM,0,
  4850000,(char *)"60M " ,LSB,0,
  5840000,(char *)"49M " ,AM,0,
  7100000,(char *)"40M " ,LSB,0,
  9520000,(char *)"31M " ,AM,0,
  11670000,(char *)"25M ",AM,0,
  14200000,(char *)"20M ",USB,1,
  17780000,(char *)"16M ",AM,1,
  21200000,(char *)"15M ",USB,1,
  24920000,(char *)"12M ",AM,1,
  28350000,(char *)"10M ",USB,1
};
///////////////////////////////////////////////
//описания экранных кнопок для тачскрина
bool more_menu = false;
int show_tap = 0;
const char* tap_name = (char*)"_______";
struct {
  char* b_name =(char*)"Filt_low";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=72+ywin;
  int y_max=y_min+h+ywin;
  int value;
  }f_low;

struct {
  char* b_name =(char*)"Filt_high";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=72+ywin;
  int y_max=y_min+h+ywin;
  int value;
  }f_high;

struct {
  char* b_name =(char*)"RF_MODE";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=105+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"Mode";
  }rfmode;

struct {
  char* b_name =(char*)"STEP";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=105+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"Step";
  }fstep;

struct {
  char* b_name =(char*)"AGC";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=138+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"AGC";
  }fagc;

struct {
  char* b_name =(char*)"POWER";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=138+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"Watt";
  }fpower;

struct {
  char* b_name =(char*)"OUT";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=171+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"Spkr";
  }fout;

struct {
  char* b_name =(char*)"TUN";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=171+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"Tun";
  }ftun;

struct {
  char* b_name =(char*)"MIC";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=204+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"MicC";
  }fmic;

struct {
  char* b_name =(char*)"ATT";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=204+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"ATT";
  }fatt;

struct {
  char* b_name =(char*)"MORE";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=237+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"MORE";
  }fmore;

struct {
  char* b_name =(char*)"TUNE";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=237+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"TUNE";
  }ftune;

struct {
  char* b_name =(char*)"CPTT";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=237+ywin;//+33
  int y_max=y_min+h+ywin;
  char* value = (char*)"CPTT";
  }fptt;


struct {
  char* b_name =(char*)"AuxI";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=72+ywin;
  int y_max=y_min+h+ywin;
  char* value = (char*)"AuxI";
  }fauxi;

struct {
  char* b_name =(char*)"AuxO";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=72+ywin;
  int y_max=y_min+h+ywin;
  char* value = (char*)"AuxO";
  }fauxo;

struct {
  char* b_name =(char*)"MicG";
  int w=50;
  int h=30;
  int x_min=370+xwin;
  int x_max=x_min+w+xwin;
  int y_min=105+ywin;
  int y_max=y_min+h+ywin;
  char* value = (char*)"MicG";
  }fmicg;

struct {
  char* b_name =(char*)"L/M";
  int w=50;
  int h=30;
  int x_min=430+xwin;
  int x_max=x_min+w+xwin;
  int y_min=105+ywin;
  int y_max=y_min+h+ywin;
  char* value = (char*)"L/M";
  }flm;


struct {
  char* b_name =(char*)"BAND";
  int w=130;
  int h=20;
  int x_min=260+xwin;
  int x_max=x_min+w+xwin;
  int y_min=10+ywin;
  int y_max=y_min+h+ywin;
  int value;
  }tp_nband;


struct {
  char* b_name =(char*)"RIT";
  int w=120;
  int h=30;
  int x_min=260+xwin;
  int x_max=x_min+w+xwin;
  int y_min=50+ywin;
  int y_max=y_min+h+ywin;
  int value;
  }tp_rit;
