
#include "Defines.h"
#define PSNUMBER 2         // Количество подключенных источников
#define COUNT_PRESET 5     // Количество пресетов и "плиток для них" напряжения в каждом источнике 
#define COUNT_RECT_MAIN 4  // Количество плиток для основного интерфейса 


#if (ENABLE_DEBUG_BLUETOOTH == 1)  
#include "BluetoothSerial.h"
const char *pin = "1234"; // Change this to more secure PIN.
String device_name = "Controller_PS_LCD";
BluetoothSerial SerialBT;
#endif

TaskHandle_t Task1;
TaskHandle_t Task2;
TaskHandle_t Task3; 
TaskHandle_t Task4; 
TaskHandle_t Task5; 
TaskHandle_t Task6; 
TaskHandle_t Task7; 
//TaskHandle_t Task8; 

void Task1code(void* pvParameters);
void Init_Task1();
void Task2code(void* pvParameters);
void Init_Task2();
void Task3code(void* pvParameters);
void Init_Task3();
void Task4code(void* pvParameters);
void Init_Task4();
void Task5code(void* pvParameters);
void Init_Task5();
void Task6code(void* pvParameters);
void Init_Task6();
//void Task7code(void* pvParameters);
//void Init_Task7();
//void Task8code(void* pvParameters);
//void Init_Task8();


void IRAM_ATTR onTimer();
byte crc8_bytes(byte *buffer, byte size);

#pragma pack(push, 1) // используем принудительное выравнивание
struct Rx_buff{       // Структура приемник от клавиатуры
  int Row;
  int Column;
  int RawBits;
  bool statPress;   
  int enc_step=0;
  int enc_stepH=0;
  int enc_click=0;
  int enc_held=0;
  byte crc;
};
#pragma pack(pop)
Rx_buff RxBuff;

#pragma pack(push, 1) // используем принудительное выравнивание
struct Tx_buff{       // Структура для отправки на клавиатуру
  int x;
  uint8_t y;
  byte crc;
};
#pragma pack(pop)
Tx_buff TxBuff;

QueueHandle_t QueueHandleUartResive; // Определить дескриптор очереди
const int QueueElementSizeUart = 10;
typedef struct{
  int activeRow;     // Номер строки
  int activeColumn;  // Номер столбца
  int statusColumn;  // Байт с битами всего столбца
  bool statPress;    // Статус нажата или отпущена кнопка
  int enc_step=0;
  int enc_stepH=0;
  int enc_click=0;
  int enc_held=0;
} message_uart_resive;


//--------------------------------------------------------------------------------------------

#define DWIDTH   320 // ширина экрана
#define DHEIGHT  240 // высота экрана
#include <TFT_eSPI.h>
TFT_eSPI  tft = TFT_eSPI();  // Экземпляр библиотеки
TFT_eSprite spr = TFT_eSprite(&tft);  
uint16_t* sprPtr;            // Указатели на запуск спрайтов в оперативной памяти (тогда это указатели на "изображение")
bool sync_update = 0;        // Переменная равна 1, когда дисплей отрисован полностью и находится в неаквном состоянии

//--------------------------------------------------------------------------------------------

#include <EEPROM.h>
struct valueEEprom {  // структура с переменными     
  int initKey; 
  char ssid[20];
  char pass[20];
  int device_max_current[PSNUMBER];
  int device_max_voltage[PSNUMBER]; 
  int device_max_power[PSNUMBER];  
  float volt_preset[PSNUMBER][COUNT_PRESET]; 
  float curr_protect[PSNUMBER][COUNT_PRESET];   
}; 
valueEEprom EE_VALUE;

//--------------------------------------------------------------------------------------------

struct ps_struct_t {  // Структура для хранения параметров источников

  String      Text_device_max_current;    // максимальный ток
  String      Text_device_max_voltage;    // максимальное напряжение
  String        Text_device_max_power;    // максимальная мощность 

  int              device_max_current;    // максимальный ток
  int              device_max_voltage;    // максимальное напряжение
  int                device_max_power;    // максимальная мощность     

  float     volt_preset[COUNT_PRESET];    // значение персетов напряжений   
  uint8_t  volt_decimal[COUNT_PRESET];    // количество знаков после запятой для каждого значения пресета 
  uint16_t   volt_color[COUNT_PRESET];    // цвет текста пресета напряжения
  uint16_t volt_colorbg[COUNT_PRESET];    // цвет фона текста пресета напряжения

  int32_t            volt_main_pos_x;     // x координата центра центрального отображения напряжения
  int32_t            volt_main_pos_y;     // y координата центра центрального отображения напряжения
  float                     volt_main;    // значение напряжения
  uint8_t           volt_main_decimal;    // количаство знаков после запятой значения напряжения
  uint32_t            volt_main_color;    // цвет текста напряжения
  uint32_t volt_main_colorbg[COUNT_RECT_MAIN];    // цвет фона текста напряжения
  
  int32_t         curr_protect_pos_x;     // x координата центра защиты по току   
  int32_t         curr_protect_pos_y;     // y координата центра защиты по току 
  float   curr_protect[COUNT_PRESET];     // значение токов пресетов
  bool              lock_curr_protect;    // флаг групповой установки токов на пресеты или отдельно на каждый
  uint8_t        curr_protect_decimal;    // количаство знаков после запятой значения тока
  uint32_t         curr_protect_color;    // цвет текста тока
  uint32_t       curr_protect_colorbg;    // цвет фона текста тока

