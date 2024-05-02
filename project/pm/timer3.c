#include "timer3.h"

// Libs
#include <avr/interrupt.h>

/*
 Initialize Mega timer 3 to mode 4 (CTC)
 *
 * @param None
 * @returns void
 */
void TIMER3_Init_CTC()
{
    // Enables interrupts
    sei();

    // Clear registers
    TIMER3_Clear();

    // Toggle on compare match: 2560 doc 155 table 17-3.
    TCCR3A |= (1 << COM3A0);

    // Refer to the documentation at 128 table 16-8 with Waveform generation
    // modes. Here it is set to CTC
    TCCR3B |= (1 << WGM32);

    // Enable output compare A match interrupt
    TIMSK3 |= (1 << OCIE3A);

    // Set default Prescaler and TOP :
    TIMER3_SetIntervalSecond();
}

/*
 * Clear Mega timer 3 registers
 *
 * @param None
 * @returns void
 */
void TIMER3_Clear()
{
    TCCR3A = 0;
    TCCR3B = 0;
    TCNT3 = 0;
    TIMSK3 = 0;
}

/*
 * Set Mega Timer 3 prescaler value 1; 8; 64; 256; 1024
 *
 * @param uint16_t prescaler value to be used for timer
 * @returns void
 */
void TIMER3_SetPrescaler(const uint16_t prescaler)
{
    switch (prescaler) {
    case 1:
        // Prescaler value 1
        TCCR3B |= 0b00000001;
        break;
    case 8:
        // Prescaler value 8
        TCCR3B |= 0b00000010;
        break;
    case 64:
        // Prescaler value 64
        TCCR3B |= 0b00000011;
        break;
    case 256:
        // Prescaler value 256
        TCCR3B |= 0b00000100;
        break;
    case 1024:
        // Prescaler value 1024
        TCCR3B |= 0b00000101;
        break;
    default:
        break;
    }
}

/*
 * Set target TOP value of Mega Timer 3
 *
 * @param uint16_t TOP value to be set to OCR3A register.
 * @returns void
 */
void TIMER3_SetTarget(uint16_t value) { OCR3A = value; }

/*
 * Function to set the CTC timer 3 interval to every second.
 *
 * @param None
 * @returns void
 */
void TIMER3_SetIntervalSecond()
{
    TIMER3_SetPrescaler(PS_1024);
    TIMER3_SetTarget(SECOND_1024);
}

/*
 EOF
 */
