#include <Arduino.h>
#ifndef LAMP_REMOTE_CONTROL_H
#define LAMP_REMOTE_CONTROL_H

// PIN, к которому подключена нагрузка (светодиоды):
#define LED_PIN 9
// PIN для подключения инфракрасного приёмника:
#define IR_PIN 2                               
unsigned char level = 0;                       // Начальный уровень яркости при подаче питания
uint8_t levels[] = {0, 30, 60, 110, 170, 255}; // Уровни яркости
// pin для подключения кнопки МИНУС (уменьшение яркости/выключение):
#define BTN_MINUS_PIN 7
// pin для подключения кнопки ПЛЮС (увеличение яркости/полная яркость)
#define BTN_PLUS_PIN 8

// #define DEBUG
// Количество команд ПДУ на каждое действие:
#define MAX_COMMANDS 5

#ifdef DEBUG
#define LOG(msg) Serial.println(msg);
#define LOG_HEX(value) Serial.println(value, HEX);
#define LOG_DEC(value) Serial.println(value, DEC);
#else
#define LOG(msg)
#define LOG_HEX(value)
#define LOG_DEC(value)
#endif

void increaseBrightness(void);
void decreaseBrightness(void);
void maxBrightness(void);
void minBrightness(void);
// void checkButton(byte, void (*shortFunc)(), void (*longFunc)());
// void waitRelease(byte);
void workModePoll(void);
void programModePoll(void);
void toggleProgramMode(void);
void writeEeprom(void);
void readEeprom(void);
void logCommands(void);
void addCommand(uint32_t* array, uint32_t value);
void modeChangeFeedback(void);
void commandRememberedFeedback();

enum Modes
{ // Не следует задавать перечислениям значения вручную - поломаются алгоритмы!
  PROGRAM_MINUS, // 0
  PROGRAM_PLUS, // 1
  PROGRAM_ON, // 2
  WORK, // 3 // WORK должен идти последним (для оптимизации)
};

#endif