  uint8_t          number_position_ps;    // номер позиции источника 0,1,2,3... 
  char            text_device_pos[10];    // значение текста  "PS_LOW" "PS_HIGH" и тд 
  int32_t          text_device_pos_x;     // x координата центра текста
  int32_t          text_device_pos_y;     // y координата центра текста
  int32_t        param_ps_volt_pos_x;     // x координата центра отображения максимального напряжения источника
  int32_t        param_ps_volt_pos_y;     // y координата центра отображения максимального напряжения источника
  int32_t        param_ps_curr_pos_x;     // x координата центра отображения максимального тока источника
  int32_t        param_ps_curr_pos_y;     // y координата центра отображения максимального тока источника
  }; 
ps_struct_t power_supply[PSNUMBER]; // Создайте структуру и получите указатель на нее

struct ui_t {
  int32_t preset_val_pos_x[COUNT_PRESET] = {32,96,160,224,288};
  int32_t preset_val_pos_y[COUNT_PRESET] = {39,39,39,39,39};
  uint8_t preset_text_size = 2;  
  int32_t preset_rect_pos_x[COUNT_PRESET] = {3,67,131,195,259};
  int32_t preset_rect_pos_y[COUNT_PRESET] = {10,10,10,10,10};  
  int32_t preset_rect_size_x[COUNT_PRESET] = {58,58,58,58,58};
  int32_t preset_rect_size_y[COUNT_PRESET] = {58,58,58,58,58};
  int32_t preset_rect_radius = 5;

  int32_t preset_rect_main_pos_x[COUNT_RECT_MAIN] = {3,93,93,237};
  int32_t preset_rect_main_pos_y[COUNT_RECT_MAIN] = {90,91,185,90};  
  int32_t preset_rect_main_size_x[COUNT_RECT_MAIN] = {80,134,134,80};
  int32_t preset_rect_main_size_y[COUNT_RECT_MAIN] = {140,85,45,140};
  uint8_t preset_text_main_size[COUNT_RECT_MAIN] = {};
  int32_t preset_rect_main_radius = 5;
  
};
ui_t ui_interface;

int level_ui = 0;                  // уровень меню (подменю)
uint8_t active_preset = 0;         // Активная позиция пресета
uint8_t active_power_supply = 0;   // Активный источник
uint8_t active_work_power = 0;     // Активный источник вкл/выкл

const char* txt_PS_LOW = "PS_LOW";
const char* txt_PS_HIGH = "PS_HIGH";
const char* text_step_10  = "enc. step 10.0 V";
const char* text_step_1   = "enc. step 1.00 V";
const char* text_step_01  = "enc. step 0.10 V";
const char* text_step_001 = "enc. step 0.01 V";         

int select_step_ps = 0;
float val_step_ps[] = {10.0, 1.0, 0.1, 0.01}; // массив значений для изменения напряжений (на шаг)
String str_step_ps[] = {text_step_10, text_step_1,text_step_01,text_step_001};

//--------------------------------------------------------------------------------------------

#define EB_FAST 60     // таймаут быстрого поворота, мс
#define EB_DEB 40      // дебаунс кнопки, мс
#define EB_CLICK 200   // таймаут накликивания, мс
#include <EncButton2.h>
EncButton2<EB_ENCBTN> enc(INPUT, ENCODER_A, ENCODER_B, BTN_HALL);  // энкодер с кнопкой
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
int Enc_step  = 0;
int Enc_stepH = 0;
int Enc_click = 0;
int Enc_held  = 0;

//--------------------------------------------------------------------------------------------

void IRAM_ATTR onTimer(){ // Опрос энкодера
  portENTER_CRITICAL_ISR(&timerMux);
  enc.tick();    // опрос энкодера  
  portEXIT_CRITICAL_ISR(&timerMux);
}

