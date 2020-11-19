#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>

#define LONG_PRESS_TIME 500

byte LED_PIN = 9;           // the PWM pin the LED is attached to
byte IR_PIN = 2;
char level = 0;    // how bright the LED is
uint8_t levels[] = {0, 30, 60, 110, 170, 255};
byte BTN_MINUS_PIN = 7;
byte BTN_PLUS_PIN = 8;


IRrecv irrecv(IR_PIN); // указываем вывод, к которому подключен приемник
decode_results results;

void setup() {
  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием
  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_MINUS_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
}

void loop() {
  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    switch ( results.value ) {
      case 0xE0E006F9: // Make lighter
      case 0xE0E0E01F:
        increaseBrightness();
        break;
      case 0xE0E08679:  // Make dimmer
      case 0xE0E0D02F:
        decreaseBrightness();
        break;
      case 0xE0E040BF:  // on/off
        if (level > 0) {
          minBrightness();
        }
        else {
          maxBrightness();
        }
        break;
    }
    irrecv.resume(); // принимаем следующую команду
  }

  checkButtons();
}

void increaseBrightness() {
  if (level >= sizeof(levels) - 1) level = sizeof(levels) - 1;
  else ++level;
  Serial.print("new level is ");
  Serial.println(level, DEC);
  analogWrite(LED_PIN, levels[level]);
}

void decreaseBrightness() {
  if (level <= 0) level = 0;
  else --level;
  Serial.print("new level is ");
  Serial.println(level, DEC);
  analogWrite(LED_PIN, levels[level]);
}

void maxBrightness() {
  level = sizeof(levels) - 1;
  analogWrite(LED_PIN, levels[level]);
}

void minBrightness() {
  level = 0;
  analogWrite(LED_PIN, levels[level]);
}

void checkButtons() {
  checkButton(BTN_MINUS_PIN,  decreaseBrightness, minBrightness);
  checkButton(BTN_PLUS_PIN,  increaseBrightness, maxBrightness);
  //  switch (checkButton(BTN_MINUS_PIN, "-")) {
  //    case 1:
  //      decreaseBrightness();
  //      Serial.println("-");
  //      break;
  //    case 2:
  //      analogWrite(LED_PIN, 0);
  //      Serial.println("off");
  //      break;
  //      break;
  //  }
  //  checkButton(BTN_PLUS_PIN, "+");
}

/**
 * Опрашивает кнопку, подключенную к пину, номер которого передан в аргументе.
 * При коротком нажатии вызывает shortFunc.
 * При длинном нажатии вызывает longFunc.
 */
void checkButton(byte button, void (*shortFunc )(), void (*longFunc )()) {
  if (digitalRead(button) == HIGH) return;
  delay(50); // Ждём окончания дребезга
  if (digitalRead(button) == HIGH) return;

  // Дребезг окончился, кнопка по-прежнему нажата:
  shortFunc();
  // Подождать отпускания:
  unsigned long time = millis();
  while (digitalRead(button) == LOW) {
    if (millis() - LONG_PRESS_TIME > time) {
      longFunc();
      break;
    }
  };
  waitRelease(button);
}

void waitRelease(byte button) {
  while (digitalRead(button) == LOW) {} ;
  delay(50);
}
