#include <cstdint>
#include <math.h>
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

void read_keypad_code(char *dest, uint8_t code_len);
int verify_code(char *to_be_checked, char *correct);

int main(void) {
  char correctCode[] = "0423";
  DDRB &= ~(1 << LED_BUILTIN);

  USART_Init(MYUBRR);
  KEYPAD_Init();

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");
  char c[5]; // [0, 1, 2, 3, 4]

  while (1) {
    read_keypad_code(c, 4);

    printf("\nYou inserted: %s\n", c);
    c[4] = '\0';
    uint8_t isValid = verify_code(c, correctCode);

    printf("Is code valid: %d\n", isValid);
    // Turn on the led
    PORTB |= (1 << LED_BUILTIN);

    _delay_ms(500);
  }

  return 0;
}

void read_keypad_code(char *dest, uint8_t code_len) {
  // DEBUG PRINT TO PUTTY
  printf("Give %d. numbers:\n", code_len);

  for (uint8_t i = 0; i < code_len; i++) {
    dest[i] = KEYPAD_GetKey();
    printf("%c ", dest[i]);
  }
}

int verify_code(char *to_be_checked, char *correct) {
  uint8_t i = 0;
  while ((to_be_checked[i] != '\0') && (correct[i] != '\0')) {
    if (to_be_checked[i] != correct[i]) {
      return 0;
    }
    i++;
  }
  return 1;
}
