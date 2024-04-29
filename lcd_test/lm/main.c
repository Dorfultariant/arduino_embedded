#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "uart.h"

const int LED_BUILTIN = PB7;

int main(void) {

  DDRB &= ~(1 << LED_BUILTIN);

  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");

  while (1) {

    printf("Ah, General Kenobi!\n");

    // Turn on the led
    PORTB ^= (1 << LED_BUILTIN);

    _delay_ms(500);
  }

  return 0;
}
