#define F_CPU 16000000UL
#define SLAVE_ADDRESS 85
#define BAUD 9600
#define MYUBRR F_CPU / 16 / BAUD - 1
#define DATA_SIZE 16

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
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#include "timer1.h"
#include "uart.h"

const int BUZZER = PB1;
volatile uint32_t toggle_counter = 0;

void singASong();

ISR(TIMER1_COMPA_vect) {
  TCNT1 = 0;
  if (0 != toggle_counter) {
    PORTB ^= (1 << BUZZER);
    if (0 < toggle_counter) {
      toggle_counter--;
    }
  }
}

int main(void) {

  USART_Init(MYUBRR);
  DDRB |= (1 << BUZZER);

  stdin = &mystdin;
  stdout = &mystdout;
  printf("Hello There!\n");
  TIMER1_Init_Mode_9();
  TIMER1_SetPrescaler(PS_64);

  singASong();

  while (1) {
    printf("La bomba\n");
  }
  return 0;
}

/*
 * A very annoying tone sequence repeated for the time being 10 times...
 *
 */
void singASong() {
  TIMER1_Init_Mode_9();
  TIMER1_SetPrescaler(PS_64);
  for (int i = 0; i < 10; i++) {
    TIMER1_SetTarget(NOTE_64_D3);
    _delay_ms(400);
    TIMER1_SetTarget(NOTE_64_Cs3);
    _delay_ms(400);

    TIMER1_SetTarget(NOTE_64_D3);
    _delay_ms(400);

    TIMER1_SetTarget(NOTE_64_Cs3);
    _delay_ms(400);
    TIMER1_SetTarget(NOTE_64_D3);
    _delay_ms(400);

    TIMER1_SetTarget(NOTE_64_Cs3);
    _delay_ms(400);
  }
  TIMER1_Clear();
}
