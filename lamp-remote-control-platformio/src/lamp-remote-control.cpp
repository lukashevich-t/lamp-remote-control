#include <Arduino.h>
#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include "GyverButton.h"

#define LONG_PRESS_TIME 500
#define DEBUG

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
void checkButtons(void);
void checkButton(byte, void (*shortFunc)(), void (*longFunc)());
void waitRelease(byte);

byte LED_PIN = 9;                              // PIN, к которому подключена нагрузка (светодиоды)
byte IR_PIN = 2;                               // PIN для подключения инфракрасного приёмника
unsigned char level = 0;                       // Начальный уровень яркости при подаче питания
uint8_t levels[] = {0, 30, 60, 110, 170, 255}; // Уровни яркости
byte BTN_MINUS_PIN = 7;                        // pin для подключения кнопки МИНУС (уменьшение яркости/выключение)
byte BTN_PLUS_PIN = 8;                         // pin для подключения кнопки ПЛЮС (увеличение яркости/полная яркость)

IRrecv irrecv(IR_PIN); // указываем вывод, к которому подключен приемник
decode_results results;
GButton buttPlus(BTN_PLUS_PIN);
GButton buttMinus(BTN_MINUS_PIN);

// команды ПДУ:
const uint32_t MINUS_COMMANDS[5] = {0xE0E08679, 0xE0E0D02F};
const uint32_t PLUS_COMMANDS[5] = {0xE0E006F9, 0xE0E0E01F};
const uint32_t ON_COMMANDS[5] = {0xE0E040BF};

void setup()
{
#ifdef DEBUG
  Serial.begin(9600); // выставляем скорость COM порта
#endif
  irrecv.enableIRIn(); // запускаем прием
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_MINUS_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
}

void loop()
{
  if (irrecv.decode(&results)) // если данные пришли
  {
    uint32_t irCommand = results.value;
    LOG_HEX(results.value); // печатаем данные
    for (byte i = 0; i < sizeof(PLUS_COMMANDS) / sizeof(uint32_t); i++)
    {
      if (irCommand == PLUS_COMMANDS[i])
      {
        increaseBrightness();
        break;
      }
    }

    for (byte i = 0; i < sizeof(MINUS_COMMANDS) / sizeof(uint32_t); i++)
    {
      if (irCommand == MINUS_COMMANDS[i])
      {
        decreaseBrightness();
        break;
      }
    }

    for (byte i = 0; i < sizeof(ON_COMMANDS) / sizeof(uint32_t); i++)
    {
      if (irCommand == ON_COMMANDS[i])
      {
        if (level > 0)
        {
          minBrightness();
        }
        else
        {
          maxBrightness();
        }
        break;
      }
    }
    irrecv.resume(); // принимаем следующую команду
  }

  checkButtons();
}

void increaseBrightness()
{
  if (level >= sizeof(levels) - 1)
    level = sizeof(levels) - 1;
  else
    ++level;
  LOG("new level is ");
  LOG_DEC(level);
  analogWrite(LED_PIN, levels[level]);
}

void decreaseBrightness()
{
  if (level != 0)
    --level;
  LOG("new level is ");
  LOG_DEC(level);
  analogWrite(LED_PIN, levels[level]);
}

void maxBrightness()
{
  level = sizeof(levels) - 1;
  analogWrite(LED_PIN, levels[level]);
}

void minBrightness()
{
  level = 0;
  analogWrite(LED_PIN, levels[level]);
}

void checkButtons()
{
  checkButton(BTN_MINUS_PIN, decreaseBrightness, minBrightness);
  checkButton(BTN_PLUS_PIN, increaseBrightness, maxBrightness);
}

/**
   Опрашивает кнопку, подключенную к пину, номер которого передан в аргументе.
   При коротком нажатии вызывает shortFunc.
   При длинном нажатии вызывает longFunc.
*/
void checkButton(byte button, void (*shortFunc)(), void (*longFunc)())
{
  if (digitalRead(button) == HIGH)
    return;
  delay(50); // Ждём окончания дребезга
  if (digitalRead(button) == HIGH)
    return;

  // Дребезг окончился, кнопка по-прежнему нажата:
  shortFunc();
  // Подождать отпускания:
  unsigned long time = millis();
  while (digitalRead(button) == LOW)
  {
    if (millis() - LONG_PRESS_TIME > time)
    {
      longFunc();
      break;
    }
  };
  waitRelease(button);
}

void waitRelease(byte button)
{
  while (digitalRead(button) == LOW)
    ;
  delay(50);
}
