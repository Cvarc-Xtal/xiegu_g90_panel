void fft_init(){
      for(int i=WP_LINE;i>0;i--){wp_num[i-1]=i-1;} //нумеруем массив номеров строк "водопада"
}

uint16_t hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v)
{
    h = (uint32_t)((uint32_t)h * 255) / 360; //color      [0..359]
    s = (uint16_t)((uint16_t)s * 255) / 100; //saturation [0..100]
    v = (uint16_t)((uint16_t)v * 255) / 100; //value      [0..100]
    uint8_t r, g, b;
    uint8_t region, remainder, p, q, t;
    if(s == 0) {
        return (uint16_t)((v >> 3) | ((v >> 2) << 5) | ((v >> 3) << 11));
    }
    region    = h / 43;
    remainder = (h - (region * 43)) * 6;
    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;
    switch(region) {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }
    uint16_t result = (r >> 3) | ((g >> 2) << 5) | ((b >> 3) << 11);
    return result;
}

uint16_t select_color(int val,int max){
  
  uint16_t h = map(val,0,max,5,101);
  uint8_t  s = 100;
  uint8_t  v = map(h,0,100,20,100);
  return hsv_to_rgb(h,s,v);  
}

void fft_for_display(uint8_t* input){
    sum_fft = 0.0f;
    max_fft = 0.0f;
    min_fft = 1000000000.0f;
    uint8_t fft_value[NUM_SAMPLE_BUF];
    for (int i = 0 ; i < NUM_SAMPLE_BUF; i++) {
      if(input[i]>=limited_fft)input[i]=limited_fft;
      if (max_fft < input[i])max_fft=input[i];
      if (min_fft > input[i])min_fft=input[i];
      fft_value[i]=input[i];
      //копирование магнитуд в отображаемый буфер,элементы которого постоянно уменьшаются
      if(input[i]<=fft_inter[i]) input[i]=fft_inter[i];
      if(input[i]>fft_inter[i])fft_inter[i] = input[i];
      //заполняем верхнюю строку массива для отображения "водопада"
      wp_value[i]=select_color(input[i],limited_fft);
      wp[wp_num[0]][i]=select_color(fft_value[i],limited_fft);
      sum_fft = sum_fft+fft_value[i];
    }
}