void Task1code(void* pvParameters) {  // Обработка принятых данных от клавиатуры
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task1code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif   
  message_uart_resive message;   
  digitalWrite(OUT_ON, LOW);
  digitalWrite(OUT_DU, LOW);

  for (;;) { 
    if(QueueHandleUartResive != NULL){ // Проверка работоспособности просто для того, чтобы убедиться, что очередь действительно существует
      int ret = xQueueReceive(QueueHandleUartResive, &message, portMAX_DELAY);
      if(ret == pdPASS){ 
               
        Enc_step  = message.enc_step;
        Enc_stepH = message.enc_stepH;
        Enc_click = message.enc_click;
        Enc_held  = message.enc_held;         
        
        if(message.activeRow == -1){digitalWrite(OUT_DU, LOW);}
        if(message.activeRow != -1){ // обработка нажатия кнопок
          //for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;} // очистить цвет всех пресетов

          if (message.activeRow == 3 && message.activeColumn == 3){for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;}power_supply[active_power_supply].volt_colorbg[0] = TFT_DARKGREEN; active_preset=0;} // переходное минимальное
          if (message.activeRow == 4 && message.activeColumn == 3){for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;}power_supply[active_power_supply].volt_colorbg[1] = TFT_DARKGREEN; active_preset=1;} // минимальное
          if (message.activeRow == 4 && message.activeColumn == 2){for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;}power_supply[active_power_supply].volt_colorbg[2] = TFT_DARKGREEN; active_preset=2;} // номинальное
          if (message.activeRow == 4 && message.activeColumn == 1){for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;}power_supply[active_power_supply].volt_colorbg[3] = TFT_DARKGREEN; active_preset=3;} // максимальное
          if (message.activeRow == 5 && message.activeColumn == 1){for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;}power_supply[active_power_supply].volt_colorbg[4] = TFT_DARKGREEN; active_preset=4;} // переходное максимальное
          if (message.activeRow == 3 && message.activeColumn == 0){digitalWrite(OUT_DU, HIGH);} // ДУ
          if (message.activeRow == 1 && message.activeColumn == 0){  // Вкл/Выкл
              active_work_power++; 
              power_supply[active_power_supply].volt_colorbg[active_preset]=TFT_DARKGREEN;              
              if(active_work_power==2){active_work_power=0;}               
              digitalWrite(OUT_ON, active_work_power);
            }                     
          }
        }
      }  
   }
}

void Init_Task1() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task1code, /* Функция задачи. */
    "Task1",   /* Ее имя. */
    4096,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task1,    /* Дескриптор задачи для отслеживания */
    1);        /* Указываем пин для данного ядра */
  delay(500);
}

void Task2code(void* pvParameters) {  // Функции энкодера
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task2code running on core ");
  Serial.println(xPortGetCoreID());
  #endif  

  for (;;) {
    
  // =============== ЭНКОДЕР ===============

   if (enc.left()|| Enc_step<0)  {  // поворот налево   
   Enc_step=0; 
    power_supply[active_power_supply].volt_preset[active_preset] = power_supply[active_power_supply].volt_preset[active_preset] - val_step_ps[select_step_ps];
      if(power_supply[active_power_supply].volt_preset[active_preset] < 0){power_supply[active_power_supply].volt_preset[active_preset] = 0;}

      }

   if (enc.right()|| Enc_step>0) {  // поворот направо  
   Enc_step=0;    
    power_supply[active_power_supply].volt_preset[active_preset] = power_supply[active_power_supply].volt_preset[active_preset] + val_step_ps[select_step_ps];     
      if(power_supply[active_power_supply].volt_preset[active_preset] > power_supply[active_power_supply].device_max_voltage){power_supply[active_power_supply].volt_preset[active_preset] = power_supply[active_power_supply].device_max_voltage;}      
      }

   if (enc.leftH()|| Enc_stepH<0) { // поворот налево нажатый
   Enc_stepH=0;
   power_supply[active_power_supply].curr_protect[active_preset] = power_supply[active_power_supply].curr_protect[active_preset] - val_step_ps[select_step_ps];
      if(power_supply[active_power_supply].lock_curr_protect == 0){
        if(power_supply[active_power_supply].curr_protect[active_preset] < 0){power_supply[active_power_supply].curr_protect[active_preset] = 0;}
      }
      if(power_supply[active_power_supply].lock_curr_protect == 1){
        if(power_supply[active_power_supply].curr_protect[active_preset] < 0){power_supply[active_power_supply].curr_protect[active_preset] = 0;}
        float curr_protect = power_supply[active_power_supply].curr_protect[active_preset];
        for (int i=0; i<COUNT_PRESET; i++){
        power_supply[active_power_supply].curr_protect[i] = curr_protect;
        }
      }
   }

   if (enc.rightH()|| Enc_stepH>0){ // поворот направо нажатый
   Enc_stepH=0;
   power_supply[active_power_supply].curr_protect[active_preset] = power_supply[active_power_supply].curr_protect[active_preset] + val_step_ps[select_step_ps]; 
      if(power_supply[active_power_supply].lock_curr_protect == 0){
        if(power_supply[active_power_supply].curr_protect[active_preset] > power_supply[active_power_supply].device_max_current){power_supply[active_power_supply].curr_protect[active_preset] = power_supply[active_power_supply].device_max_current;}
      }
      if(power_supply[active_power_supply].lock_curr_protect == 1){
        if(power_supply[active_power_supply].curr_protect[active_preset] > power_supply[active_power_supply].device_max_current){power_supply[active_power_supply].curr_protect[active_preset] = power_supply[active_power_supply].device_max_current;}
        float curr_protect = power_supply[active_power_supply].curr_protect[active_preset];
        for (int i=0; i<COUNT_PRESET; i++){
          power_supply[active_power_supply].curr_protect[i] = curr_protect;
        }
      }  
   }  

   if (enc.hasClicks(2)) {
      Serial.println("action 2 clicks");  
      active_work_power=0;    // отключить источник           
      active_power_supply++;
      if(active_power_supply == PSNUMBER){active_power_supply = 0;}            

      if(active_power_supply == 1){
        power_supply[active_power_supply].volt_main_colorbg[0] = TFT_SILVER;
        power_supply[active_power_supply].volt_main_colorbg[3] = TFT_DARKGREEN;
        for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;} // очистить цвет всех пресетов
        power_supply[active_power_supply].volt_colorbg[active_preset]=TFT_DARKGREEN;
      }
      if(active_power_supply == 0){
        power_supply[active_power_supply].volt_main_colorbg[0] = TFT_DARKGREEN;
        power_supply[active_power_supply].volt_main_colorbg[3] = TFT_SILVER;
        for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;} // очистить цвет всех пресетов
        power_supply[active_power_supply].volt_colorbg[active_preset]=TFT_DARKGREEN;
      }
   }

   if (enc.click()|| Enc_click==1){ 
    Enc_click=0;   
      select_step_ps++;
      if(select_step_ps == 4){select_step_ps = 0;}
    }  

   if (enc.held() || Enc_held==1){   
    Enc_held=0; 
         power_supply[active_power_supply].lock_curr_protect = power_supply[active_power_supply].lock_curr_protect ^ 1; 
         //Serial.print("lock_curr_protect ");
         //Serial.println(power_supply[active_power_supply].lock_curr_protect);                                                                                                                                              
    } 
    
          
  #if (ENABLE_DEBUG_ENC == 1)  
  if (enc.left()) Serial.println("left");     // поворот налево
  if (enc.right()) Serial.println("right");   // поворот направо
  if (enc.leftH()) Serial.println("leftH");   // нажатый поворот налево
  if (enc.rightH()) Serial.println("rightH"); // нажатый поворот направо
  #endif

  // =============== КНОПКА ===============
  
  #if (ENABLE_DEBUG_ENC == 1)
  if (enc.press()) Serial.println("press");
  if (enc.click()) Serial.println("click");
  if (enc.release()) Serial.println("release"); 
  if (enc.held()) Serial.println("held");      // однократно вернёт true при удержании
  #endif
   
   enc.resetState();     
   vTaskDelay(30);    
  }
}

