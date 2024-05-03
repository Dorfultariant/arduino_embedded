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
void timer1_init_mode_9()
{
    // Enables interrupts
    sei();

    // Clear registers
    timer1_clear();

    // Toggle on compare match: UNO 328p doc 131 table 16-1.
    TCCR1A |= (1 << COM1A0);

    // UNO doc page 132 table 16-4, PWM, Phase and Frequency Correct, TOP OCR1A
    TCCR1A |= (1 << WGM10);
    TCCR1B |= (1 << WGM13);

    // Enable output compare A match interrupt
    TIMSK1 |= (1 << OCIE1A);
    TCNT1 = 0;
}

/*
 * Clear UNO timer 1 registers
 *
 * @param None
 * @returns void
 */
void timer1_clear()
{
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
void timer1_set_prescaler(const uint16_t prescaler)
{
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
void timer1_set_target(uint16_t value) 
{ 
    OCR1A = value; 
}

/*
 EOF
 */
