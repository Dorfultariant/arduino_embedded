#include "timer1.h"

// Libs
#include <avr/interrupt.h>

/*
 * Initialize UNO timer 1 to mode 9 which is PWM, Phase and frequency correct,
 * OCR1A
 *
 * @param None
 * @returns Void
 */
void TIMER1_Init_Mode_9() {
  // Enables interrupts
  sei();

  // Clear registers
  TIMER1_Clear();

  // Toggle on compare match: UNO 328p doc 131 table 16-1.
  TCCR1A |= (1 << COM1A0);

  // UNO doc page 132 table 16-4, PWM, Phase and Frequency Correct, TOP OCR1A
  TCCR1A |= (1 << WGM10);
  TCCR1B |= (1 << WGM13);

  // Enable output compare A match interrupt
  TIMSK1 |= (1 << OCIE1A);
  TCNT1 = 0;
  // // Set default Prescaler and TOP to Second:
  // TIMER1_SetPrescaler(PS_64);
  // TIMER1_SetTarget(900);
}

/*
 * Initialize UNO timer 1 to mode 4 which is CTC
 * OCR1A
 *
 * @param None
 * @returns Void
 */
void TIMER0_Init_Mode_2() {
  // Enables interrupts
  sei();
  TIMER0_Clear();
  // Toggle on compare match: UNO 328p doc 104 table 15-2.
  TCCR0A |= (1 << COM0A0);

  // UNO doc page 132 table 15-8 CTC, TOP OCRA
  TCCR0A |= (1 << WGM01);

  // prescaler 64
  TCCR0B |= (1 << CS02) | (1 << CS00);

  // Enable output compare A match interrupt
  TIMSK1 |= (1 << OCIE0A);
  TCNT0 = 0;
}

void TIMER0_Clear() {
  // Clear registers
  TCCR0A = 0;
  TIMSK0 = 0;
  TCNT0 = 0;
}

uint32_t tone(uint16_t frequency, uint16_t duration) {
  if (0 < duration) {
    return (2 * frequency * duration / 1000);
  } else {
    return -1;
  }
}

/*
 * Clear UNO timer 1 registers
 *
 * @param None
 * @returns void
 */
void TIMER1_Clear() {
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TIMSK1 = 0;
}

/*
 * Set UNO Timer 1 prescaler value 1; 8; 64; 256; 1024
 *
 * @param None
 * @returns Void
 */
void TIMER1_SetPrescaler(const uint16_t prescaler) {
  switch (prescaler) {
  case 1:
    // Prescaler value 1
    TCCR1B |= 0b00000001;
    break;
  case 8:
    // Prescaler value 8
    TCCR1B |= 0b00000010;
    break;
  case 64:
    // Prescaler value 64
    TCCR1B |= 0b00000011;
    break;
  case 256:
    // Prescaler value 256
    TCCR1B |= 0b00000100;
    break;
  case 1024:
    // Prescaler value 1024
    TCCR1B |= 0b00000101;
    break;
  default:
    break;
  }
}

/*
 * Set target TOP value of UNO Timer 1
 *
 * @param uint16_t value of target TOP.
 * @returns Void
 */
void TIMER1_SetTarget(uint16_t value) { OCR1A = value; }

/*
 EOF
 */
