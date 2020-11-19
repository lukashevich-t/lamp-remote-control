#include <IRremote.h>
#include <ir_Lego_PF_BitStreamEncoder.h>


/*
  Fade

  This example shows how to fade an LED on pin 9 using the analogWrite()
  function.

  The analogWrite() function uses PWM, so if you want to change the pin you're
  using, be sure to use another PWM capable pin. On most Arduino, the PWM pins
  are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11.

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/Fade
*/

byte LED_PIN = 9;           // the PWM pin the LED is attached to
byte IR_PIN = 2;
char level = 0;    // how bright the LED is
uint8_t levels[] = {0, 30, 60, 110, 170, 255};

IRrecv irrecv(IR_PIN); // указываем вывод, к которому подключен приемник
decode_results results;

void setup() {
  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием
  pinMode(LED_PIN, OUTPUT);
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
          level = 0;
          analogWrite(LED_PIN, 0);
        }
        else { 
          level = sizeof(levels) - 1;
          analogWrite(LED_PIN, levels[level]);
        }
        break;
    }
    irrecv.resume(); // принимаем следующую команду
  }
}

//void setBrightness(char value) {
//  level = value;
//  Serial.print("new level is ");
//  Serial.println(level, DEC);
//  if (level >= sizeof(levels)) { 
//    
//    level = sizeof(levels) - 1;
//  }
//  if (level < 0) level = 0;
//  analogWrite(LED_PIN, levels[level]);
//}

void increaseBrightness() {
  if(level >= sizeof(levels) - 1) level = sizeof(levels) - 1;
  else ++level;
  Serial.print("new level is ");
  Serial.println(level, DEC);
  analogWrite(LED_PIN, levels[level]);
}

void decreaseBrightness() {
  if(level <= 0) level = 0;
  else --level;
  Serial.print("new level is ");
  Serial.println(level, DEC);
  analogWrite(LED_PIN, levels[level]);
}
