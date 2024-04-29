#define F_CPU 16000000UL
#define DATA_SIZE 16
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1

/*
 * NOTEs are calculated with following formula:
 * TOP_value = F_CPU / (2 * N_prescaler * note_frequency)
 *
 * So, for example C5 note (523Hz)
 * TOP = F_CPU / (2 * 1 * 523) = 15296,36... ≃ 15296
 */

// Prescaler 1 notes
#define NOTE_1_C5 15296  // 523Hz
#define NOTE_1_Cs3 57971 // 138Hz
#define NOTE_1_Ds4 25723 // 311Hz
#define NOTE_1_D3 54794  // 146Hz

// Prescaler 64 notes
#define NOTE_64_Cs3 906 // 138Hz
#define NOTE_64_D3 856  // 146Hz

#include <avr/interrupt.h>
// #include <avr/io.h>
// #include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
// #include <util/delay.h>

#include "timer3.h"
#include "uart.h"

const int LED_BUILTIN = PB7;

// Represents PIR sensor
const int START_TIMER = PE5;

// Represents Code verification from keypad
const int STOP_TIMER = PE4;

volatile uint16_t counter = 0;

ISR(TIMER3_COMPA_vect) {
  // Clear timer counter
  TCNT3 = 0;
  counter++;
}

int main(void) {

  // Timer starter and stopper pin (PIR would be start and STOP would be code
  // verification)
  DDRE &= ~(1 << START_TIMER) | (1 << STOP_TIMER);

  // Set OC3A as OUTPUT (MEGA AIN[1])
  DDRE |= (1 << PE3);

  USART_Init(MYUBRR);

  stdin = &mystdin;
  stdout = &mystdout;

  printf("Hello There!\n");

  while (1) {
    printf("%d \n", counter);
    //_delay_ms(200);
    if (counter == 10) {
      TIMER3_Clear();
      printf("Sound the Alarm!\n");
    }
    if (PINE & (1 << START_TIMER)) {
      counter = 0;
      TIMER3_Init_CTC();
      TIMER3_SetIntervalSecond();
    } else if (PINE & (1 << STOP_TIMER)) {
      counter = 0;
      TIMER3_Clear();
    }
  }

  return 0;
}
