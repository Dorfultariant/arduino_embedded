#define F_CPU 16000000UL
#define SLAVE_ADDRESS 85
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1
#define DATA_SIZE 16

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "uart.h"

const int BUILTIN_LED = PB5;

int main(void) {

  // DDRC &= ~(1 << SDA) & ~(1 << SDL);

  // Turn off led
  DDRB &= ~(1 << BUILTIN_LED);

  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;
  printf("Hello There!\n");

  while (1) {

    printf("Ah, General Kenobi!\n");
    PORTB ^= (1 << BUILTIN_LED);
    _delay_ms(500);
  }
  return 0;
}
