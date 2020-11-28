#include <Arduino.h>
#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>
#include "GyverButton.h"
#include "lamp-remote-control.h"

IRrecv irrecv(IR_PIN); // указываем вывод, к которому подключен приемник
decode_results results;
GButton buttPlus(BTN_PLUS_PIN);
GButton buttMinus(BTN_MINUS_PIN);

// команды ПДУ:
uint32_t minusCommands[MAX_COMMANDS] = {}; // {0xE0E08679, 0xE0E0D02F};
uint32_t plusCommands[MAX_COMMANDS] = {};  // {0xE0E006F9, 0xE0E0E01F};
uint32_t onCommands[MAX_COMMANDS] = {};    // {0xE0E040BF};

Modes mode = WORK;
// признак того, что в результате программирования изменились команды ПДУ и их по выходу из режима программирования надо записать в EEPROM
boolean commandsChanged;

void setup()
{
  buttPlus.setTimeout(1000);
  buttMinus.setTimeout(1000);
#ifdef DEBUG
  Serial.begin(9600); // выставляем скорость COM порта
  LOG("before read:")
  logCommands();
#endif
  readEeprom();
#ifdef DEBUG
  LOG("after read:")
  logCommands();
#endif

  irrecv.enableIRIn(); // запускаем прием
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_MINUS_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
}

void loop()
{
  if (mode == WORK)
  {
    workModePoll();
  }
  else
  {
    programModePoll();
  }
}

/**
 * Добавить команду ПДУ к списку команд.
 * @param array массив с командами.
 * @param value код новой команды.
 */
void addCommand(uint32_t *array, uint32_t value)
{
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    // в массиве команд уже есть такая - ничего не предпринимаем.
    if (array[i] == value)
      return;
  }
  // Команда в массиве не найдена. Подвинем команды в массиве, чтобы записать в 0-ю позицию новую команду:
  for (byte i = MAX_COMMANDS - 1; i >= 1; i--)
  {
    array[i] = array[i - 1];
  }
  array[0] = value;
  commandsChanged = true;
}

/**
 * Опрос кнопок и пульта в режиме обычной работы (не программирования).
 */
void workModePoll()
{
  if (irrecv.decode(&results)) // если пришли данные от ПДУ
  {
    uint32_t irCommand = results.value;
    LOG_HEX(results.value); // печатаем данные
    for (byte i = 0; i < MAX_COMMANDS; i++)
    {
      if (irCommand == plusCommands[i])
      {
        increaseBrightness();
        break;
      }
      else if (irCommand == minusCommands[i])
      {
        decreaseBrightness();
        break;
      }
      else if (irCommand == onCommands[i])
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

  // опрос кнопок:
  buttPlus.tick();
  buttMinus.tick();
  if (buttPlus.isClick())
  {
    increaseBrightness();
  }
  if (buttMinus.isClick())
  {
    decreaseBrightness();
  }
  if (buttMinus.isHold())
  {
    if (buttMinus.getHoldClicks() >= 3)
    {
      toggleProgramMode();
    }
    else
    {
      minBrightness();
    }
  }
  if (buttPlus.isHold())
  {
    maxBrightness();
  }
}

/**
 * Опрос кнопок и пульта в режиме программирования.
 */
void programModePoll()
{
  if (irrecv.decode(&results)) // если данные пришли
  {
    uint32_t irCommand = results.value;
    LOG_HEX(results.value); // печатаем данные
    switch (mode)
    {
    case PROGRAM_MINUS:
      addCommand(minusCommands, irCommand);
      commandRememberedFeedback();
      break;
    case PROGRAM_PLUS:
      addCommand(plusCommands, irCommand);
      commandRememberedFeedback();
      break;
    case PROGRAM_ON:
      addCommand(onCommands, irCommand);
      commandRememberedFeedback();
      break;
    case WORK:
    default:
      break;
    }
    irrecv.resume(); // принимаем следующую команду
  }

  buttPlus.tick();
  buttMinus.tick();
  if (buttPlus.isClick())
  {
    //переход к программированию следующего действия:
    mode = static_cast<Modes>((static_cast<int>(mode) + 1) % WORK);
    LOG("Next prog mode:");
    LOG_DEC(mode);
    modeChangeFeedback();
  }
  if (buttMinus.isHold() && buttMinus.getHoldClicks() >= 3)
  {
    // Выход из режима программирования:
    toggleProgramMode();
  }
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

void toggleProgramMode()
{
  buttPlus.resetStates();
  buttMinus.resetStates();
  if (mode == WORK)
  {
    LOG("Entering programming mode");
    mode = PROGRAM_MINUS;
  }
  else
  {
    LOG("Leaving programming mode");
    if (commandsChanged)
    {
      LOG("writing new commands to eeprom");
#ifdef DEBUG
      logCommands();
#endif
      writeEeprom();
    }
    mode = WORK;
    level = sizeof(levels) - 1;
  }
  commandsChanged = false;
  modeChangeFeedback();
}

inline void readEeprom()
{
  uint32_t *ptr = 0;
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    minusCommands[i] = eeprom_read_dword(ptr++);
  }
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    plusCommands[i] = eeprom_read_dword(ptr++);
  }
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    onCommands[i] = eeprom_read_dword(ptr++);
  }
}

inline void writeEeprom()
{
  uint32_t *ptr = 0;
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    eeprom_write_dword(ptr++, minusCommands[i]);
  }
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    eeprom_write_dword(ptr++, plusCommands[i]);
  }
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    eeprom_write_dword(ptr++, onCommands[i]);
  }
}

#ifdef DEBUG
void logCommands()
{
  LOG("minus:");
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    LOG_HEX(minusCommands[i]);
  }

  LOG("plus:");
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    LOG_HEX(plusCommands[i]);
  }

  LOG("on:");
  for (byte i = 0; i < MAX_COMMANDS; i++)
  {
    LOG_HEX(onCommands[i]);
  }
}
#endif

void modeChangeFeedback()
{
  switch (mode)
  {
  case PROGRAM_MINUS:
    for (int i = 255; i >= 0; i--)
    {
      analogWrite(LED_PIN, i);
      delay(4);
    }
    break;
  case PROGRAM_PLUS:
    for (int i = 0; i <= 255; i++)
    {
      analogWrite(LED_PIN, i);
      delay(4);
    }
    break;
  case PROGRAM_ON:
    for (int i = 0; i < 3; i++)
    {
      digitalWrite(LED_PIN, LOW);
      delay(500);
      digitalWrite(LED_PIN, HIGH);
      delay(500);
    }
    break;
  case WORK:
    for (int i = 0; i < 5; i++)
    {
      digitalWrite(LED_PIN, LOW);
      delay(100);
      digitalWrite(LED_PIN, HIGH);
      delay(100);
    }
    break;
  default:
    break;
  }
}

/**
 * Даёт отзыв о том, что команда ПДУ запомнена
 * 
 */
void commandRememberedFeedback()
{
  for (byte i = 0; i < 2; i++)
  {
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}