void Init_Task2() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task2code, /* Функция задачи. */
    "Task2",   /* Ее имя. */
    4096,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task2,    /* Дескриптор задачи для отслеживания */
    0);        /* Указываем пин для данного ядра */
  delay(500);
}

void Task3code(void* pvParameters) {  // Работа LCD
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task3code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif 
  int sprite_select = 0;
  int sprite_offset = 0;
  int sprite_draw = 0;
  unsigned long tms;
  unsigned long tmst;
  
  for (;;) {    
  
  sync_update = 0;
  
  switch (sprite_select){  
    case 0: sprite_offset =  0;   sprite_draw =  0;  break;  
    case 1: sprite_offset = -60;  sprite_draw = 60;  break;
    case 2: sprite_offset = -120; sprite_draw = 120; break;
    case 3: sprite_offset = -180; sprite_draw = 180; break;
    case 4: sprite_offset = 0; sprite_draw = 0; sprite_select = 0; break;
  }  
  
  sprPtr = (uint16_t*)spr.createSprite(DWIDTH, 60);
  if(sprite_select != 0){spr.setViewport(0, sprite_offset, DWIDTH, DHEIGHT);}
  spr.setTextDatum(MC_DATUM);  
  
  /*------------Сетка,плитки------------------------------------*/
  
  //spr.fillSprite(TFT_BLACK); // Очистка экрана
  spr.fillRoundRect(ui_interface.preset_rect_pos_x[0], ui_interface.preset_rect_pos_y[0], ui_interface.preset_rect_size_x[0], ui_interface.preset_rect_size_y[0], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_colorbg[0]);
  spr.fillRoundRect(ui_interface.preset_rect_pos_x[1], ui_interface.preset_rect_pos_y[1], ui_interface.preset_rect_size_x[1], ui_interface.preset_rect_size_y[1], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_colorbg[1]);
  spr.fillRoundRect(ui_interface.preset_rect_pos_x[2], ui_interface.preset_rect_pos_y[2], ui_interface.preset_rect_size_x[2], ui_interface.preset_rect_size_y[2], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_colorbg[2]);
  spr.fillRoundRect(ui_interface.preset_rect_pos_x[3], ui_interface.preset_rect_pos_y[3], ui_interface.preset_rect_size_x[3], ui_interface.preset_rect_size_y[3], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_colorbg[3]);  
  spr.fillRoundRect(ui_interface.preset_rect_pos_x[4], ui_interface.preset_rect_pos_y[4], ui_interface.preset_rect_size_x[4], ui_interface.preset_rect_size_y[4], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_colorbg[4]);   
  spr.drawFastHLine(0, 80, DWIDTH, TFT_SILVER);  //отрисовка горизонтальной линии
 
  spr.fillRoundRect(ui_interface.preset_rect_main_pos_x[0], ui_interface.preset_rect_main_pos_y[0], ui_interface.preset_rect_main_size_x[0], ui_interface.preset_rect_main_size_y[0], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_main_colorbg[0]);
  spr.fillRoundRect(ui_interface.preset_rect_main_pos_x[1], ui_interface.preset_rect_main_pos_y[1], ui_interface.preset_rect_main_size_x[1], ui_interface.preset_rect_main_size_y[1], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_main_colorbg[1]);
  spr.fillRoundRect(ui_interface.preset_rect_main_pos_x[2], ui_interface.preset_rect_main_pos_y[2], ui_interface.preset_rect_main_size_x[2], ui_interface.preset_rect_main_size_y[2], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_main_colorbg[2]);
  spr.fillRoundRect(ui_interface.preset_rect_main_pos_x[3], ui_interface.preset_rect_main_pos_y[3], ui_interface.preset_rect_main_size_x[3], ui_interface.preset_rect_main_size_y[3], ui_interface.preset_rect_radius, power_supply[active_power_supply].volt_main_colorbg[3]); 

/*--------------Отображение значений пресетов--------------------------*/

  if(power_supply[active_power_supply].volt_preset[active_preset]>=100){power_supply[active_power_supply].volt_decimal[active_preset]=1;} 
  if(power_supply[active_power_supply].volt_preset[active_preset]<100){power_supply[active_power_supply].volt_decimal[active_preset]=2;} 
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_colorbg[0]);
  spr.drawFloat(power_supply[active_power_supply].volt_preset[0], power_supply[active_power_supply].volt_decimal[0], ui_interface.preset_val_pos_x[0], ui_interface.preset_val_pos_y[0], ui_interface.preset_text_size);  
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_colorbg[1]);
  spr.drawFloat(power_supply[active_power_supply].volt_preset[1], power_supply[active_power_supply].volt_decimal[1], ui_interface.preset_val_pos_x[1], ui_interface.preset_val_pos_y[1], ui_interface.preset_text_size);  
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_colorbg[2]);
  spr.drawFloat(power_supply[active_power_supply].volt_preset[2], power_supply[active_power_supply].volt_decimal[2], ui_interface.preset_val_pos_x[2], ui_interface.preset_val_pos_y[2], ui_interface.preset_text_size);
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_colorbg[3]);
  spr.drawFloat(power_supply[active_power_supply].volt_preset[3], power_supply[active_power_supply].volt_decimal[3], ui_interface.preset_val_pos_x[3], ui_interface.preset_val_pos_y[3], ui_interface.preset_text_size);
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_colorbg[4]);
  spr.drawFloat(power_supply[active_power_supply].volt_preset[4], power_supply[active_power_supply].volt_decimal[4], ui_interface.preset_val_pos_x[4], ui_interface.preset_val_pos_y[4], ui_interface.preset_text_size);

