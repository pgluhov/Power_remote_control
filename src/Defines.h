
#ifndef DEFINES_H
#define DEFINES_H
#include <Arduino.h>

// ========== ДЕФАЙНЫ НАСТРОЕК ==========
String DEVICE_NAME = "Power remote LCD control"; // Имя девайса
String CURRENT_VERSION_SW = "1.03";     // Текущая версиия прошивки 
String VERSION_SW = "Версия ПО 1.03";   // Текст для отображения


#define ENABLE_DEBUG_BLUETOOTH 0  // Если 1 то отладка через bluetooth
#define ENABLE_DEBUG_UART      0  // Если 1 то отладка обмена по uart включена в SerialBT
#define ENABLE_DEBUG           0  // Если 1 то отладка включена в Serial
#define ENABLE_DEBUG_ENC       0  // Если 1 то отладка энкодера в Serial 
#define ENABLE_DEBUG_TASK      1 

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

//--------номера IO-------------------

#endif //DEFINES_H