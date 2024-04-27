#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "keypad.h"
#include "uart.h"

const int LED_BUILTIN = PB7;
/*
void KEYPAD_Init();
void KEYPAD_WaitForKeyRelease();
void KEYPAD_WaitForKeyPress();
uint8_t KEYPAD_GetKey();
*/

int main(void) {

  DDRB &= ~(1 << LED_BUILTIN);

  USART_Init(MYUBRR);
  KEYPAD_Init();

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");
  char c[5]; // [0, 1, 2, 3, 4]

  while (1) {
    printf("Give numbers:\n");

    for (uint8_t i = 0; i <= 4; i++) {
      c[i] = KEYPAD_GetKey();
    }

    printf("\nYou inserted: %a\n", c);

    // Turn on the led
    PORTB ^= (1 << LED_BUILTIN);

    _delay_ms(500);
  }

  return 0;
}