/*---------------------------------------------------------------------*/  

  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_main_colorbg[0]);   
  spr.drawString(txt_PS_LOW, 43, 140, 2);
  spr.drawString(power_supply[active_power_supply].Text_device_max_voltage, 43, 160, 2); 
  spr.drawString(power_supply[active_power_supply].Text_device_max_current, 43, 180, 2); 

  spr.setTextColor(TFT_BLACK, TFT_SILVER);   
  spr.drawFloat(power_supply[active_power_supply].volt_preset[active_preset], power_supply[active_power_supply].volt_decimal[active_preset], 160, 134, 6);
  spr.drawString(str_step_ps[select_step_ps], 160, 165, 2); 

  spr.drawFloat(power_supply[active_power_supply].curr_protect[active_preset], 2, 160, 210, 4);
  if(power_supply[active_power_supply].lock_curr_protect == 0){spr.drawString("*", 110, 215, 4);}
  if(power_supply[active_power_supply].lock_curr_protect == 0){spr.drawString("*", 210, 215, 4);} 
  
  spr.setTextColor(TFT_BLACK, power_supply[active_power_supply].volt_main_colorbg[3]);     
  spr.drawString(txt_PS_HIGH, 277, 140, 2);
  spr.drawString(power_supply[active_power_supply].Text_device_max_voltage, 277, 160, 2);
  spr.drawString(power_supply[active_power_supply].Text_device_max_current, 277, 180, 2); 


  tft.pushImageDMA(0, sprite_draw, 320, 60, sprPtr);
  //vTaskDelay(1);  
  spr.deleteSprite();  // Delete the sprite to free up the RAM
  sprite_select ++;
  if(sprite_select==4){sync_update=1;}

  vTaskDelay(15);
  }
}

void Init_Task3() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task3code, /* Функция задачи. */
    "Task3",   /* Ее имя. */
    4096,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task3,    /* Дескриптор задачи для отслеживания */
    1);        /* Указываем пин для данного ядра */
  delay(500);
}

void Task4code(void* pvParameters) {  // Выключение Отложенная задача 
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task4code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif

  for (;;) {
   
   vTaskDelay(100);   
  }
}

void Init_Task4() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task4code, /* Функция задачи. */
    "Task4",   /* Ее имя. */
    1024,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task4,    /* Дескриптор задачи для отслеживания */
    0);        /* Указываем пин для данного ядра */
  //delay(500);
}

