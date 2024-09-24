// Программа для PCB [ 3411550 ESP32_LCD_rev_1.10 ]

#ifndef DEFINES_H
#define DEFINES_H
#include <Arduino.h>

// ========== ДЕФАЙНЫ НАСТРОЕК ==========
String DEVICE_NAME = "Power remote LCD control"; // Имя девайса
String CURRENT_VERSION_SW = "1.10";     // Текущая версиия прошивки 
String VERSION_SW = "Версия ПО 1.10";   // Текст для отображения


#define ENABLE_DEBUG_BLUETOOTH 0  // Если 1 то отладка через bluetooth
#define ENABLE_DEBUG_UART      0  // Если 1 то отладка обмена по uart включена в SerialBT
#define ENABLE_DEBUG           0  // Если 1 то отладка включена в Serial
#define ENABLE_DEBUG_ENC       0  // Если 1 то отладка энкодера в Serial 
#define ENABLE_DEBUG_TASK      0
#define DEBUG_RESIVE_UART1     0

#define INIT_KEY      37     // ключ первого запуска. 0-254, на выбор

//-------------объявлять ДО ПОДКЛЮЧЕНИЯ БИБЛИОТЕКИ GyverPortal------------------
#define GP_NO_MDNS          // убрать поддержку mDNS из библиотеки (вход по хосту в браузере)
#define GP_NO_DNS           // убрать поддержку DNS из библиотеки (для режима работы как точка доступа)
//#define GP_NO_OTA         // убрать поддержку OTA обновления прошивки
#define GP_NO_UPLOAD        // убрать поддержку загрузки файлов на сервер
#define GP_NO_DOWNLOAD      // убрать поддержку скачивания файлов с сервера

//--------номера IO-------------------

#define BTN_HALL     33 
#define ENCODER_A    39 
#define ENCODER_B    36

//#define RX_COM  21  
//#define TX_COM  22

#define OUT_ON 22
#define OUT_DU 21

//Аппартный uart0 
#define RX0PIN  16  
#define TX0PIN  17

//Аппартный uart1 
#define RX1PIN  34  
#define TX1PIN  32

//Аппартный uart2 
#define RX2PIN  35  
#define TX2PIN  25

//--------номера IO-------------------

#endif //DEFINES_H