void Task5code(void* pvParameters) {  // Тест интерфейса LCD  
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task5code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif

  for (;;) {
    active_power_supply = 0;
    active_work_power = 1;
    select_step_ps = 0;
    power_supply[active_power_supply].volt_main_colorbg[0] = TFT_DARKGREEN;
    power_supply[active_power_supply].volt_main_colorbg[3] = TFT_SILVER;
    //power_supply[1].volt_main_colorbg[0] = TFT_DARKGREEN;
    //power_supply[1].volt_main_colorbg[3] = TFT_SILVER;
    
    
    for(int i=0; i<COUNT_PRESET; i++){power_supply[0].volt_colorbg[i] = TFT_SILVER;}
    active_preset = 0;
    power_supply[0].volt_preset[0] = power_supply[0].volt_preset[0] + 1.12;
    power_supply[0].volt_colorbg[0] = TFT_DARKGREEN;
    power_supply[0].curr_protect[0] = 25.89;
    vTaskDelay(5000); 
   
   for(int i=0; i<COUNT_PRESET; i++){power_supply[0].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 1;
   power_supply[0].volt_preset[1] = power_supply[0].volt_preset[1] + 2.21;
   power_supply[0].volt_colorbg[1] = TFT_DARKGREEN;
   power_supply[0].curr_protect[1] = 15.12;
   select_step_ps = 1;
   vTaskDelay(5000); 

   for(int i=0; i<COUNT_PRESET; i++){power_supply[0].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 2;
   power_supply[0].volt_preset[2] = power_supply[0].volt_preset[2] + 3.53;
   power_supply[0].volt_colorbg[2] = TFT_DARKGREEN;
   power_supply[0].curr_protect[2] = 36.46;
   vTaskDelay(5000);

   for(int i=0; i<COUNT_PRESET; i++){power_supply[0].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 3;
   power_supply[0].volt_preset[3] = power_supply[0].volt_preset[3] + 4.81;
   power_supply[0].volt_colorbg[3] = TFT_DARKGREEN;
   power_supply[0].curr_protect[3] = 9.28;
   vTaskDelay(5000);

   for(int i=0; i<COUNT_PRESET; i++){power_supply[0].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 4;
   power_supply[0].volt_preset[4] = power_supply[0].volt_preset[4] + 5.11;
   power_supply[0].volt_colorbg[4] = TFT_DARKGREEN;
   power_supply[0].curr_protect[4] = 100.31;
   vTaskDelay(5000);

//---------------------------------------------------------------------------------------
   active_power_supply = 1;
   active_work_power = 0;
   power_supply[active_power_supply].volt_main_colorbg[0] = TFT_SILVER;
   power_supply[active_power_supply].volt_main_colorbg[3] = TFT_DARKGREEN;
   //power_supply[0].volt_main_colorbg[0] = TFT_SILVER;
   //power_supply[0].volt_main_colorbg[3] = TFT_DARKGREEN;
   select_step_ps = 2;
   
   for(int i=0; i<COUNT_PRESET; i++){power_supply[1].volt_colorbg[i] = TFT_SILVER;}
    active_preset = 0;
    power_supply[1].volt_preset[0] = power_supply[1].volt_preset[0] + 1.12;
    power_supply[1].volt_colorbg[0] = TFT_DARKGREEN;
    power_supply[1].curr_protect[0] = 25.89;
    vTaskDelay(3000); 
   
   for(int i=0; i<COUNT_PRESET; i++){power_supply[1].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 1;
   power_supply[1].volt_preset[1] = power_supply[1].volt_preset[1] + 2.21;
   power_supply[1].volt_colorbg[1] = TFT_DARKGREEN;
   power_supply[1].curr_protect[1] = 15.12;
   select_step_ps = 3;
   vTaskDelay(3000); 

   for(int i=0; i<COUNT_PRESET; i++){power_supply[1].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 2;
   power_supply[1].volt_preset[2] = power_supply[1].volt_preset[2] + 3.53;
   power_supply[1].volt_colorbg[2] = TFT_DARKGREEN;
   power_supply[1].curr_protect[2] = 36.46;
   vTaskDelay(3000);

   for(int i=0; i<COUNT_PRESET; i++){power_supply[1].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 3;
   power_supply[1].volt_preset[3] = power_supply[1].volt_preset[3] + 4.81;
   power_supply[1].volt_colorbg[3] = TFT_DARKGREEN;
   power_supply[1].curr_protect[3] = 9.28;
   vTaskDelay(3000);

   for(int i=0; i<COUNT_PRESET; i++){power_supply[1].volt_colorbg[i] = TFT_SILVER;} 
   active_preset = 4;
   power_supply[1].volt_preset[4] = power_supply[1].volt_preset[4] + 5.11;
   power_supply[1].volt_colorbg[4] = TFT_DARKGREEN;
   power_supply[1].curr_protect[4] = 100.31;
   vTaskDelay(3000);
  }
}

void Init_Task5() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task5code, /* Функция задачи. */
    "Task5",   /* Ее имя. */
    1024,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task5,    /* Дескриптор задачи для отслеживания */
    0);        /* Указываем пин для данного ядра */
  delay(50);
}

void Task6code(void* pvParameters) {  // Моргание активным пресетом 
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task6code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif  

  for (;;) { 

    if(active_work_power==1 && sync_update==1 && power_supply[active_power_supply].volt_colorbg[active_preset]==TFT_DARKGREEN){      
      power_supply[active_power_supply].volt_colorbg[active_preset] = TFT_OLIVE;
      //sync_update=0;
      vTaskDelay(350);}
      
    if(active_work_power==1 && sync_update==1 && power_supply[active_power_supply].volt_colorbg[active_preset]==TFT_OLIVE){  
      power_supply[active_power_supply].volt_colorbg[active_preset] = TFT_DARKGREEN;  
      //sync_update=0;    
      vTaskDelay(350);}
   vTaskDelay(1);
  }
}

void Init_Task6() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task6code, /* Функция задачи. */
    "Task6",   /* Ее имя. */
    1024,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task6,    /* Дескриптор задачи для отслеживания */
    0);        /* Указываем пин для данного ядра */
  delay(50);
}

void Task7code(void* pvParameters) {  // Опрос входов 
  #if (ENABLE_DEBUG_TASK == 1)
  Serial.print("Task7code running on core ");
  Serial.println(xPortGetCoreID()); 
  #endif  

  int OldStatPowerSelect = 0;

  for (;;) { 
    int StatPowerSelect = digitalRead(POWER_SELECT);    

    if (OldStatPowerSelect != StatPowerSelect){
      if (StatPowerSelect == 1){
        active_work_power=0;    // отключить источник 
        digitalWrite(OUT_ON, active_work_power); 
        active_power_supply = 1;
        power_supply[active_power_supply].volt_main_colorbg[0] = TFT_SILVER;
        power_supply[active_power_supply].volt_main_colorbg[3] = TFT_DARKGREEN;
        for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;} // очистить цвет всех пресетов
        power_supply[active_power_supply].volt_colorbg[active_preset]=TFT_DARKGREEN;
        OldStatPowerSelect = StatPowerSelect;
        }
      if (StatPowerSelect == 0){
        active_work_power=0;    // отключить источник 
        digitalWrite(OUT_ON, active_work_power);  
        active_power_supply = 0;
        power_supply[active_power_supply].volt_main_colorbg[0] = TFT_DARKGREEN;
        power_supply[active_power_supply].volt_main_colorbg[3] = TFT_SILVER;
        for(int i=0; i<COUNT_PRESET; i++){power_supply[active_power_supply].volt_colorbg[i] = TFT_SILVER;} // очистить цвет всех пресетов
        power_supply[active_power_supply].volt_colorbg[active_preset]=TFT_DARKGREEN;
        OldStatPowerSelect = StatPowerSelect;
        }
    }
   
   vTaskDelay(100);
  }
}

void Init_Task7() {  //создаем задачу
  xTaskCreatePinnedToCore(
    Task7code, /* Функция задачи. */
    "Task7",   /* Ее имя. */
    1024,      /* Размер стека функции */
    NULL,      /* Параметры */
    2,         /* Приоритет */
    &Task7,    /* Дескриптор задачи для отслеживания */
    0);        /* Указываем пин для данного ядра */
  delay(50);
}

void IRAM_ATTR serialEvent(){   
  #if (ENABLE_DEBUG_UART == 1)  
  SerialBT.println("Есть данные в прерывании Serial");  
  #endif
  if (Serial.readBytes((byte*)&RxBuff, sizeof(RxBuff))) {
  byte crc = crc8_bytes((byte*)&RxBuff, sizeof(RxBuff));
  if (crc == 0) {
    #if (ENABLE_DEBUG_UART == 1)
    SerialBT.println("CRC PASSED");
    #endif       
      message_uart_resive message; 
      message.activeRow = RxBuff.Row;        // Номер строки
      message.activeColumn = RxBuff.Column;  // Номер столбца
      message.statusColumn = RxBuff.RawBits; // Байт с битами всего столбца
      message.statPress = RxBuff.statPress;  // Статус нажата или отпущена кнопка
      message.enc_step = RxBuff.enc_step; 
      message.enc_stepH = RxBuff.enc_stepH;  
      message.enc_click = RxBuff.enc_click; 
      message.enc_held = RxBuff.enc_held;     
    
      if(QueueHandleUartResive != NULL && uxQueueSpacesAvailable(QueueHandleUartResive) > 0){ // проверьте, существует ли очередь И есть ли в ней свободное место
        int ret = xQueueSend(QueueHandleUartResive, (void*) &message, 0);
        if(ret == pdTRUE){
          #if (ENABLE_DEBUG_UART == 1)
          SerialBT.println("serialEvent Отправлены данные в очередь "); 
          SerialBT.println(message.activeRow); 
          SerialBT.println(message.activeColumn); 
          SerialBT.println(message.statusColumn); 
          SerialBT.println(message.statPress); 
          SerialBT.println(message.enc_step);
          SerialBT.println(message.enc_click);
          SerialBT.println(message.enc_held);   
          #endif    
          }
        else if(ret == errQUEUE_FULL){
          #if (ENABLE_DEBUG_UART == 1)
          SerialBT.println("Не удалось отправить данные в очередь из serialEvent()");
          #endif
        } 
      }
      else if (QueueHandleUartResive != NULL && uxQueueSpacesAvailable(QueueHandleUartResive) == 0){
        #if (ENABLE_DEBUG_UART == 1)
        SerialBT.println("Очередь отсутствует или нет свободного места");
        #endif
        }
      } 
   else {
      #if (ENABLE_DEBUG_UART == 1)
      SerialBT.println("CRC ERROR");
      #endif
     }
   }  
}

byte crc8_bytes(byte *buffer, byte size) {
  byte crc = 0;
  for (byte i = 0; i < size; i++) {
    byte data = buffer[i];
    for (int j = 8; j > 0; j--) {
      crc = ((crc ^ data) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
      data >>= 1;
    }
  }
  return crc;
}

void INIT_PWM_IO(){  
  pinMode(BTN_HALL, INPUT);
  pinMode(ENCODER_A, INPUT);
  pinMode(ENCODER_B, INPUT);
  pinMode(POWER_SELECT, INPUT_PULLUP);

  pinMode(OUT_ON, OUTPUT);
  //digitalWrite(OUT_ON, LOW);

  pinMode(OUT_DU, OUTPUT);
  //digitalWrite(OUT_DU, LOW);
}

void INIT_LCD(){
  tft.init();  
  tft.setRotation(1);
  tft.initDMA();
  tft.fillScreen(TFT_BLACK);  
  tft.startWrite(); // TFT chip select held low permanently
}

void INIT_PS(){
  
  for (int i = 0; i < PSNUMBER; i++) {  // Инициализация структур общих параметров источников 
    
    power_supply[i].Text_device_max_voltage = String(EE_VALUE.device_max_voltage[i])+" V";
    power_supply[i].Text_device_max_current = String(EE_VALUE.device_max_current[i])+" A";
    power_supply[i].Text_device_max_power   = String(EE_VALUE.device_max_power[i])+" W";

    power_supply[i].device_max_voltage = EE_VALUE.device_max_voltage[i];
    power_supply[i].device_max_current = EE_VALUE.device_max_current[i];
    power_supply[i].device_max_power   = EE_VALUE.device_max_power[i];

    for(int j=0; j<COUNT_PRESET; j++){     
      power_supply[i].volt_preset[j] = EE_VALUE.volt_preset[i][j];
      power_supply[i].volt_decimal[j] = 2;
      power_supply[i].volt_colorbg[j] = TFT_SILVER;
      power_supply[i].curr_protect[j] = EE_VALUE.curr_protect[i][j];
    }
    for(int j=0; j<COUNT_RECT_MAIN; j++){
      power_supply[i].volt_main_colorbg[j] = TFT_SILVER;
      power_supply[i].volt_colorbg[0] = TFT_DARKGREEN;    
      power_supply[i].volt_main_colorbg[0] = TFT_DARKGREEN;
      
    }
    power_supply[i].lock_curr_protect = 1;
  }  
}

void INIT_TIM_ENC(){
  timer = timerBegin(0, 240, true);
  timerAttachInterrupt(timer, &onTimer, false);
  timerAlarmWrite(timer, 500, true);
  timerAlarmEnable(timer); //Just Enable
}

void INIT_DEFAULT_VALUE(){ // Заполняем переменные в EEPROM базовыми значениями 
    
    EE_VALUE.initKey = INIT_KEY;
    String ssid_default = "AEDON";
    String pass_default = ""; 
    ssid_default.toCharArray(EE_VALUE.ssid, 20);    
    pass_default.toCharArray(EE_VALUE.pass, 20);      

    for(int i=0; i<PSNUMBER; i++){
      EE_VALUE.device_max_current[i]=30;
      EE_VALUE.device_max_voltage[i]=100; 
      EE_VALUE.device_max_power[i]=500;       
      
      for(int j=0; j<COUNT_PRESET; j++){
        EE_VALUE.volt_preset[i][j] = 10; 
        EE_VALUE.curr_protect[i][j] = 30;       
        }
      } 
    //EEPROM.put(0, EE_VALUE);      // сохраняем
    //EEPROM.commit();              // записываем
}

void setup() {
  INIT_PWM_IO();  
  Serial.setTimeout(5);
  Serial.begin(115200, SERIAL_8N1, 3, 1);
  EEPROM.begin(2048);
  EEPROM.get(0, EE_VALUE); //читаем всё из памяти  
  
  #if (ENABLE_DEBUG_BLUETOOTH == 1)
  SerialBT.begin(device_name); //Bluetooth device name
  SerialBT.setPin(pin);
  #endif   

  if (EE_VALUE.initKey != INIT_KEY){INIT_DEFAULT_VALUE();} // первый запуск устройства
  
  QueueHandleUartResive = xQueueCreate(QueueElementSizeUart, sizeof(message_uart_resive)); // Создайте очередь, которая будет содержать <Размер элемента очереди> количество элементов, каждый из которых имеет размер `message_t`, и передайте адрес в <QueueHandleKeyboard>.
  if(QueueHandleUartResive == NULL){  // Проверьте, была ли успешно создана очередь
    Serial.println("QueueHandleUartResive could not be created. Halt.");
    while(1) delay(1000);   // Halt at this point as is not possible to continue
  }
  
  INIT_DEFAULT_VALUE;
  INIT_PS();
  INIT_LCD();
  INIT_TIM_ENC();
  Init_Task3();   // Работа LCD
  Init_Task2();   // Обработка энкодера
  Init_Task1();   // Обработка принятых данных от клавиатуры
  //Init_Task4(); // Выключение Отложенная задача
  //Init_Task5(); // тест интерфейса
  Init_Task6();   // Моргание активным пресетом 
  Init_Task7();   // Опрос входов 

}

void loop() {

}